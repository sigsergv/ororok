/*
 * formats.cpp
 *
 *  Created on: Oct 20, 2009
 *      Author: Sergei Stolyarov
 */
#include <QtDebug>

#include "formats.h"
#include <fileref.h>
#include <mpegfile.h>
#include <flacfile.h>
#include <id3v2tag.h>
#include <tag.h>

#define S2Q(s) QString::fromUtf8(s.toCString(true))

QStringList Ororok::supportedFileExtensions()
{
	QStringList res;

	res << "mp3";

	return res;
}

/**
 * Read track metadata from the file.
 */
Ororok::MusicTrackMetadata * Ororok::getMusicFileMetadata(const QString & filename)
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
	metadata->title = S2Q(tag->title());
	metadata->comment = S2Q(tag->comment());
	metadata->year = tag->year();
	metadata->track = tag->track();
	metadata->length = f.audioProperties()->length();

	TagLib::MPEG::File * mf = dynamic_cast<TagLib::MPEG::File*>(f.file());
	TagLib::MPEG::Properties * ap = mf->audioProperties();

	metadata->bitrate = ap->bitrate();

	if (mf->ID3v2Tag(false)) {
		TagLib::ID3v2::FrameList l = mf->ID3v2Tag(false)->frameListMap()["TCOM"];
		if (!l.isEmpty()) {
			metadata->composer = S2Q(l.front()->toString());
		}

		l = mf->ID3v2Tag(false)->frameListMap()["TEXT"];
		if (!l.isEmpty()) {
			metadata->lyricsAuthor = S2Q(l.front()->toString());
		}

		l = mf->ID3v2Tag(false)->frameListMap()["GENRE"];
		if (!l.isEmpty()) {
			metadata->genre = S2Q(l.front()->toString());
		}
	}
	return metadata;
}
