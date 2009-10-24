/*
 * cache.h
 *
 *  Created on: Oct 24, 2009
 *      Author: Sergei Stolyarov
 */

#ifndef CACHE_H_
#define CACHE_H_

#include <QPixmap>

namespace Ororok
{
enum CachedImageSize { ImageSizeIcon, ImageSizeNormal, ImageSizeOriginal };

QPixmap cachedImage(const QString & path, CachedImageSize size);

}
#endif /* CACHE_H_ */
