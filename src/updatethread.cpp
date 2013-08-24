/*
 * updatethread.cpp
 *
 *  Created on: Oct 20, 2009
 *      Author: Sergey Stolyarov
 */

#include <QtCore>
#include <QtSql>

#include "updatethread.h"
#include "formats.h"

const int COL_ID = 0;
const int COL_NAME = 1;
const int COL_PATH = 2;

typedef QPair<QString, QString> StringsPair;
typedef QSet<QString> StringSet;
typedef QSet<int> IntegerSet;

static const QString TABLE_SQL_DFN_DIR_TMP("CREATE TEMPORARY TABLE _dir ("
	"id INTEGER PRIMARY KEY, "
	"path VARCHAR, " //
		"modtime INTEGER " // file modification time (unixtime)
		")");

void scanDir(QDir & d, QSqlQuery * query) {
	QFileInfoList el = d.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot,
			QDir::NoSort);

	Q_FOREACH (const QFileInfo & s, el) {
		// add directory to the temporary table
		query->bindValue(":path", s.absoluteFilePath()); // set full path
		query->bindValue(":modtime", s.lastModified().toTime_t());
		query->exec();
		d.cd(s.fileName());
		scanDir(d, query);
		d.cdUp();
	}
}

struct UpdateThread::Private
{
	bool stopRequested;
	QString errorMessage;
	UpdateThread::UpdateThreadError errorCode;
	QSqlDatabase * db;
	QSqlQuery * query;
	QHash<QString, int> genresHash;
	QHash<QString, int> artistsHash;
	//QHash<QString, int> albumsHash;
};

UpdateThread::UpdateThread(QObject * parent) :
	QThread(parent)
{
	p = new Private();
	p->stopRequested = false;
}

UpdateThread::~UpdateThread()
{
	delete p;
}

void UpdateThread::softStop()
{
	p->stopRequested = true;
}

QString UpdateThread::errorMessage()
{
	return p->errorMessage;
}

UpdateThread::UpdateThreadError UpdateThread::errorCode()
{
	return p->errorCode;
}

QString UpdateThread::errorToText(UpdateThread::UpdateThreadError err)
{
	QString res;

	switch (err) {
	case UpdateThread::NoError:
		res = "No error";
		break;
	case UpdateThread::GetCollectionsListError:
		res = "Collections retrieving failed";
		break;
	case UpdateThread::UnableToLoadArtists:
		res = "Unable to load artists list from the database";
		break;
	case UpdateThread::UnableToLoadGenres:
		res = "Unable to load genres list from the database";
		break;
	case UpdateThread::UnableToCreateTemporaryDirsTable:
		res = "Failed to create temporary table “_dirs”";
		break;
	case UpdateThread::UnableToDetectAlbumArtist:
		res = "Unable to detect album artist";
		break;
	case UpdateThread::UnableToSaveAlbumArtist:
		res = "Unable to save album artist to the database";
		break;
	}

	return res;
}

/**
 * scan single collection, recursively find all directories
 * in the directory collectionPath. All found dirs will be
 * inserted into temporary table "_dir".
 *
 * @param collectionPath
 * @return
 */
UpdateThread::ReturnAction UpdateThread::scanCollection(const QString & collectionPath)
{
	QDir d(collectionPath);
	p->query->prepare(
			"INSERT INTO _dir (path, modtime) VALUES (:path, :modtime)");
	scanDir(d, p->query);
	return Continue;
}

/**
 * update database using content of just created temporary table "_dir"
 * find all new, alterd and deleted directories. Update files for
 * all affected existing directories
 *
 * @return
 */
