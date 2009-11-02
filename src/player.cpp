/*
 * player.cpp
 *
 *  Created on: Nov 1, 2009
 *      Author: Sergei Stolyarov
 */
#include <QtCore>
#include <Phonon/MediaObject>
#include <Phonon/AudioOutput>
#include <QtDebug>

#include "player.h"

Player * Player::inst = 0;

struct Player::Private
{
	Phonon::AudioOutput * audioOutput;
	Phonon::MediaObject * mediaObject;
	QList<Phonon::MediaSource> mediaSources;
};

Player::Player()
{
	p = new Private;
	p->audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
	p->mediaObject = new Phonon::MediaObject(this);

	// connect signals: tick, stateChanged etc

	Phonon::createPath(p->mediaObject, p->audioOutput);
}

void Player::stopTrackPlay()
{

}

void Player::startTrackPlay(const QStringList & trackInfo)
{
	qDebug() << "start track " << trackInfo;
}

Player * Player::instance()
{
	if (Player::inst == 0) {
		Player::inst = new Player();
	}

	return Player::inst;
}
