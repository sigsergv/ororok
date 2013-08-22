/*
 * xmlplaylistreader.cpp
 *
 *  Created on: Apr 16, 2010
 *      Author: Sergey Stolyarov
 */

#include <QtXml>
#include <QtDebug>

#include "xmlplaylistreader.h"
#include "mimetrackinfo.h"

/*
 * <playlist>
 * <info>
 *   <name></name>
 * </info>
 * <tracks>
 *   <track num="21" title="Some song name" artist="artist name" album="album"
 *       year="1993" genre="Rock" length="213">
 *     TRACK_FILE
 *   </track>
 * </tracks>
 * </playlist>
 */

struct Ororok::XmlPlaylistReader::Private
{
	QXmlStreamReader * xml;

	// playlist details
	QString name;
	QList<QStringList> tracks;
};

Ororok::XmlPlaylistReader::XmlPlaylistReader(QIODevice *  device)
{
	p = new Private;
	p->xml = new QXmlStreamReader(device);

	if (p->xml->readNextStartElement()) {
		if (p->xml->name() == "playlist" && p->xml->attributes().value("version") == "1.0") {
			readPlaylist();
		}
	}
}

Ororok::XmlPlaylistReader::~XmlPlaylistReader()
{
	delete p->xml;
	delete p;
}

/*
 * process file and read playlist info and
 */
QList<QStringList> Ororok::XmlPlaylistReader::tracks()
{
	return p->tracks;
}

QString Ororok::XmlPlaylistReader::name()
{
	return p->name;
}

void Ororok::XmlPlaylistReader::readPlaylist()
{
	while (p->xml->readNextStartElement()) {
		if (p->xml->name() == "info") {
			readInfo();
		} else if (p->xml->name() == "tracks") {
			readTracks();
		}
	}
}

void Ororok::XmlPlaylistReader::readInfo()
{
	while (p->xml->readNextStartElement()) {
		if (p->xml->name() == "name") {
			p->name = p->xml->readElementText();
		} else {
			p->xml->skipCurrentElement();
		}
	}
}

void Ororok::XmlPlaylistReader::readTracks()
{

	while (p->xml->readNextStartElement()) {
		if (p->xml->name() == "track") {
			QStringList trackInfo = Ororok::emptyTrackInfo();

			trackInfo[Ororok::TrackFieldNo] = p->xml->attributes().value("num").toString();
			trackInfo[Ororok::TrackFieldTitle] = p->xml->attributes().value("title").toString();
			trackInfo[Ororok::TrackFieldYear] = p->xml->attributes().value("year").toString();
			trackInfo[Ororok::TrackFieldAlbum] = p->xml->attributes().value("album").toString();
			trackInfo[Ororok::TrackFieldArtist] = p->xml->attributes().value("artist").toString();
			trackInfo[Ororok::TrackFieldGenre] = p->xml->attributes().value("genre").toString();
			trackInfo[Ororok::TrackFieldLength] = p->xml->attributes().value("length").toString();

			trackInfo[Ororok::TrackFieldPath] = p->xml->readElementText();
			p->tracks.append(trackInfo);
		} else {
			p->xml->skipCurrentElement();
		}
	}
}