UpdateThread::ReturnAction UpdateThread::updateCollections()
{
	// find deleted directoris
	QStringList deletedDirectories;
	QStringList coverImages;
	ReturnAction ra = ContinueNoChanges;

	coverImages << "cover.jpg" << "cover.gif" << "cover.png" << "folder.jpg" << "folder.gif" << "folder.png";

	qDebug() << ">> find deleted directories";
	p->query->exec(
			"SELECT d.id, d.path FROM dir AS d LEFT JOIN _dir AS _d ON d.path=_d.path WHERE _d.path IS NULL");

	while (p->query->next()) {
		deletedDirectories << p->query->value(0).toString();
		if (p->stopRequested) {
		}
	}

	qDebug() << ">> deleted directories number: " << deletedDirectories.count();

	if (deletedDirectories.length() > 0) {
		qDebug() << "delete directories from db" << deletedDirectories;
		QString t = deletedDirectories.join(", ");
		ra = Continue;
		p->query->prepare(
				QString("DELETE FROM track WHERE dir_id IN (%1)").arg(t));
		if (!p->query->exec()) {
			qDebug() << p->query->lastError();
		}
		p->query->prepare(
			QString("DELETE FROM dir WHERE id IN (%1)").arg(t));
		if (!p->query->exec()) {
			qDebug() << p->query->lastError();
		}
	}

	emit progressPercentChanged(3);

	QSqlQuery subq;
	QList<QStringList> updatedDirs; // [id, path, modtime]
	//QStringList updatedDirs;
	//QStringList updatedDirsPaths;

	subq.prepare(
			"INSERT INTO dir (path, modtime) VALUES(:path, :modtime)");
	qDebug() << ">> find new directories";
	p->query->exec(
			"SELECT _d.path, _d.modtime FROM _dir AS _d LEFT JOIN dir AS d ON d.path=_d.path WHERE d.path IS NULL");
	int cnt = 0;
	while (p->query->next()) {
		ra = Continue;
		cnt++;
		subq.bindValue(":path", p->query->value(0).toString());
		subq.bindValue(":modtime", p->query->value(1).toString());
		subq.exec();
		StringsPair pair(subq.lastInsertId().toString(), p->query->value(0).toString());
		QStringList sl;
		sl << subq.lastInsertId().toString() << p->query->value(0).toString() << p->query->value(1).toString();
		updatedDirs << sl;
	}
	qDebug() << ">> new directories number: " << cnt;

	emit progressPercentChanged(4);
	qDebug() << ">> find updated directories";
	p->query->exec(
			"SELECT d.id, d.path, _d.modtime FROM dir AS d JOIN _dir as _d ON d.path=_d.path AND d.modtime != _d.modtime");
	cnt = 0;
	while (p->query->next()) {
		cnt++;
		QStringList sl;
		sl << p->query->value(0).toString() << p->query->value(1).toString() << p->query->value(2).toString();
		updatedDirs << sl;
	}
	qDebug() << ">> updated directories number: " << cnt;

	int dirsNum = updatedDirs.size();
	if (dirsNum) {
		ra = Continue;
		// delete all tracks for updated and new directories
		QStringList dirsIds;
		Q_FOREACH (const QStringList & sl, updatedDirs) {
			dirsIds << sl[0];
		}
		QString t = dirsIds.join(", ");
		p->query->prepare(QString("DELETE FROM track WHERE dir_id IN (%1)").arg(t));
		if (!p->query->exec()) {
			qDebug() << p->query->lastError();
		}

		QStringList name_filters;
		Q_FOREACH(const QString & s, Ororok::supportedFileExtensions()) {
			name_filters << QString("*.%1").arg(s);
		}

		emit progressPercentChanged(5);
		p->query->prepare(
				"INSERT INTO track (dir_id, title, filename, artist_id, album_id, genre_id, track, length, year) "
				"VALUES (:dirId, :title, :filename, :artistId, :albumId, :genreId, :track, :length, :year)");
		float processedDirs = 0;

		QDir dir;
		QStringList values;

		// find files in the updated and new directories, add them to the db
		Q_FOREACH (const QStringList & sl, updatedDirs) {
			const QString & dirId = sl[0];
			const QString & dirPath = sl[1];
			const QString & dirModtime = sl[2];
			// <albumMame, albumId>
			// <albumName, {albumYear,...}>
			QHash<QString, IntegerSet> albumYears; // tracks' years
			QHash<QString, IntegerSet> albumArtists; // tracks' artists' ids

			qDebug() << "updated directory " << dirPath;

			// update directory modtime
			subq.prepare("UPDATE dir SET modtime=:time WHERE id=:id");
			subq.bindValue(":time", dirModtime);
			subq.bindValue(":id", dirId);
			subq.exec();

			// read all tracks from the directory "dirPath"
			dir.setPath(dirPath);
			QList<QFileInfo> files = dir.entryInfoList(name_filters, QDir::Files);

			// all files in the directoy must belong to single album
			QSet<QString> dirAlbums;

			QHash<QString, Ororok::MusicTrackMetadata*> mdHash;
			Q_FOREACH (const QFileInfo & fi, files) {
				Ororok::MusicTrackMetadata * md = Ororok::getMusicFileMetadata(fi.filePath());
				mdHash[fi.filePath()] = md;
				dirAlbums.insert(md->album);
			}

			// if dirAlbums contains more than one element then take album name from the directory name
			QString albumName;
			int albumYear = -1;

			if (dirAlbums.size() == 1 && dirAlbums.constBegin()->trimmed().length() != 0) {
				albumName = *dirAlbums.constBegin();
			} else {
				albumName = dir.dirName();
				// try to extract album year from directory name
				QRegExp rx("^([0-9]{4})[ -_]");
				int pos = rx.indexIn(albumName);
				if (pos != -1) {
					bool bOk;
					int yearInt = rx.cap(1).toInt(&bOk);
					if (bOk && yearInt > 1850 && yearInt < 2300) { // quite reasonable assumption
						albumYear = yearInt;
						albumName = albumName.mid(pos).trimmed();
					}
				}
			}

			subq.prepare("INSERT INTO album (name) VALUES (:name)");
			subq.bindValue(":name", albumName);
			int albumId = 0;
			if (subq.exec()) {
				albumId = subq.lastInsertId().toInt();
			}

			// extract info from the file
			Q_FOREACH (const QFileInfo & fi, files) {
				// get file metadata
				Ororok::MusicTrackMetadata * md = mdHash[fi.filePath()];
				int artistId = 0;
				int genreId = 0;

				if (!md->artist.isEmpty()) {
					artistId = p->artistsHash.value(md->artist, -1);
					if (-1 == artistId) {
						// append artist to the database...
						subq.prepare("INSERT INTO artist (name) VALUES (:name)");
						subq.bindValue(":name", md->artist);
						if (subq.exec()) {
							artistId = subq.lastInsertId().toInt();
						}
						// ...and add to the artistsHash
						p->artistsHash[md->artist] = artistId;
					}
				}
				if (artistId) {
					// add artistId to album artists list
					albumArtists[md->album].insert(artistId);
				}

				if (!md->genre.isEmpty()) {
					genreId = p->genresHash.value(md->genre, -1);
					if (-1 == genreId) {
						subq.prepare("INSERT INTO genre (name) VALUES (:name)");
						subq.bindValue(":name", md->genre);
						if (subq.exec()) {
							genreId = subq.lastInsertId().toInt();
						}
						p->genresHash[md->genre] = genreId;
					}
				}

				if (md->year > 0) {
					// add track year to album years list
					albumYears[md->album].insert(md->year);
				}

				p->query->bindValue(":dirId", dirId);
				p->query->bindValue(":filename", fi.fileName());
				p->query->bindValue(":artistId", artistId);
				p->query->bindValue(":albumId", albumId);
				p->query->bindValue(":genreId", genreId);
				p->query->bindValue(":track", md->track);
				p->query->bindValue(":length", md->length);
				p->query->bindValue(":year", md->year);
				p->query->bindValue(":title", md->title);
				delete md;
				if (!p->query->exec()) {
					qDebug() << p->query->lastError();
				}
			}

			// all files in the current directory have been processed
			// so it's time to guess various details for albums (year, artist)
			// walk through all detected albums
			int albumImageId = -1;
			QDir dir(dirPath);
			QStringList res = dir.entryList(coverImages, QDir::Files|QDir::Readable, QDir::NoSort);
			if (res.count()) {
				QString imgPath = dirPath + "/" + res[0];

				subq.prepare("SELECT id FROM image WHERE path = :path");
				subq.bindValue(":path", imgPath);
				if (subq.exec()) {
					if (subq.next()) {
						albumImageId = subq.value(0).toInt();
					} else {
						QSqlQuery q(*p->db);
						q.prepare("INSERT OR IGNORE INTO image (path) VALUES (:imagePath)");
						q.bindValue(":imagePath", imgPath);
						if (q.exec()) {
							albumImageId = q.lastInsertId().toInt();
						}
					}
				}
			}


			IntegerSet s = albumArtists.value(albumName);
			int albumArtistId = -1;
			if (s.size() == 1) {
				// all tracks for this album in this directory have the same
				// artist, so consider it as an album's artist
				albumArtistId = *s.constBegin();
			}

			s = albumYears.value(albumName);
			if (s.size() == 1) {
				// all tracks for this album in this directory have the same
				// year, so consider it as an album's year
				albumYear = *s.constBegin();
			}

			// update album info
			subq.prepare("UPDATE album SET artist_id=:artistId, year=:year, image_id=:imageId WHERE id=:albumId");
			subq.bindValue(":artistId", albumArtistId);
			subq.bindValue(":year", albumYear);
			subq.bindValue(":imageId", albumImageId);
			subq.bindValue(":albumId", albumId);
			subq.exec(); // ignore query result status
			qDebug() << "Updated album info" << albumYear << albumId << albumArtistId;

			// one directory updated, increase progress
			processedDirs += 1;
			float x = (processedDirs / dirsNum) * 95;
			emit progressPercentChanged(5 + static_cast<int>(x));
		}
	}

	// perform db cleanup: delete orphaned records etc
	// delete albums that do not contain tracks
	QStringList orphanedAlbums;
	p->query->prepare("SELECT album.name, album.id FROM album LEFT JOIN track ON album.id=track.album_id WHERE track.id IS NULL");
	if (p->query->exec()) {
		while (p->query->next()) {
			orphanedAlbums << p->query->value(1).toString();
		}
	}
	p->query->prepare(QString("DELETE FROM album WHERE id IN (%1)").arg(orphanedAlbums.join(", ")));
	p->query->exec(); // ignore execution result

	return ra;
}

