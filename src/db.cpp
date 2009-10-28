#include <QtSql>
#include <QtDebug>

#include "db.h"

//static const QString TABLE_SQL_DFN_("CREATE TABLE  ("
//    "id INTEGER PRIMARY KEY, "
//    "name VARCHAR "
//    ")");

static const QString TABLE_SQL_DFN_GENRE("CREATE TABLE genre ("
		"id INTEGER PRIMARY KEY, "
		"name VARCHAR, "
		"UNIQUE(name)"
		")");

static const QString TABLE_SQL_DFN_ARTIST("CREATE TABLE artist ("
		"id INTEGER PRIMARY KEY, "
		"name VARCHAR, "
		"UNIQUE(name)"
		")");

static const QString TABLE_SQL_DFN_ALBUM("CREATE TABLE album ("
		"id INTEGER PRIMARY KEY, "
		"name VARCHAR, "
		"artist_id INTEGER, "
		"image_id INTEGER, "
		"year INTEGER"
		//"UNIQUE(name, artist_id)"
		")");

static const QString TABLE_SQL_DFN_IMAGE("CREATE TABLE image ("
		"id INTEGER PRIMARY KEY, "
		"path VARCHAR, "
		"UNIQUE(path) "
		")");

static const QString TABLE_SQL_DFN_TRACK("CREATE TABLE track ("
		"id INTEGER PRIMARY KEY, "
		"dir_id INTEGER, "       // relatve path to track file
		"filename VARCHAR, "  // filename (without path?)
		"title VARCHAR, "     // track title
		"modtime INTEGER, "   // file modification time (unixtime)
		"artist_id INTEGER, " // artist id, ref "artist" table
		"genre_id INTEGER, "  // genre id, ref "genre" table
		"album_id INTEGER, "  // album id, ref "album"
		"track INTEGER, "     // track number in the album
		"length INTEGER, "    // track length
		"year INTEGER "       // track year
		")");
static const QString TABLE_SQL_DFN_TRACK_INDEX_ARTIST_ID("CREATE INDEX IF NOT EXISTS track_index_album_id "
		"ON track (album_id)");
static const QString TABLE_SQL_DFN_TRACK_INDEX_GENRE_ID("CREATE INDEX IF NOT EXISTS track_index_genre_id "
		"ON track (genre_id)");
static const QString TABLE_SQL_DFN_TRACK_INDEX_DIR_ID("CREATE INDEX IF NOT EXISTS track_index_dir_id "
		"ON track (dir_id)");

static const QString TABLE_SQL_DFN_COLLECTION("CREATE TABLE collection ("
		"id INTEGER PRIMARY KEY, "
		"name VARCHAR, " // human readable collection name, title
		"path VARCHAR, " // path to collection root directory
		"enabled INTEGER " // collection state (1|0), if enabled then collection will be scanned for updates
		")");

static const QString TABLE_SQL_DFN_DIR("CREATE TABLE dir ("
		"id INTEGER PRIMARY KEY, "
		"path VARCHAR, " // directory path
		"modtime INTEGER" // recorded directory modification time
		")");

QSqlError initDb() {
	// open database
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
	// TODO: replace with actual path to the database file
	db.setDatabaseName("collection.sqlite");

	if (!db.open()) {
		return db.lastError();
	}

	QStringList tables = db.tables();

	// attempt to create missing tables
	QSqlQuery q(db);
	if (!tables.contains("track")) {
		qDebug() << "create table 'track'";
		if (!q.exec(TABLE_SQL_DFN_TRACK)) {
			return q.lastError();
		}
		if (!q.exec(TABLE_SQL_DFN_TRACK_INDEX_ARTIST_ID)) {
			return q.lastError();
		}
	}
	if (!tables.contains("dir")) {
		qDebug() << "create table 'dir'";
		if (!q.exec(TABLE_SQL_DFN_DIR)) {
			return q.lastError();
		}
	}
	if (!tables.contains("artist")) {
		qDebug() << "create table 'artist'";
		if (!q.exec(TABLE_SQL_DFN_ARTIST)) {
			return q.lastError();
		}
	}
	if (!tables.contains("album")) {
		qDebug() << "create table 'album'";
		if (!q.exec(TABLE_SQL_DFN_ALBUM)) {
			return q.lastError();
		}
	}
	if (!tables.contains("image")) {
		qDebug() << "create table 'image'";
		if (!q.exec(TABLE_SQL_DFN_IMAGE)) {
			return q.lastError();
		}
	}
	if (!tables.contains("genre")) {
		qDebug() << "create table 'genre'";
		if (!q.exec(TABLE_SQL_DFN_GENRE)) {
			return q.lastError();
		}
	}
	if (!tables.contains("collection")) {
		qDebug() << "create table 'collection'";
		if (!q.exec(TABLE_SQL_DFN_COLLECTION)) {
			return q.lastError();
		}

		// init collection with come data
		if (!q.exec(
				//"INSERT INTO collection VALUES (3, 'collection 1', '/share/gate/data/music/Rock', 1)")) {
				"INSERT INTO collection VALUES (3, 'collection 1', '/share/gate/data/music/Ethno', 1)")) {
				//"INSERT INTO collection VALUES (3, 'collection 1', '/share/gate/data/music/Original', 1)")) {
				//"INSERT INTO collection VALUES (3, 'collection 1', '/share/gate/data/music', 1)")) {
			return q.lastError();
		}
	}

	return QSqlError();
}
