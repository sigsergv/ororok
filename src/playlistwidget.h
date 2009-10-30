/*
 * playlistwidget.h
 *
 *  Created on: Oct 28, 2009
 *      Author: Sergei Stolyarov
 */

#ifndef PLAYLISTWIDGET_H_
#define PLAYLISTWIDGET_H_

#include <QWidget>

struct QModelIndex;

class PlaylistWidget : public QWidget
{
	Q_OBJECT

public:
	PlaylistWidget(QWidget * parent = 0);
	~PlaylistWidget();

	/**
	 * fetch track that is playing now or selected for playing. If there
	 * is no such track return first track of playlist. If there are no tracks
	 * in the playlist return empty QStringList object.
	 * @return
	 */
	QStringList activeTrackInfo();

signals:
	void trackPlayRequsted(const QStringList & trackInfo);

protected slots:
	void playlistDoubleClicked(const QModelIndex & index);

private:
	struct Private;
	Private * p;
};

#endif /* PLAYLISTWIDGET_H_ */