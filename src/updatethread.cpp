/*
 * updatethread.cpp
 *
 *  Created on: Oct 20, 2009
 *      Author: Sergei Stolyarov
 */

#include <QtCore>
#include <QtSql>

#include "updatethread.h"
#include "formats.h"

const int COL_ID = 0;
const int COL_NAME = 1;
const int COL_PATH = 2;

typedef QPair<QString, QString> StringsPair;

static const QString TABLE_SQL_DFN_DIR_TMP("CREATE TEMPORARY TABLE _dir ("
	"id INTEGER PRIMARY KEY, "
	"path VARCHAR, " //
		"modtime INTEGER " // file modification time (unixtime)
		")");

void scanDir(QDir & d, QSqlQuery * query) {
	QFileInfoList el = d.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot,
			QDir::NoSort);

	Q_FOREACH (const QFileInfo & s, el) {
		//qDebug() << s.fileName();
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
	QHash<QString, int> albumsHash;
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
		QString t = deletedDirectories.join(", ");
		qDebug() << "delete directories from db" << deletedDirectories;
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
	QList<StringsPair> updatedDirs;
	//QStringList updatedDirs;
	//QStringList updatedDirsPaths;

	subq.prepare(
			"INSERT INTO dir (path, modtime) VALUES(:path, :modtime)");
	qDebug() << ">> find new directories";
	p->query->exec(
			"SELECT _d.path, _d.modtime FROM _dir AS _d LEFT JOIN dir AS d ON d.path=_d.path WHERE d.path IS NULL");
	int cnt = 0;
	while (p->query->next()) {
		cnt++;
		subq.bindValue(":path", p->query->value(0).toString());
		subq.bindValue(":modtime", p->query->value(1).toString());
		subq.exec();
		StringsPair pair(subq.lastInsertId().toString(), p->query->value(0).toString());
		updatedDirs << pair;
	}
	qDebug() << ">> new directories number: " << cnt;

	emit progressPercentChanged(4);
	qDebug() << ">> find updated directories";
	p->query->exec(
			"SELECT d.id, d.path FROM dir AS d JOIN _dir as _d ON d.path=_d.path AND d.modtime != _d.modtime");
	cnt = 0;
	while (p->query->next()) {
		cnt++;
		StringsPair pair(p->query->value(0).toString(), p->query->value(1).toString());
		updatedDirs << pair;
	}
	qDebug() << ">> updated directories number: " << cnt;

	if (updatedDirs.count()) {
		// delete all tracks for updated and new directories
		QStringList dirs;
		Q_FOREACH (const StringsPair & sp, updatedDirs) {
			dirs << sp.first;
		}
		QString t = dirs.join(", ");
		p->query->prepare(QString("DELETE FROM track WHERE dir_id IN (%1)").arg(t));
		if (!p->query->exec()) {
			qDebug() << p->query->lastError();
		}

		QStringList name_filters;
		Q_FOREACH(const QString & s, Ororok::supportedFileExtensions()) {
			name_filters << QString("*.%1").arg(s);
		}

		emit progressPercentChanged(5);
		int dirsNum = updatedDirs.size();
		// find files in the updated and new directories, add them to the db
		p->query->prepare(
				"INSERT INTO track (dir_id, title, filename, artist_id, album_id, genre_id, track, year) "
				"VALUES (:dirId, :title, :filename, :artistId, :albumId, :genreId, :track, :year)");
		float processedDirs = 0;

		QDir dir;
		QStringList values;

		Q_FOREACH (const StringsPair & sp, updatedDirs) {
			const QString & dirId = sp.first;
			const QString & dirPath = sp.second;

			// read all tracks from the directory "dirPath"
			dir.setPath(dirPath);
			QList<QFileInfo> files = dir.entryInfoList(name_filters,
					QDir::Files);

			// extract info from the file
			Q_FOREACH (const QFileInfo & fi, files) {
				bool success;

				// get file metadata
				Ororok::MusicTrackMetadata * md = Ororok::getMusicFileMetadata(fi.filePath(), success);
				int artistId = 0;
				int albumId = 0;
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

				// the same with the genres
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

				// now find album id, this process is mainly empiric
				if (!md->album.isEmpty()) {
					albumId = p->albumsHash.value(md->album, -1);
					if (-1 == albumId) {
						subq.prepare("INSERT INTO album (name) VALUES (:name)");
						subq.bindValue(":name", md->album);
						if (subq.exec()) {
							albumId = subq.lastInsertId().toInt();
						}
						p->albumsHash[md->album] = albumId;
					}
				}
				p->query->bindValue(":dirId", dirId);
				p->query->bindValue(":filename", fi.fileName());
				p->query->bindValue(":artistId", artistId);
				p->query->bindValue(":albumId", albumId);
				p->query->bindValue(":genreId", genreId);
				p->query->bindValue(":track", md->track);
				p->query->bindValue(":year", md->year);
				p->query->bindValue(":title", md->title);
				delete md;
				if (!p->query->exec()) {
					qDebug() << p->query->lastError();
				}
			}
			// one directory updated, increase progress
			processedDirs += 1;
			float x = (processedDirs / dirsNum) * 95;
			emit progressPercentChanged(5 + static_cast<int>(x));
		}
	}

	emit progressPercentChanged(100);



	//

	QSqlQuery q1(*(p->db));
	QSqlQuery q2(*(p->db));
	QSqlQuery q3(*(p->db));
	QSqlQuery q4(*(p->db));
	QSqlQuery q(*(p->db));
	QStringList coverImages;
	coverImages << "cover.jpg" << "cover.gif" << "cover.png" << "folder.jpg" << "folder.gif" << "folder.png";

	// process all altered/added albums
	q1.prepare("SELECT artist_id FROM track WHERE album_id=:albumId GROUP BY artist_id");
	q2.prepare("UPDATE album SET artist_id=:artistId, image_id=:imageId WHERE id=:albumId");
	q3.prepare("SELECT track.id, dir.path FROM track INNER JOIN dir ON track.dir_id=dir.id AND track.album_id=:albumId LIMIT 1;");
	q4.prepare("INSERT OR IGNORE INTO image (path) VALUES (:imagePath)");
	// process all just added albums and detect album's artist
	QHash<QString, int>::const_iterator a;
	for (a=p->albumsHash.constBegin(); a!=p->albumsHash.constEnd(); a++) {
		// find album artist
		const int & albumId = a.value();

		q1.bindValue(":albumId", albumId);
		if (!q1.exec()) {
			p->errorMessage = "Unable to detect album artist";
			p->errorCode = UnableToDetectAlbumArtist;
			return Terminate;
		}

		int k = 0;
		QVariant artistId;
		while (q1.next()) {
			k++;
			artistId = q1.value(0);
		}
		if (k == 0) {
			continue;
		}

		// find album picture
		q3.bindValue(":albumId", albumId);
		if (!q3.exec()) {
			p->errorMessage = "Unable to fetch album directory";
			p->errorCode = UnableToFetchAlbumDir;
			return Terminate;
		}
		int imgId = -1;
		if (q3.next()) {
			QString path = q3.value(1).toString();
			// try to find picture in this folder: (cover|folder).(jpg|png|gif)
			Q_FOREACH(const QString coverImage, coverImages) {
				QDir dir(path);
				QStringList res = dir.entryList(coverImages, QDir::Files|QDir::Readable, QDir::NoSort);
				if (!res.count()) {
					continue;
				}
				QString imgPath = path+"/"+res[0];
				q.prepare("SELECT id FROM image WHERE path = :path");
				q.bindValue(":path", imgPath);
				if (!q.exec()) {
					continue;
				}
				if (q.next()) {
					imgId = q.value(0).toInt();
				} else {
					q4.bindValue(":imagePath", imgPath);
					if (!q4.exec()) {
						continue;
					}
					imgId = q4.lastInsertId().toInt();
				}
			}
		}

		q2.bindValue(":albumId", albumId);
		q2.bindValue(":imageId", imgId);
		if (k == 1) {
			q2.bindValue(":artistId", artistId);
		} else if (k > 1) {
			// this is various artists album
			q2.bindValue(":artistId", "-1");
		}
		if (!q2.exec()) {
			p->errorMessage = "Unable to write album artist";
			p->errorCode = UnableToSaveAlbumArtist;
			return Terminate;
		}
	}

	return Continue;
}

