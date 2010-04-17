/*
 * formats.h
 *
 *  Created on: Oct 20, 2009
 *      Author: Sergei Stolyarov
 */

#ifndef FORMATS_H_
#define FORMATS_H_

#include <QStringList>

namespace Ororok
{

struct MusicTrackMetadata
{
	QString artist;
	QString title;
	QString album;
	QString genre;
	QString comment;
	QString lyricsAuthor;
	QString composer;
	int track;
	int year;
	int bitrate;
	int length;
};

QStringList supportedFileExtensions();
MusicTrackMetadata * getMusicFileMetadata(const QString & filename);

}

#endif /* FORMATS_H_ */
