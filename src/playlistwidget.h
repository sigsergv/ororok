/*
 * playlistwidget.h
 *
 *  Created on: Oct 28, 2009
 *      Author: Sergei Stolyarov
 */

#ifndef PLAYLISTWIDGET_H_
#define PLAYLISTWIDGET_H_

#include <QWidget>

class PlaylistWidget : public QWidget
{
	Q_OBJECT
public:
	PlaylistWidget(QWidget * parent = 0);
	~PlaylistWidget();

private:
	struct Private;
	Private * p;
};

#endif /* PLAYLISTWIDGET_H_ */
