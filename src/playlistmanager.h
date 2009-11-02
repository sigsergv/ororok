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
	PlaylistWidget * playlist(const QString & name);
	QTabWidget * playlistsTabWidget();

protected slots:
	void requestTrackPlay(const QStringList & trackInfo);

private:
	struct Private;
	Private * p;
	static PlaylistManager * inst;
	PlaylistManager();
};


#endif /* PLAYLISTMANAGER_H_ */
