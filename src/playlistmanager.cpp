/*
 * playlistmanager.cpp
 *
 *  Created on: Nov 1, 2009
 *      Author: Sergei Stolyarov
 */

#include <QtCore>
#include <QtGui>
#include <QtDebug>

#include "playlistmanager.h"
#include "playlistwidget.h"
#include "player.h"

PlaylistManager * PlaylistManager::inst = 0;

struct PlaylistManager::Private
{
	QHash<QString, PlaylistWidget*> playlists;
	QTabWidget * playlistsTabWidget;
};

PlaylistManager::PlaylistManager()
{
	p = new Private;
	p->playlistsTabWidget = 0;
}

PlaylistWidget * PlaylistManager::playlist(const QString & name)
{
	PlaylistWidget * pw = p->playlists.value(name, 0);
	if (0 == pw) {
		// create playlist widget, load tracks if required and
		// add to the hash
		pw = new PlaylistWidget;
		p->playlists[name] = pw;
		connect(pw, SIGNAL(trackPlayRequsted(const QStringList &)), this,
				SLOT(requestTrackPlay(const QStringList &)));
	}

	return p->playlists[name];
}

QTabWidget * PlaylistManager::playlistsTabWidget()
{
	if (p->playlistsTabWidget == 0) {
		QTabWidget * tw = new QTabWidget();

		PlaylistWidget * defaultPlaylist = playlist("default");
		defaultPlaylist->setParent(tw);
		tw->addTab(defaultPlaylist, tr("Default"));
		p->playlistsTabWidget = tw;
	}

	return p->playlistsTabWidget;
}

void PlaylistManager::requestTrackPlay(const QStringList & trackInfo)
{
	Player * player = Player::instance();

	// stop currently playing track
	player->stopTrackPlay();

	// notify all PlaylistWidget instances that track is stopped
	// also tell that track is started, playlist should
	// recognize track and mark it properly if required
	Q_FOREACH (PlaylistWidget* pw, p->playlists) {
		pw->stopActiveTrack();
		player->startTrackPlay(trackInfo);
		pw->startActiveTrack(trackInfo);
	}

	//qDebug() << "track play requested" << trackInfo;
}

PlaylistManager * PlaylistManager::instance()
{
	if (PlaylistManager::inst == 0) {
		PlaylistManager::inst = new PlaylistManager();
	}

	return PlaylistManager::inst;
}
