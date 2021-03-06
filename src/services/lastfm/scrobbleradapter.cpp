/*
 * scrobbleradapter.cpp
 *
 *  Created on: Mar 31, 2010
 *      Author: Sergey Stolyarov
 */

#include <math.h>
#include <QtDebug>
#include "qdebugreleaseworkaround.h"
#include <lastfm/ws.h>
#include <lastfm/Audioscrobbler.h>
#include <lastfm/Track.h>
#include "scrobbleradapter.h"
#include "lastfm.h"

struct Ororok::lastfm::ScrobblerAdapter::Private
{
public:
    Private()
        : m_scrobbler(0)
    {}

    ~Private()
    {
        delete m_scrobbler;
    }

    ::lastfm::Audioscrobbler * scrobbler()
    {
        if (m_scrobbler && ::lastfm::ws::Username.isEmpty()) {
            // destroy scrobbler
            delete m_scrobbler;
            m_scrobbler = 0;
        } else if (!m_scrobbler && !::lastfm::ws::Username.isEmpty()) {
            // re-init scrobbler
            m_scrobbler = new ::lastfm::Audioscrobbler("oro");
        }

        return m_scrobbler;
    }
    ::lastfm::MutableTrack currentTrack;

private:
    ::lastfm::Audioscrobbler * m_scrobbler;
};

Ororok::lastfm::ScrobblerAdapter::ScrobblerAdapter(QObject * parent)
    : QObject(parent), p(new Private)
{
}

Ororok::lastfm::ScrobblerAdapter::~ScrobblerAdapter()
{
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
    if (!p->currentTrack.isNull() && p->scrobbler()) {
        p->scrobbler()->nowPlaying(p->currentTrack);
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
        if (p->scrobbler() && duration > 30 && diff > duration/2) {
            p->scrobbler()->cache(p->currentTrack);
            p->scrobbler()->submit();
		}
	}
}
