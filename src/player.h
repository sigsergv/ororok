/*
 * player.h
 *
 *  Created on: Nov 1, 2009
 *      Author: Sergey Stolyarov
 */

#ifndef PLAYER_H_
#define PLAYER_H_

#include <QObject>
#include <QMediaPlayer>
// #include <Phonon/Global>

namespace Phonon
{
struct SeekSlider;
struct VolumeSlider;
struct MediaSource;
}

class QAbstractSlider;

class Player : public QObject
{
	Q_OBJECT
public:
	static Player * instance();
	QAbstractSlider * seekSlider();
	QAbstractSlider * volumeSlider();
	QMediaPlayer::State state();

signals:
	void trackTimeChanged(qint64 time, qint64 totalTime);
	void nextTrackNeeded();
	void trackChanged(const QStringList & trackInfo);
	void midTrackReached(const QStringList & trackInfo, const QDateTime & startTime); // emitted when middle of playing track is reached

public slots:
	void stop();
	void pause();
	void play();
	void start(const QStringList & trackInfo);
	void enqueue(const QStringList & trackInfo);

protected slots:
	void tick(qint64 time);
	void almostFinished();
	void sourceChanged(const Phonon::MediaSource &);
	void setPosition(int);

private:
	struct Private;
	Private * p;
	static Player * inst;
	Player();
};

#endif /* PLAYER_H_ */
