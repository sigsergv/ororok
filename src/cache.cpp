/*
 * cache.cpp
 *
 *  Created on: Oct 24, 2009
 *      Author: Sergey Stolyarov
 */
#include <QtCore>
#include <QCryptographicHash>

#include "cache.h"
#include "settings.h"

typedef QByteArray CacheHashKeyType;
typedef QHash<CacheHashKeyType, QPixmap> ImagesCache;

static ImagesCache cachedIconImages;
static ImagesCache cachedNormalImages;
static QHash<QString,CacheHashKeyType> hashesHash;
static QImage defaultIconImg(Ororok::ALBUM_ICON_SIZE, Ororok::ALBUM_ICON_SIZE, QImage::Format_Mono);

namespace Ororok
{

/**
 * load image from the file and create corresponding entries, return
 * entry for size size
 *
 * @param path
 * @param hashKey
 */
QPixmap _loadIconImage(const QString & path, const CacheHashKeyType & hashKey)
{
	QImage iconImg;

	// check that icon is cached on disk
	QString cachedIconPath = imagesCachePath() + "/i-" + QString(hashKey);
	QFile cachedIconFile(cachedIconPath);
	if (!cachedIconFile.exists()) {
		// try to create icon file

		// load original image...
		QImage img(path);
		if (img.isNull()) {
			return QPixmap();
		}

		// ... and resize it to icon size (maximum dimension must be ALBUM_ICON_SIZE pixels)
		if (img.width() > img.height()) {
			iconImg = img.scaledToWidth(ALBUM_ICON_SIZE, Qt::SmoothTransformation);
		} else {
			iconImg = img.scaledToHeight(ALBUM_ICON_SIZE, Qt::SmoothTransformation);
		}
		// finally save data to the file
		if (cachedIconFile.open(QIODevice::WriteOnly)) {
			iconImg.save(&cachedIconFile, "JPG");
		} else {
			iconImg = defaultIconImg;
		}
		cachedIconFile.close();
	} else {
		// file exists in the disk cache so try to load it from there
		if (cachedIconFile.open(QIODevice::ReadOnly)) {
			iconImg.load(&cachedIconFile, "JPG");
		} else {
			iconImg = defaultIconImg;
		}
	}

	QPixmap p = QPixmap::fromImage(iconImg);
	// save icon to the icons memory cache
	cachedIconImages[hashKey] = p;

	return p;
}

QPixmap cachedImage(const QString & path, CachedImageSize size)
{
	QPixmap p;
	CacheHashKeyType hashKey = hashesHash.value(path);

	if (hashKey.isEmpty()) {
		qDebug() << "generate hash" << hashKey;
		hashKey = QCryptographicHash::hash(path.toAscii(), QCryptographicHash::Md5).toHex();
		hashesHash[path] = hashKey;
	}

	switch (size) {
	case ImageSizeIcon:
		// check that image exists in the cache
		p = cachedIconImages.value(hashKey);
		if (p.isNull()) {
			qDebug() << "load icon with hash " << hashKey;
			p = _loadIconImage(path, hashKey);
		}
		return p;

		// load
		break;

	case ImageSizeNormal:
		break;

	case ImageSizeOriginal:
		break;
	}

	return QPixmap();
}

}
