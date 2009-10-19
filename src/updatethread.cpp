/*
 * updatethread.cpp
 *
 *  Created on: Oct 20, 2009
 *      Author: Sergei Stolyarov
 */

#include <QtCore>
#include <QtSql>

#include "updatethread.h"

const int COL_ID = 0;
const int COL_NAME = 1;
const int COL_PATH = 2;

static const QString TABLE_SQL_DFN_DIR_TMP("CREATE TEMPORARY TABLE _dir ("
	"id INTEGER PRIMARY KEY, "
	"path VARCHAR, " //
		"modtime INTEGER " // file modification time (unixtime)
		")");

int scanDir(QDir & d, QSqlQuery * query) {
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
	int errorCode;
	QSqlDatabase * db;
	QSqlQuery * query;
};

UpdateThread::UpdateThread(QObject * parent) :
	QThread(parent)
{
	p->stopRequested = false;
	p = new Private();
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

int UpdateThread::errorCode()
{
	return p->errorCode;
}

UpdateThread::ReturnAction UpdateThread::processCollection(const QStringList & c)
{
	QString collection_id = c[COL_ID];
	QString collection_path = c[COL_PATH];
	QString collection_name = c[COL_NAME];

	// find all subdirs in the directory collection_path
	//
	QString t;
	QDir d(collection_path);
	p->query->prepare(
			"INSERT INTO _dir (path, modtime) VALUES (:path, :modtime)");
	scanDir(d, p->query);

	/*
	 query.exec("SELECT path, modtime FROM _dir");
	 while (query.next()) {
	 qDebug() << query.value(0).toString();
	 qDebug() << query.value(1).toInt();
	 }
	 */

	// compare just found dirs with the ones from db
	// find and process deleted directories
	QStringList dirs_to_delete;
	qDebug() << ">> deleted directories";
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
	name_filters << "*.Mp3";
	QStringList::const_iterator i = dirs_to_update.constBegin();
	const QString values_template("('%1', '%2')");

	p->query->prepare(
			"INSERT INTO track (path_id, filename) VALUES (:path_id, :filename)");
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
			p->query->bindValue(":path_id", path_id);
			p->query->bindValue(":filename", fi.fileName());
			if (!p->query->exec()) {
				qDebug() << p->query->lastError();
			}
		}
		//qDebug() << values;
		qDebug() << "xx: " << path;
		i++;
	}

	// find deleted directories
}

void UpdateThread::run() {
	p->errorCode = NoError;
	p->errorMessage.clear();

	p->db = &QSqlDatabase::database();

	p->query = new QSqlQuery(*(p->db));

	p->db->transaction();

	QList<QStringList> collections;

	if (!p->query->exec("SELECT id, name, path FROM collection WHERE enabled=1")) {
		p->errorCode = GetCollectionsListError;
		p->db->rollback();
		return;
	}

	while (p->query->next()) {
		QStringList values;
		values.append(p->query->value(0).toString());
		values.append(p->query->value(1).toString());
		values.append(p->query->value(2).toString());
		collections.append(values);
	}

	if (p->query->exec(TABLE_SQL_DFN_DIR_TMP)) {
		p->errorCode = CannotCreateTemporaryDirsTable;
		p->db->rollback();
		return;
	}

	bool commit = true;

	Q_FOREACH (const QStringList & c, collections) {
		ReturnAction a = processCollection(c);
		if (Break == a) {
			break;
		} else if (Terminate == a) {
			commit = false;
			break;
		}
	}

	p->query->exec("DROP TABLE _dir");

	if (commit) {
		p->db->commit();
	} else {
		p->db->rollback();
	}
}
