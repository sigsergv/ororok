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
static QString uiLangsPath;

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
			ini.copy(newProfilePath + "/ororok.ini");
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

	#ifdef QT_NO_DEBUG
	// and also try to migrate collection database, only for non-debug version
	QFile newCollectionDb(newProfilePath + "/collection.db");
	QFile oldCollectionDb("collection.sqlite");

	if (!newCollectionDb.exists() && oldCollectionDb.exists()) {
		oldCollectionDb.rename(newProfilePath + "/collection.db");
	}
	#endif
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

	#ifdef QT_NO_DEBUG
	QString path = QDir::homePath() + "/.config/ororok";
	#else
	QString path = QDir::currentPath() + "/.debug-config-dir";
	#endif
	if (!dir.exists(path) && !dir.mkpath(path)) {
		// TODO: do something
		return QString();
	}

	return path;
}

QString uiLang()
{
    QString lang = "en";
    QString langEnv = qgetenv("LANG");

    if (langEnv.contains("ru_RU")) {
        lang = "ru";
    }

    return lang;
	
}

QString uiLangsPath()
{
    // first check for local paths
    QString localPath = QCoreApplication::applicationDirPath() + QDir::separator() + "translations";
    QDir d(localPath);
    QStringList files = d.entryList(QDir::Files);
    foreach (QString f, files) {
        if (f.endsWith(".qm")) {
            ::uiLangsPath = localPath;
            break;
        }
    }

    // find directory with translations
    if (::uiLangsPath.isEmpty()) {
#ifdef Q_OS_UNIX
        // check standard dirs
        QStringList checkPaths;
        checkPaths << "/usr/share/ororok/translations/";

        foreach (QString path, checkPaths) {
            QDir d(path);
            bool found = false;
            if (d.exists()) {
                // check for *.qm files there
                QStringList files = d.entryList(QDir::Files);
                foreach (QString f, files) {
                    if (f.endsWith(".qm")) {
                        ::uiLangsPath = path;
                        found = true;
                        break;
                    }
                }
                if (found) {
                    break;
                }
            }
        }
#endif
    }
    return ::uiLangsPath;
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
