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
#include "playlistmodel.h"
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

PlaylistWidget * PlaylistManager::playlist(const QString & name, const QString & title)
{
	if (p->playlistsTabWidget == 0) {
		return 0;
	}

	PlaylistWidget * pw = p->playlists.value(name, 0);
	if (0 == pw) {
		// create playlist widget, load tracks if required and
		// add to the hash
		qDebug() << "create playlist " << name;
		QString playlistTitle(title);
		if (playlistTitle.isEmpty()) {
			playlistTitle = name;
		}

		pw = new PlaylistWidget;
		p->playlists[name] = pw;
		pw->setParent(p->playlistsTabWidget);
		p->playlistsTabWidget->addTab(pw, playlistTitle);

		connect(pw, SIGNAL(trackPlayRequsted(const QStringList &)), this,
				SLOT(requestTrackPlay(const QStringList &)));
	}

	return p->playlists[name];
}

QTabWidget * PlaylistManager::playlistsTabWidget()
{
	if (p->playlistsTabWidget == 0) {
		p->playlistsTabWidget = new QTabWidget();
		playlist("default", tr("Default"));
	}

	return p->playlistsTabWidget;
}

void PlaylistManager::requestTrackPause()
{
	Player * player = Player::instance();

	// find playlist that owns currently playing track and pause it
	PlaylistWidget *  targetPlaylist = 0;

	Q_FOREACH (PlaylistWidget * pw, p->playlists) {
		if (PlaylistModel::TrackStatePlaying == pw->model()->activeTrackState()) {
			targetPlaylist = pw;
			break;
		}
	}

	player->pause();

	if (targetPlaylist) {
		targetPlaylist->model()->markActiveTrackPaused();
	}
}

void PlaylistManager::requestTrackResume()
{
	Player * player = Player::instance();

	// find playlist that owns currently playing track and pause it
	PlaylistWidget *  targetPlaylist = 0;

	Q_FOREACH (PlaylistWidget * pw, p->playlists) {
		if (PlaylistModel::TrackStatePaused == pw->model()->activeTrackState()) {
			targetPlaylist = pw;
			break;
		}
	}

	player->play();

	if (targetPlaylist) {
		targetPlaylist->model()->markActiveTrackPlaying();
	}
}
/**
 * track play action requested
 *
 * @param trackInfo contains track specs, that track is owned by one of playlists
 */
void PlaylistManager::requestTrackPlay(const QStringList & trackInfo)
{
	Player * player = Player::instance();

	player->stop();

	// notify all PlaylistWidget instances that track is stopped
	// also tell that track is started, playlist should
	// recognize track and mark it properly if required
	PlaylistWidget *  targetPlaylist = 0;

	Q_FOREACH (PlaylistWidget * pw, p->playlists) {
		pw->model()->markActiveTrackStopped();
		if (pw->model()->selectActiveTrack(trackInfo)) {
			targetPlaylist = pw;
		}
	}

	// stop currently playing track and start new
	player->start(trackInfo);

	if (targetPlaylist) {
		// TODO: check is track really started
		targetPlaylist->model()->markActiveTrackStarted();
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
