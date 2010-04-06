/*
 * scrobbleradapter.cpp
 *
 *  Created on: Mar 31, 2010
 *      Author: Sergei Stolyarov
 */

#include <math.h>
#include <QtDebug>
#include <lastfm/ws.h>
#include <lastfm/Audioscrobbler>
#include <lastfm/Track>

#include "scrobbleradapter.h"

struct Ororok::lastfm::ScrobblerAdapter::Private
{
	::lastfm::Audioscrobbler * scrobbler;
	::lastfm::MutableTrack currentTrack;
};

Ororok::lastfm::ScrobblerAdapter::ScrobblerAdapter(QObject * parent)
	: QObject(parent)
{
	p = new Private;
	// TODO: replace "tst" with real client-id
	p->scrobbler = new ::lastfm::Audioscrobbler("oro");
	//qDebug() << "::lastfm::ws::SessionKey" << ::lastfm::ws::SessionKey;
	//qDebug() << "::lastfm::ws::Username" << ::lastfm::ws::Username;
}

Ororok::lastfm::ScrobblerAdapter::~ScrobblerAdapter()
{
	delete p->scrobbler;
	delete p;
}

void Ororok::lastfm::ScrobblerAdapter::nowPlaying(const QString & title, const QString & artist, const QString & album, uint duration)
{
	p->currentTrack.stamp();
	p->currentTrack.setTitle(title);
	p->currentTrack.setDuration(duration);
	if (!artist.isEmpty()) {
		p->currentTrack.setArtist(artist);
	}
	if (!album.isEmpty()) {
		p->currentTrack.setAlbum(album);
	}

	p->currentTrack.setSource(::lastfm::Track::Player);
	if (!p->currentTrack.isNull()) {
		p->scrobbler->nowPlaying(p->currentTrack);
	}
}

void Ororok::lastfm::ScrobblerAdapter::submit(const QString & title, const QString & artist, const QString & album,
		uint duration, uint trackNum, QDateTime timeStarted)
{
	p->currentTrack.stamp();
	p->currentTrack.setTitle(title);
	p->currentTrack.setDuration(duration);
	if (trackNum > 0) {
		p->currentTrack.setTrackNumber(trackNum);
	}

	if (!artist.isEmpty()) {
		p->currentTrack.setArtist(artist);
	}
	if (!album.isEmpty()) {
		p->currentTrack.setAlbum(album);
	}

	p->currentTrack.setSource(::lastfm::Track::Player);
	if (!p->currentTrack.isNull()) {
		// check that submit is allowed
		// submitted track must be played at least "duration/2"
		// calc difference, in seconds
		uint diff = abs( (timeStarted.toTime_t() - QDateTime::currentDateTime().toTime_t()) );
		if (duration > 30 && diff > duration/2) {
			p->scrobbler->cache(p->currentTrack);
			p->scrobbler->submit();
		}
	}
}