typedef QList<QVariant> VariantList;

void UpdateThread::run() {
	p->errorCode = NoError;
	p->errorMessage.clear();

	emit progressPercentChanged(0);

	QSqlDatabase _db = QSqlDatabase::database();
	// required for commits in other class' functions
	p->db = &_db;
	p->query = new QSqlQuery(*(p->db));

	// load artists
	p->artistsHash.clear();
	if (!p->query->exec("SELECT id, name FROM artist")) {
		p->errorCode = UnableToLoadArtists;
		p->db->rollback();
		return;
	}
	while (p->query->next()) {
		p->artistsHash[p->query->value(1).toString()] = p->query->value(0).toInt();
	}

	p->genresHash.clear();
	if (!p->query->exec("SELECT id, name FROM genre")) {
		p->errorCode = UnableToLoadGenres;
		p->db->rollback();
		return;
	}
	while (p->query->next()) {
		p->genresHash[p->query->value(1).toString()] = p->query->value(0).toInt();
	}

	// TODO: load albums, it's not too easy because albums are  not name-unique
	p->albumsHash.clear();

	p->db->transaction();

	QStringList collections;

	if (!p->query->exec("SELECT path FROM collection WHERE enabled=1")) {
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

	Q_FOREACH (const QString & c, collections) {
		ReturnAction a;
		a = scanCollection(c);
		if (Break == a) {
			break;
		} else if (Terminate == a) {
			commit = false;
			break;
		}
	}

	emit progressPercentChanged(2);

	if (commit && Terminate == updateCollections()) {
		commit = false;
	}

	p->query->exec("DROP TABLE _dir");
	delete p->query;
	p->query = 0;
	p->albumsHash.clear();
	p->artistsHash.clear();
	p->genresHash.clear();

	if (commit) {
		p->db->commit();
	} else {
		p->db->rollback();
	}
	p->db = 0;
}
