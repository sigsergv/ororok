/*
 * settings.cpp
 *
 *  Created on: Oct 24, 2009
 *      Author: Sergei Stolyarov
 */

#include <QtCore>

#include "settings.h"

static QString _imagesCachePath;
static QString _playlistsPath;
static QString _tmpPlaylistsPath;
static int lastUid = 0;
static QSettings * _settings = 0;

namespace Ororok
{

void initSettings()
{
	QCoreApplication::setApplicationName("Ororok");
	QCoreApplication::setApplicationVersion(OROROK_VERSION);
	//QCoreApplication::setApplicationVersion("1.0"); // required for lastfm
	QCoreApplication::setOrganizationName("regolit.com");
	QCoreApplication::setOrganizationDomain("ororok.regolit.com");

	// perform settings migration, from "~/.ororok" to "~/.config/ororok"
	// move all contents except "images-cache"
	QString newProfilePath = QDir::homePath() + "/.config/ororok";
	QString oldProfilePath = QDir::homePath() + "/.ororok";

	QDir dir;

	if (!dir.exists(newProfilePath) && dir.exists(oldProfilePath)) {
		// migrate settings
		dir.mkpath(newProfilePath);
		QFile ini(oldProfilePath + "/ororok.ini");
		if (ini.exists()) {
			ini.copy(newProfilePath + "/ororok.ini"); // move instead of copy
		}
	}

	// also copy (recursively) directory "tmp-playlists-store"
	if (!dir.exists(newProfilePath + "/tmp-playlists-store") 
		&& dir.exists(oldProfilePath + "/tmp-playlists-store") )
	{
		QDir d(oldProfilePath + "/tmp-playlists-store");
		dir.mkpath(newProfilePath + "/tmp-playlists-store");
		QStringList files = d.entryList(QDir::Files);
		foreach (const QString & fn, files) {
			QFile::copy(oldProfilePath + "/tmp-playlists-store/" + fn, 
				newProfilePath + "/tmp-playlists-store/" + fn);
		}
	}

	// and also try to migrate collection database
	QFile newCollectionDb(newProfilePath + "/collection.db");
	QFile oldCollectionDb("collection.db");

	if (!newCollectionDb.exists() && oldCollectionDb.exists()) {
		oldCollectionDb.copy(newProfilePath + "/collection.db");
	}
}

QSettings * settings()
{
	if (0 == _settings) {
		_settings = new QSettings(profilePath()+"/ororok.ini", QSettings::IniFormat);
	}
	return _settings;
}

QString version()
{
	return QString(OROROK_VERSION);
}

QString profilePath()
{
	QDir dir;

	QString path = QDir::homePath() + "/.config/ororok";
	if (!dir.exists(path) && !dir.mkpath(path)) {
		// TODO: do something
		return QString();
	}

	return path;
}

int generateUid()
{
	::lastUid++;
	return ::lastUid;
}

QString imagesCachePath()
{
	if (_imagesCachePath.isEmpty()) {
		// check that directory exists and create it if required
		QDir dir;
		QString path = profilePath() + "/images-cache";
		if (!dir.exists(path) && !dir.mkpath(path)) {
			return QString();
		}

		_imagesCachePath = path;
	}

	return _imagesCachePath;
}

QString playlistsStorePath()
{
	if (_playlistsPath.isEmpty()) {
		QDir dir;
		QString path = profilePath() + "/playlists-store";
		if (!dir.exists(path) && !dir.mkpath(path)) {
			return QString();
		}

		_playlistsPath = path;
	}

	return _playlistsPath;
}

QString tmpPlaylistsStorePath()
{
	if (_tmpPlaylistsPath.isEmpty()) {
		QDir dir;
		QString path = profilePath() + "/tmp-playlists-store";
		if (!dir.exists(path) && !dir.mkpath(path)) {
			return QString();
		}

		_tmpPlaylistsPath = path;
	}

	return _tmpPlaylistsPath;
}



}
