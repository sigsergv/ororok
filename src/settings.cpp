/*
 * settings.cpp
 *
 *  Created on: Oct 24, 2009
 *      Author: Sergei Stolyarov
 */

#include <QtCore>

#include "settings.h"

static QString _imagesCachePath;
static int lastUid = 0;

namespace Ororok
{

QString profilePath()
{
	QDir dir;

	QString path = QDir::homePath() + "/.ororok";
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

}
