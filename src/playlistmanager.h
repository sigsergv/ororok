/*
 * playlistmanager.h
 *
 *  Created on: Nov 1, 2009
 *      Author: Sergei Stolyarov
 */

#ifndef PLAYLISTMANAGER_H_
#define PLAYLISTMANAGER_H_

#include <QObject>
#include "playlistwidget.h"

struct QString;
struct QChar;
struct QStringList;
struct QTabWidget;
struct QDateTime;

class PlaylistManager : public QObject
{
	Q_OBJECT
public:
	static PlaylistManager * instance();
	/**
	 * get PlaylistWindget object, fetch existing or create new one
	 *
	 * @param name name of the playlist
	 * @return
	 */
	//PlaylistWidget * playlist(const QString & name=QString(), const QString & title=QString());
	PlaylistWidget * createPlaylist(const QString & name=QString());
	PlaylistWidget * loadPlaylist(const QChar & plType, const QString & uid, const QString & name);
	QTabWidget * playlistsTabWidget();
	QStringList fetchNextTrack();
	QStringList fetchPrevTrack();

protected:
	PlaylistWidget * initPlaylistWidget(const QString & uid, const QChar & plType, const QString & name);

public slots:
	void requestTrackPause();
	void requestTrackResume();
	void requestTrackStop();
	void requestTrackPlay(const QStringList & trackInfo);
	void requestTrackPlay();
	void trackPlayingStarted(const QStringList & trackInfo);

	void tabCloseRequested(int index);

protected slots:
	void midTrackReached(const QStringList & trackInfo, const QDateTime & startTime);
	void playlistTypeChanged(const QString & uid, PlaylistWidget::PlaylistType newType);
	void playlistNameChanged(const QString & uid, const QString newName);

private:
	struct Private;
	Private * p;
	static PlaylistManager * inst;
	PlaylistManager();
};


#endif /* PLAYLISTMANAGER_H_ */
