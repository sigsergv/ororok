/*
 * player.h
 *
 *  Created on: Nov 1, 2009
 *      Author: Sergei Stolyarov
 */

#ifndef PLAYER_H_
#define PLAYER_H_

#include <QObject>

class Player : public QObject
{
	Q_OBJECT
public:
	static Player * instance();

public slots:
	void stopTrackPlay();
	void startTrackPlay(const QStringList & trackInfo);

private:
	struct Private;
	Private * p;
	static Player * inst;
	Player();
};

#endif /* PLAYER_H_ */
