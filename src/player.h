/*
 * player.h
 *
 *  Created on: Nov 1, 2009
 *      Author: Sergei Stolyarov
 */

#ifndef PLAYER_H_
#define PLAYER_H_

#include <QObject>

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

signals:
	void trackTimeChanged(qint64 time, qint64 totalTime);

public slots:
	void stopTrackPlay();
	void startTrackPlay(const QStringList & trackInfo);

protected slots:
	void tick(qint64 time);

private:
	struct Private;
	Private * p;
	static Player * inst;
	Player();
};

#endif /* PLAYER_H_ */
