/*
 * formats.cpp
 *
 *  Created on: Oct 20, 2009
 *      Author: Sergei Stolyarov
 */
#include "formats.h"
#include <fileref.h>
#include <mpegfile.h>
#include <tag.h>

#define S2Q(s) QString::fromUtf8(s.toCString(true))

QStringList Ororok::supportedFileExtensions()
{
	QStringList res;

	res << "mp3";

	return res;
}

Ororok::MusicTrackMetadata * Ororok::getMusicFileMetadata(const QString & filename, bool & success)
{
	// try to open file and read metadata from there
	TagLib::FileRef f(filename.toLocal8Bit().constData());

	if (f.isNull() || !f.audioProperties()) {
		return 0;
	}

	TagLib::Tag * tag = f.tag();
	Ororok::MusicTrackMetadata * metadata = new Ororok::MusicTrackMetadata;

	metadata->artist = S2Q(tag->artist());
	metadata->album = S2Q(tag->album());
	metadata->genre = S2Q(tag->genre());
	metadata->year = tag->year();
	metadata->track = tag->track();
	return metadata;
}
