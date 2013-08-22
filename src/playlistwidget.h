/*
 * playlistwidget.h
 *
 *  Created on: Oct 28, 2009
 *      Author: Sergey Stolyarov
 */

#ifndef PLAYLISTWIDGET_H_
#define PLAYLISTWIDGET_H_

#include "maintabswidget.h"
#include "ororok.h"

struct QModelIndex;
struct PlaylistModel;

class PlaylistWidget : public MainTabsWidget
{
	Q_OBJECT

public:
	//PlaylistWidget(QWidget * parent = 0);
	PlaylistWidget(QString uid, Ororok::PlaylistType t, QWidget * parent = 0);
	~PlaylistWidget();
	QString uid();
	QString name();

	/**
	 * fetch track that is playing now or selected for playing. If there
	 * is no such track return first track of playlist. If there are no tracks
	 * in the playlist return empty QStringList object.
	 * @return
	 */
	QStringList activeTrackInfo();
	PlaylistModel * model();
	virtual bool close();

signals:
	void trackPlayRequsted(const QStringList & trackInfo);
	void playlistTypeChanged(const QString & uid, Ororok::PlaylistType newType);
	void playlistNameChanged(const QString & uid, const QString & newName);
	void deletePlaylist(const QString & uid, bool removeFile);

protected slots:
	void playlistDoubleClicked(const QModelIndex & index);
	void resizeEvent(QResizeEvent * event);
	void deleteSelectedTracks();
	void clearPlaylist();
	void shufflePlaylist();
	void renamePlaylist();
	void tracksContextMenu(const QPoint & pos);

private:
	struct Private;
	Private * p;
};

#endif /* PLAYLISTWIDGET_H_ */
