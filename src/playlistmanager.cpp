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
#include "application.h"
#include "playlistwidget.h"
#include "playlistmodel.h"
#include "player.h"

PlaylistManager * PlaylistManager::inst = 0;

struct PlaylistManager::Private
{
	QHash<QString, PlaylistWidget*> playlists;
	QTabWidget * playlistsTabWidget;
	int index;
};

PlaylistManager::PlaylistManager()
{
	p = new Private;
	p->playlistsTabWidget = 0;
	p->index = 0;

	Player * player = Player::instance();
	connect(player, SIGNAL(trackChanged(const QStringList &)),
			this, SLOT(trackPlayingStarted(const QStringList &)));
}

PlaylistWidget * PlaylistManager::playlist(const QString & name, const QString & title)
{
	QString playlistName(name);

	if (name == "") {
		p->index++;
		playlistName = QString("playlist-%1").arg(p->index);
	}

	if (p->playlistsTabWidget == 0) {
		return 0;
	}

	PlaylistWidget * pw = p->playlists.value(playlistName, 0);
	if (0 == pw) {
		// create playlist widget, load tracks if required and
		// add to the hash
		qDebug() << "create playlist " << playlistName;
		QString playlistTitle(title);
		if (playlistTitle.isEmpty()) {
			playlistTitle = playlistName;
		}

		pw = new PlaylistWidget;
		p->playlists[playlistName] = pw;
		pw->setParent(p->playlistsTabWidget);
		p->playlistsTabWidget->addTab(pw, playlistTitle);

		connect(pw, SIGNAL(trackPlayRequsted(const QStringList &)), this,
				SLOT(requestTrackPlay(const QStringList &)));
	}

	return p->playlists[playlistName];
}

QTabWidget * PlaylistManager::playlistsTabWidget()
{
	if (p->playlistsTabWidget == 0) {
		p->playlistsTabWidget = new QTabWidget();
		playlist("default", tr("Default"));
	}

	return p->playlistsTabWidget;
}

/**
 * fetch next track from the current playlist
 *
 * @return next track spec or empty list if there is no one
 */
QStringList PlaylistManager::fetchNextTrack()
{
	QStringList trackInfo;

	// find playlist that owns currently playing track
	PlaylistModel * model = 0;

	Q_FOREACH (PlaylistWidget * pw, p->playlists) {
		if (PlaylistModel::TrackStatePlaying == pw->model()->activeTrackState()) {
			model = pw->model();
			break;
		}
	}

	if (model) {
		trackInfo = model->trackAfterActive();
	}

	return trackInfo;
}

QStringList PlaylistManager::fetchPrevTrack()
{
	QStringList trackInfo;

	// find playlist that owns currently playing track
	PlaylistModel * model = 0;

	Q_FOREACH (PlaylistWidget * pw, p->playlists) {
		if (PlaylistModel::TrackStatePlaying == pw->model()->activeTrackState()) {
			model = pw->model();
			break;
		}
	}

	if (model) {
		trackInfo = model->trackBeforeActive();
	}

	return trackInfo;
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
 *
 */
void PlaylistManager::requestTrackStop()
{
	Player * player = Player::instance();

	player->stop();

	Q_FOREACH (PlaylistWidget * pw, p->playlists) {
		pw->model()->markActiveTrackStopped();
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

/**
 * play current playlist
 */
void PlaylistManager::requestTrackPlay()
{
	// first stop all
	Player * player = Player::instance();
	player->stop();

	// take currently opened playlist
	PlaylistWidget * targetPlaylist =
			qobject_cast<PlaylistWidget*>(p->playlistsTabWidget->currentWidget());
	PlaylistModel * model = targetPlaylist->model();

	if (model->rowCount() == 0) {
		Ororok::warning(tr("Error"), tr("No tracks found in the current playlist"));
		return;
	}

	QStringList activeTrack = model->activeTrack();
	if (activeTrack.isEmpty()) {
		Ororok::warning(tr("Error"), tr("Playlist returned invalid track"));
		return;
	}

	model->selectActiveTrack(activeTrack);
	player->start(activeTrack);
	model->markActiveTrackPlaying();

}

/**
 * player just started new track and reported about that
 *
 * @param trackInfo
 */
void PlaylistManager::trackPlayingStarted(const QStringList & trackInfo)
{
	PlaylistModel * model = 0;

	Q_FOREACH (PlaylistWidget * pw, p->playlists) {
		//pw->model()->markActiveTrackStopped();
		if (pw->model()->selectActiveTrack(trackInfo)) {
			model = pw->model();
		}
	}

	if (model) {
		model->markActiveTrackPlaying();
	}
}

PlaylistManager * PlaylistManager::instance()
{
	if (PlaylistManager::inst == 0) {
		PlaylistManager::inst = new PlaylistManager();
	}

	return PlaylistManager::inst;
}
