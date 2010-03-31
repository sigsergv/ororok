/*
 * player.cpp
 *
 *  Created on: Nov 1, 2009
 *      Author: Sergei Stolyarov
 */
#include <QtCore>
#include <Phonon/MediaObject>
#include <Phonon/MediaSource>
#include <Phonon/AudioOutput>
#include <Phonon/SeekSlider>
#include <Phonon/VolumeSlider>
#include <QtDebug>

#include "player.h"
#include "mimetrackinfo.h"

Player * Player::inst = 0;

struct Player::Private
{
	Phonon::AudioOutput * audioOutput;
	Phonon::MediaObject * mediaObject;
	QList<Phonon::MediaSource> mediaSources;
	Phonon::SeekSlider * seekSlider;
	Phonon::VolumeSlider * volumeSlider;

	QStringList nextTrack;
	QStringList currentTrack;
	bool midTrackReached;
};

Player::Player()
{
	p = new Private;
	p->audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
	p->mediaObject = new Phonon::MediaObject(this);
	p->seekSlider = new Phonon::SeekSlider;
	p->seekSlider->setMediaObject(p->mediaObject);
	p->seekSlider->setIconVisible(false);
	p->seekSlider->setFocusPolicy(Qt::NoFocus);
	p->volumeSlider = new Phonon::VolumeSlider;
	p->volumeSlider->setAudioOutput(p->audioOutput);
	p->volumeSlider->setMaximumWidth(200);
	p->volumeSlider->setFocusPolicy(Qt::NoFocus);
	p->midTrackReached = false;

	// connect signals: tick, stateChanged etc
	p->mediaObject->setTickInterval(1000);
	connect(p->mediaObject, SIGNAL(tick(qint64)), this, SLOT(tick(qint64)));
	connect(p->mediaObject, SIGNAL(aboutToFinish()), this, SLOT(almostFinished()));
	connect(p->mediaObject, SIGNAL(currentSourceChanged(const Phonon::MediaSource &)),
			this, SLOT(sourceShanged(const Phonon::MediaSource &)));

	Phonon::createPath(p->mediaObject, p->audioOutput);
}

Phonon::SeekSlider * Player::seekSlider()
{
	return p->seekSlider;
}

Phonon::VolumeSlider * Player::volumeSlider()
{
	return p->volumeSlider;
}

Phonon::State Player::state()
{
	return p->mediaObject->state();
}

void Player::stop()
{
	p->mediaObject->stop();
}

void Player::pause()
{
	p->mediaObject->pause();
}

void Player::play()
{
	p->mediaObject->play();
}

void Player::start(const QStringList & trackInfo)
{
	Phonon::MediaSource f(trackInfo[Ororok::TrackFieldPath]);
	f.setAutoDelete(true);
	//qDebug() << "start track " << trackFile;
	p->mediaObject->clear();
	p->mediaObject->enqueue(f);
	p->mediaObject->play();
	p->currentTrack = trackInfo;
	p->nextTrack.clear();
	p->midTrackReached = false;
	emit trackChanged(trackInfo);
}

void Player::enqueue(const QStringList & trackInfo)
{
	Phonon::MediaSource f(trackInfo[Ororok::TrackFieldPath]);
	f.setAutoDelete(true);
	p->mediaObject->enqueue(f);
	p->nextTrack = trackInfo;
}

void Player::tick(qint64 time)
{
	qint64 totalTime = p->mediaObject->totalTime();
	// emit signal with track time
	emit trackTimeChanged(time, totalTime);
	if (!p->midTrackReached && time >= totalTime/2) {
		emit midTrackReached(p->currentTrack);
		p->midTrackReached = true;
	}
}

void Player::almostFinished()
{
	emit nextTrackNeeded();
	if (!p->midTrackReached) {
		emit midTrackReached(p->currentTrack);
		p->midTrackReached = true;
	}
}

void Player::sourceShanged(const Phonon::MediaSource &)
{
	if (!p->nextTrack.isEmpty()) {
		qDebug() << "playing next track, emit signal";
		p->currentTrack = p->nextTrack;
		emit trackChanged(p->nextTrack);
	}
	p->nextTrack.clear();
}

Player * Player::instance()
{
	if (Player::inst == 0) {
		Player::inst = new Player();
	}

	return Player::inst;
}