typedef QList<QVariant> VariantList;

void UpdateThread::run() {
	p->errorCode = NoError;
	p->errorMessage.clear();

	emit progressPercentChanged(1);

	QSqlDatabase _db = QSqlDatabase::database();
	// required for commits in other class' functions
	p->db = &_db;
	p->query = new QSqlQuery(*(p->db));

	// load artists
	p->artistsHash.clear();
	if (!p->query->exec("SELECT id, name FROM artist")) {
		p->errorCode = UnableToLoadArtists;
		return;
	}
	while (p->query->next()) {
		p->artistsHash[p->query->value(1).toString()] = p->query->value(0).toInt();
	}

	p->genresHash.clear();
	if (!p->query->exec("SELECT id, name FROM genre")) {
		p->errorCode = UnableToLoadGenres;
		return;
	}
	while (p->query->next()) {
		p->genresHash[p->query->value(1).toString()] = p->query->value(0).toInt();
	}

	p->db->transaction();

	QStringList collections;

	if (!p->query->exec("SELECT path FROM collection")) {
		p->errorCode = GetCollectionsListError;
		p->db->rollback();
		return;
	}

	while (p->query->next()) {
		collections.append(p->query->value(0).toString());
	}

	if (!p->query->exec(TABLE_SQL_DFN_DIR_TMP)) {
		p->errorCode = UnableToCreateTemporaryDirsTable;
		p->db->rollback();
		return;
	}

	bool commit = true;

	ReturnAction a = Break;
	Q_FOREACH (const QString & c, collections) {
		a = scanCollection(c);
		if (Break == a) {
			break;
		} else if (Terminate == a) {
			commit = false;
			break;
		}
	}

	emit progressPercentChanged(2);

	if (commit) {
		a = updateCollections();
		if (a == Terminate) {
			commit = false;
		}
	}

	p->query->exec("DROP TABLE _dir");
	delete p->query;
	p->query = 0;
	p->artistsHash.clear();
	p->genresHash.clear();

	if (commit) {
		qDebug() << "commit changes";
		p->db->commit();
	} else {
		p->db->rollback();
	}
	p->db = 0;
	emit progressPercentChanged(100);
	if (a == Continue) {
		emit databaseAltered();
	} else {
		qDebug() << "No changes were made in the database so skip refreshing collections tree";
	}
}
