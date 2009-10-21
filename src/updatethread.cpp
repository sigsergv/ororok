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
	QMap<QString, int> genresMap;
	QMap<QString, int> artistsMap;
	QMap<QString, int> albumsMap;
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
	case UpdateThread::UbableToSaveAlbumArtist:
		res = "Unable to save album artist to the database";
		break;
	}

	return res;
}

UpdateThread::ReturnAction UpdateThread::scanCollection(int collectionId,
		const QString & collectionPath, const QString & collectionName)
{
	// find all subdirs in the directory collection_path
	//
	QDir d(collectionPath);
	p->query->prepare(
			"INSERT INTO _dir (path, modtime) VALUES (:path, :modtime)");
	scanDir(d, p->query);
	return Continue;
}

UpdateThread::ReturnAction UpdateThread::processCollections()
{
	// compare just found dirs with the ones from db
	// find and process deleted directories
	QStringList dirs_to_delete;
	QString t;
	qDebug() << ">> find deleted directories";
	p->query->exec(
			"SELECT d.id, d.path FROM dir AS d LEFT JOIN _dir AS _d ON d.path=_d.path WHERE _d.path IS NULL");

	while (p->query->next()) {
		dirs_to_delete << p->query->value(0).toString();
		if (p->stopRequested) {
		}
	}

	if (dirs_to_delete.length() > 0) {
		t = dirs_to_delete.join(", ");
		qDebug() << "delete directories from db" << dirs_to_delete;
		p->query->prepare(
				QString("DELETE FROM track WHERE path_id IN (%1)").arg(
						t));
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
	QVariant last_insert_id;
	QStringList dirs_to_update;
	QStringList dirs_to_update_paths;

	subq.prepare(
			"INSERT INTO dir (path, modtime) VALUES(:path, :modtime)");
	qDebug() << ">> new directories";
	p->query->exec(
			"SELECT _d.path, _d.modtime FROM _dir AS _d LEFT JOIN dir AS d ON d.path=_d.path WHERE d.path IS NULL");
	while (p->query->next()) {
		subq.bindValue(":path", p->query->value(0).toString());
		subq.bindValue(":modtime", p->query->value(1).toString());
		subq.exec();
		last_insert_id = subq.lastInsertId();
		dirs_to_update << last_insert_id.toString();
		dirs_to_update_paths << p->query->value(0).toString();
		//qDebug() << query.value(0).toString();
	}

	emit progressPercentChanged(4);
	// find updated dirs
	qDebug() << ">> updated directories";
	p->query->exec(
			"SELECT d.id, d.path FROM dir AS d JOIN _dir as _d ON d.path=_d.path AND d.modtime != _d.modtime");
	while (p->query->next()) {
		dirs_to_update << p->query->value(0).toString();
		dirs_to_update_paths << p->query->value(1).toString();
		//qDebug() << query.value(1).toString();
	}

	// update all tracks for updated dirs
	// delete all track records for all updated dirs
	t = dirs_to_update.join(", ");
	p->query->prepare(
			QString("DELETE FROM track WHERE path_id IN (%1)").arg(t));
	if (!p->query->exec()) {
		qDebug() << p->query->lastError();
	}
	// re-read tracks again if required
	QString path_id;
	QDir dir;
	QStringList name_filters;
	QStringList values;
	Q_FOREACH(const QString s, Ororok::supportedFileExtensions()) {
		name_filters << QString("*.%1").arg(s);
	}

	emit progressPercentChanged(5);
	QStringList::const_iterator i = dirs_to_update.constBegin();
	const QString values_template("('%1', '%2')");

	p->query->prepare(
			"INSERT INTO track (path_id, filename, artist_id, album_id, genre_id, track, year) "
			"VALUES (:path_id, :filename, :artistId, :albumId, :genreId, :track, :year)");

	int dirsNum = dirs_to_update_paths.length();
	float processedDirs = 0;
	Q_FOREACH (const QString & path, dirs_to_update_paths)
	{
		path_id = *i;
		// read all tracks from the directory path
		dir.setPath(path);
		QList<QFileInfo> files = dir.entryInfoList(name_filters,
				QDir::Files);

		// extract info from the file

		Q_FOREACH (const QFileInfo & fi, files)
		{
			bool success;

			// get file metadata
			Ororok::MusicTrackMetadata * md = Ororok::getMusicFileMetadata(fi.filePath(), success);
			int artistId = 0;
			int albumId = 0;
			int genreId = 0;
			if (md->artist.length()) {
				if (p->artistsMap.contains(md->artist)) {
					artistId = p->artistsMap[md->artist];
				} else {
					// append artist to the database
					subq.prepare("INSERT INTO artist (name) VALUES (:name)");
					subq.bindValue(":name", md->artist);
					if (subq.exec()) {
						artistId = subq.lastInsertId().toInt();
					}
					p->artistsMap[md->artist] = artistId;
				}
			}
			if (md->genre.length()) {
				if (p->genresMap.contains(md->genre)) {
					genreId = p->genresMap[md->genre];
				} else {
					// append genre to the database
					subq.prepare("INSERT INTO genre (name) VALUES (:name)");
					subq.bindValue(":name", md->genre);
					if (subq.exec()) {
						genreId = subq.lastInsertId().toInt();
					}
					p->genresMap[md->genre] = genreId;
				}
			}
			if (md->album.length()) {
				if (p->albumsMap.contains(md->album)) {
					albumId = p->albumsMap[md->album];
				} else {
					// append genre to the database
					subq.prepare("INSERT INTO album (name) VALUES (:name)");
					subq.bindValue(":name", md->album);
					if (subq.exec()) {
						albumId = subq.lastInsertId().toInt();
					}
					p->albumsMap[md->album] = albumId;
				}
			}
			p->query->bindValue(":path_id", path_id);
			p->query->bindValue(":filename", fi.fileName());
			p->query->bindValue(":artistId", artistId);
			p->query->bindValue(":albumId", albumId);
			p->query->bindValue(":genreId", genreId);
			p->query->bindValue(":track", md->track);
			p->query->bindValue(":year", md->year);
			delete md;
			if (!p->query->exec()) {
				qDebug() << p->query->lastError();
			}
		}
		// one directory updated, increase progress
		processedDirs += 1;
		i++;

		float x = (processedDirs / dirsNum) * 95;
		emit progressPercentChanged(5 + static_cast<int>(x));
	}

	//

	QSqlQuery q1(*(p->db));
	QSqlQuery q2(*(p->db));

	q1.prepare("SELECT artist_id FROM track WHERE album_id=:albumId GROUP BY artist_id");
	q2.prepare("UPDATE album SET artist_id=:artistId WHERE id=:albumId");
	// process all just added albums and detect album's artist
	QMap<QString, int>::const_iterator a;
	for (a=p->albumsMap.constBegin(); a!=p->albumsMap.constEnd(); a++) {
		const QString & albumName = a.key();
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

		q2.bindValue(":albumId", albumId);
		if (k == 1) {
			q2.bindValue(":artistId", artistId);
		} else if (k > 1) {
			// this is various artists album
			q2.bindValue(":artistId", "-1");
		}
		if (!q2.exec()) {
			p->errorMessage = "Unable to write album artist";
			p->errorCode = UbableToSaveAlbumArtist;
			return Terminate;
		}
	}

	return Continue;
}

typedef QList<QVariant> VariantList;

void UpdateThread::run() {
	p->errorCode = NoError;
	p->errorMessage.clear();
	p->artistsMap.clear();
	p->genresMap.clear();
	p->albumsMap.clear();

	emit progressPercentChanged(0);

	QSqlDatabase _db = QSqlDatabase::database();
	// required for commits in other class' functions
	p->db = &_db;

	p->query = new QSqlQuery(*(p->db));

	p->db->transaction();

	QList<VariantList> collections;

	if (!p->query->exec("SELECT id, name, path FROM collection WHERE enabled=1")) {
		p->errorCode = GetCollectionsListError;
		p->db->rollback();
		return;
	}

	while (p->query->next()) {
		VariantList values;
		values.append(p->query->value(0)); // id
		values.append(p->query->value(1)); // name
		values.append(p->query->value(2)); // path
		collections.append(values);
	}

	if (!p->query->exec(TABLE_SQL_DFN_DIR_TMP)) {
		p->errorCode = UnableToCreateTemporaryDirsTable;
		p->db->rollback();
		return;
	}

	bool commit = true;

	Q_FOREACH (const VariantList & c, collections) {
		ReturnAction a;
		a = scanCollection(c[0].toInt(), c[2].toString(), c[1].toString()); // id, path, name!
		if (Break == a) {
			break;
		} else if (Terminate == a) {
			commit = false;
			break;
		}
	}

	emit progressPercentChanged(2);

	if (Terminate == processCollections()) {
		commit = false;
	}

	p->query->exec("DROP TABLE _dir");
	delete p->query;
	p->query = 0;
	p->albumsMap.clear();
	p->artistsMap.clear();
	p->genresMap.clear();

	if (commit) {
		p->db->commit();
	} else {
		p->db->rollback();
	}
	p->db = 0;
}
