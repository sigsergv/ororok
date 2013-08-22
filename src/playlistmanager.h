/*
 * playlistmanager.h
 *
 *  Created on: Jul 4, 2010
 *      Author: Sergey Stolyarov
 */

#ifndef PLAYLISTMANAGER_H_
#define PLAYLISTMANAGER_H_

#include <QObject>
#include "ororok.h"

struct PlaylistWidget;
struct QDateTime;

class PlaylistManager : public QObject
{
	Q_OBJECT

public:
	static PlaylistManager * instance();
	~PlaylistManager();
	PlaylistWidget * createPlaylist();
	QStringList fetchNextTrack();
	QStringList fetchPrevTrack();
	QList<Ororok::PlaylistInfo> loadPlaylistItems();
	PlaylistWidget * loadPlaylist(const Ororok::PlaylistInfo & pi);
	PlaylistWidget * activePlaylist();

public slots:
	void requestTrackPause();
	void requestTrackResume();
	void requestTrackStop();
	void requestTrackPlay(const QStringList & trackInfo);
	void requestTrackPlay();
	void trackPlayingStarted(const QStringList & trackInfo);
	void lastfmLoveCurrentTrack();

protected slots:
	void midTrackReached(const QStringList & trackInfo, const QDateTime & startTime);
	void playlistTypeChanged(const QString & uid, Ororok::PlaylistType newType);
	void playlistNameChanged(const QString & uid, const QString & newName);
	void deletePlaylist(const QString & uid, bool removeFile);

private:
	struct Private;
	Private * p;
	static PlaylistManager * inst;
	PlaylistManager();
	void savePlaylistItems(const QList<Ororok::PlaylistInfo> & items);
	PlaylistWidget * initPlaylistWidget(const Ororok::PlaylistInfo & pi);
};
#endif /* PLAYLISTMANAGER_H_ */
