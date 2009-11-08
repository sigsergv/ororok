/*
 * player.h
 *
 *  Created on: Nov 1, 2009
 *      Author: Sergei Stolyarov
 */

#ifndef PLAYER_H_
#define PLAYER_H_

#include <QObject>
#include <Phonon/Global>

namespace Phonon
{
struct SeekSlider;
struct VolumeSlider;
}

class Player : public QObject
{
	Q_OBJECT
public:
	static Player * instance();
	Phonon::SeekSlider * seekSlider();
	Phonon::VolumeSlider * volumeSlider();
	Phonon::State state();

signals:
	void trackTimeChanged(qint64 time, qint64 totalTime);

public slots:
	void stop();
	void pause();
	void play();
	void start(const QStringList & trackInfo);

protected slots:
	void tick(qint64 time);

private:
	struct Private;
	Private * p;
	static Player * inst;
	Player();
};

#endif /* PLAYER_H_ */
