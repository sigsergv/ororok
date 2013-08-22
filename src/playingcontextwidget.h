/*
 * playingcontextwidget.h
 *
 *  Created on: Apr 5, 2010
 *      Author: Sergey Stolyarov
 */

#ifndef PLAYINGCONTEXTWIDGET_H_
#define PLAYINGCONTEXTWIDGET_H_

#include <QWidget>

struct QStringList;

class PlayingContextWidget : public QWidget
{
	Q_OBJECT
public:
	PlayingContextWidget(QWidget * parent = 0, Qt::WindowFlags f = 0);
	~PlayingContextWidget();

public slots:
	void playerTrackStarted(const QStringList & trackInfo);

private:
	struct Private;
	Private * p;
};

#endif /* PLAYINGCONTEXTWIDGET_H_ */
