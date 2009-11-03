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

	// connect signals: tick, stateChanged etc
	p->mediaObject->setTickInterval(1000);
	connect(p->mediaObject, SIGNAL(tick(qint64)), this, SLOT(tick(qint64)));

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

void Player::stopTrackPlay()
{

}

void Player::startTrackPlay(const QStringList & trackInfo)
{
	Phonon::MediaSource f(trackInfo[Ororok::TrackFieldPath]);
	//qDebug() << "start track " << trackFile;
	p->mediaObject->clear();
	p->mediaObject->enqueue(f);
	p->mediaObject->play();
}

void Player::tick(qint64 time)
{
	qint64 totalTime = p->mediaObject->totalTime();
	// emit signal with track time
	emit trackTimeChanged(time, totalTime);
}

Player * Player::instance()
{
	if (Player::inst == 0) {
		Player::inst = new Player();
	}

	return Player::inst;
}
