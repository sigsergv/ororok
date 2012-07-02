/*
 * scrobbleradapter.cpp
 *
 *  Created on: Mar 31, 2010
 *      Author: Sergei Stolyarov
 */

#include <math.h>
#include <QtDebug>
#include "qdebugreleaseworkaround.h"
#include <lastfm/ws.h>
#include <lastfm/Audioscrobbler>
#include <lastfm/Track>
#include "scrobbleradapter.h"
#include "lastfm.h"

struct Ororok::lastfm::ScrobblerAdapter::Private
{
	::lastfm::Audioscrobbler * scrobbler;
	::lastfm::MutableTrack currentTrack;
};

Ororok::lastfm::ScrobblerAdapter::ScrobblerAdapter(QObject * parent)
	: QObject(parent)
{
	p = new Private;

	if (!Ororok::lastfm::isSubmitEnabled()) {
		p->scrobbler = 0;
		return;
	}

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
	if (!Ororok::lastfm::isSubmitEnabled()) {
		return;
	}

	qDebug() << "send \"now playing\" notification to Last.fm:" << title;

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

void Ororok::lastfm::ScrobblerAdapter::love(const QString & title, const QString & artist, const QString & album)
{
	if (!Ororok::lastfm::isSubmitEnabled()) {
		return;
	}

	qDebug() << "Love lastfm track" << title << artist << album;

	p->currentTrack.stamp();
	p->currentTrack.setTitle(title);
	if (!artist.isEmpty()) {
		p->currentTrack.setArtist(artist);
	}
	if (!album.isEmpty()) {
		p->currentTrack.setAlbum(album);
	}

	p->currentTrack.setSource(::lastfm::Track::Player);
	if (!p->currentTrack.isNull()) {
		p->currentTrack.love();
	}
}

void Ororok::lastfm::ScrobblerAdapter::submit(const QString & title, const QString & artist, const QString & album,
		uint duration, uint trackNum, QDateTime timeStarted)
{
	if (!Ororok::lastfm::isSubmitEnabled()) {
		return;
	}

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
