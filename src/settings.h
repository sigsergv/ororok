/*
 * settings.h
 *
 *  Created on: Oct 24, 2009
 *      Author: Sergei Stolyarov
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <QString>
#include <QChar>
#include <QList>

struct QSettings;

namespace Ororok
{

const int ALBUM_ICON_SIZE = 35;
const int ALBUM_ITEM_PADDING = 2;

QString imagesCachePath();
QString playlistsStorePath();
QString tmpPlaylistsStorePath();
QString profilePath();
QString uiLang();
QString uiLangsPath();
void initSettings();
QSettings * settings();
QString version();

/**
 * generate unique (in the session) value
 *
 * @return
 */
int generateUid();
}


#endif /* SETTINGS_H_ */
