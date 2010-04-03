/*
 * playlistmanager.h
 *
 *  Created on: Nov 1, 2009
 *      Author: Sergei Stolyarov
 */

#ifndef PLAYLISTMANAGER_H_
#define PLAYLISTMANAGER_H_

#include <QObject>

struct PlaylistWidget;
struct QString;
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
	PlaylistWidget * playlist(const QString & name=QString(), const QString & title=QString());
	QTabWidget * playlistsTabWidget();
	QStringList fetchNextTrack();
	QStringList fetchPrevTrack();

public slots:
	void requestTrackPause();
	void requestTrackResume();
	void requestTrackStop();
	void requestTrackPlay(const QStringList & trackInfo);
	void requestTrackPlay();
	void trackPlayingStarted(const QStringList & trackInfo);

protected slots:
	void midTrackReached(const QStringList & trackInfo, const QDateTime & startTime);

private:
	struct Private;
	Private * p;
	static PlaylistManager * inst;
	PlaylistManager();
};


#endif /* PLAYLISTMANAGER_H_ */
