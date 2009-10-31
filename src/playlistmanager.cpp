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
				SLOT(playlistRequestedTrackPlay(const QStringList &)));
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

void PlaylistManager::playlistRequestedTrackPlay(const QStringList & trackInfo)
{
	// in this action we must do the following:
	// 1. stop currently playing track by sending corresponding signals
	// 2. request playing of requested track

	qDebug() << "track play requested" << trackInfo;
}

PlaylistManager * PlaylistManager::instance()
{
	if (PlaylistManager::inst == 0) {
		PlaylistManager::inst = new PlaylistManager();
	}

	return PlaylistManager::inst;
}
