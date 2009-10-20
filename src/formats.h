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
	QString album;
	QString genre;
	int track;
	int year;
	int bitrate;
};

QStringList supportedFileExtensions();
MusicTrackMetadata * getMusicFileMetadata(const QString & filename, bool & success);

}

#endif /* FORMATS_H_ */
