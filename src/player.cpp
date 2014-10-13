/*
 * player.cpp
 *
 *  Created on: Nov 1, 2009
 *      Author: Sergey Stolyarov
 */
#include <QtCore>
#include <QtWidgets>
#include <QtMultimedia>
#include <QtDebug>
 
// #include <Phonon/MediaObject>
// #include <Phonon/MediaSource>
// #include <Phonon/AudioOutput>
// #include <Phonon/SeekSlider>
// #include <Phonon/VolumeSlider>

#include "player.h"
#include "mimetrackinfo.h"

Player * Player::inst = 0;

struct Player::Private
{
	QAudioOutput * audioOutput;
	QMediaPlayer * mediaPlayer;
	// QList<Phonon::MediaSource> mediaSources;
	QAbstractSlider * seekSlider;
	QAbstractSlider * volumeSlider;

	qint32 tickInterval;
	QStringList nextTrack;
	QStringList currentTrack;
	QDateTime currentTrackStartTime;
	qint64 currentTrackPlayingTime;
	bool midTrackReached;
};

Player::Player()
{
	p = new Private;
	p->audioOutput = new QAudioOutput(QAudioFormat(), this);
	p->mediaPlayer = new QMediaPlayer(this);

	// QMediaPlaylist * playlist = new QMediaPlaylist();
	p->mediaPlayer->setPlaylist(new QMediaPlaylist());


	p->seekSlider = new QSlider(Qt::Horizontal);
	p->seekSlider->setRange(0, 300);
	// p->seekSlider->setMediaObject(p->mediaPlayer);
	// p->seekSlider->setIconVisible(false);
	p->seekSlider->setFocusPolicy(Qt::NoFocus);

	p->volumeSlider = new QSlider(Qt::Horizontal);
	p->volumeSlider->setRange(0, 100);
	// p->volumeSlider->setAudioOutput(p->audioOutput);
	p->volumeSlider->setMaximumWidth(200);
	p->volumeSlider->setFocusPolicy(Qt::NoFocus);

	p->midTrackReached = false;

	// connect signals: tick, stateChanged etc
	// p->mediaPlayer->setTickInterval(1000);
	// connect(p->mediaPlayer, SIGNAL(tick(qint64)), this, SLOT(tick(qint64)));
	connect(p->mediaPlayer, SIGNAL(aboutToFinish()), this, SLOT(almostFinished()));
	connect(p->mediaPlayer, SIGNAL(currentSourceChanged(const Phonon::MediaSource &)),
			this, SLOT(sourceChanged(const Phonon::MediaSource &)));
	connect(p->volumeSlider, SIGNAL(sliderMoved(int)), p->mediaPlayer, SLOT(setVolume(int)));
	connect(p->seekSlider, SIGNAL(sliderMoved(int)), this, SLOT(setPosition(int)));

	// Phonon::createPath(p->mediaPlayer, p->audioOutput);
	// p->tickInterval = p->mediaPlayer->tickInterval();
	p->tickInterval = 0;
}

QAbstractSlider * Player::seekSlider()
{
	return p->seekSlider;
}

QAbstractSlider * Player::volumeSlider()
{
	return p->volumeSlider;
}

QMediaPlayer::State Player::state()
{
	return p->mediaPlayer->state();
}

void Player::stop()
{
	p->mediaPlayer->stop();
}

void Player::pause()
{
	p->mediaPlayer->pause();
}

void Player::play()
{
	p->mediaPlayer->play();
}

void Player::start(const QStringList & trackInfo)
{
	QMediaContent f(trackInfo[Ororok::TrackFieldPath]);
	// f.setAutoDelete(true);
	//qDebug() << "start track " << trackFile;
	// p->mediaPlayer->clear();
	// p->mediaPlayer->enqueue(f);
	p->mediaPlayer->setMedia(f);
	p->mediaPlayer->play();
	p->currentTrack = trackInfo;
	p->currentTrackStartTime = QDateTime::currentDateTime();
	p->nextTrack.clear();
	p->midTrackReached = false;
	p->currentTrackPlayingTime = 0;
	emit trackChanged(trackInfo);
}

void Player::enqueue(const QStringList & trackInfo)
{
	QMediaContent f(trackInfo[Ororok::TrackFieldPath]);
	// f.setAutoDelete(true);
	p->mediaPlayer->playlist()->addMedia(f);
	p->nextTrack = trackInfo;
}

void Player::tick(qint64 posTime)
{
	Q_UNUSED(posTime);
	/* // disabled for now
	qint64 totalTime = p->mediaPlayer->totalTime();
	p->currentTrackPlayingTime += p->tickInterval;

	// emit signal with track time
	emit trackTimeChanged(posTime, totalTime);
	if (!p->midTrackReached && p->currentTrackPlayingTime >= 4000+totalTime/2) { // 4000 - 4 seconds
		emit midTrackReached(p->currentTrack, p->currentTrackStartTime);
		p->midTrackReached = true;
	}
	*/
}

void Player::almostFinished()
{
	emit nextTrackNeeded();
	if (!p->midTrackReached) {
		emit midTrackReached(p->currentTrack, p->currentTrackStartTime);
		p->midTrackReached = true;
	}
}

void Player::sourceChanged(const Phonon::MediaSource &)
{
	if (!p->nextTrack.isEmpty()) {
		qDebug() << "playing next track, emit signal";
		p->currentTrack = p->nextTrack;
		emit trackChanged(p->nextTrack);
	}
	p->midTrackReached = false;
	p->currentTrackPlayingTime = 0;
	p->nextTrack.clear();
}

void Player::setPosition(int pos)
{
	Q_UNUSED(pos);
}

Player * Player::instance()
{
	if (Player::inst == 0) {
		Player::inst = new Player();
	}

	return Player::inst;
}
