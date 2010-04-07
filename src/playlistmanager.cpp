/*
 * playlistmanager.cpp
 *
 *  Created on: Nov 1, 2009
 *      Author: Sergei Stolyarov
 */

#include <QtCore>
#include <QtGui>
#include <QtDebug>
#include <stdlib.h>

#include "playlistmanager.h"
#include "application.h"
#include "playlistwidget.h"
#include "playlistmodel.h"
#include "player.h"
#include "settings.h"
#include "mimetrackinfo.h"
#include "services/lastfm/scrobbleradapter.h"

PlaylistManager * PlaylistManager::inst = 0;

struct PlaylistManager::Private
{
	QHash<QString, PlaylistWidget*> playlists;
	QTabWidget * playlistsTabWidget;
	Ororok::lastfm::ScrobblerAdapter * lastfmScrobbler;
	int index;
};

PlaylistManager::PlaylistManager()
{
	p = new Private;
	p->playlistsTabWidget = 0;
	p->index = 0;
	p->lastfmScrobbler = new Ororok::lastfm::ScrobblerAdapter(this);

	Player * player = Player::instance();
	connect(player, SIGNAL(trackChanged(const QStringList &)),
			this, SLOT(trackPlayingStarted(const QStringList &)));
	connect(player, SIGNAL(midTrackReached(const QStringList &, const QDateTime &)),
			this, SLOT(midTrackReached(const QStringList &, const QDateTime &)));
}
/*
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
*/

PlaylistWidget * PlaylistManager::createPlaylist(const QString & name)
{
	QString playlistName(name);
	if (name.isEmpty()) {
		playlistName = tr("New Playlist");
	}

	QString storePath = Ororok::tmpPlaylistsStorePath();
	QString plFilePath;
	QString uid;

	// allocate UID
	while (true) {
		QStringList uidc;
		for (int i=0; i<4; i++) {
			uidc.append(QString("%1").arg(rand(), 0, 16));
		}
		uidc << QString("%1").arg(QDateTime::currentDateTime().toUTC().toTime_t(), 0, 16);
		uid = uidc.join("-");
		plFilePath = storePath + "/" + uid;
		QFileInfo fi(plFilePath);
		if (!fi.exists()) {
			break;
		}
	}

	return initPlaylistWidget(uid, 't', playlistName);
}

PlaylistWidget * PlaylistManager::loadPlaylist(const QChar & plType, const QString & uid, const QString & name)
{
	QChar t(plType);
	if (t != 't' && t != 'p') {
		// unknown playlist type: neither 't'emporary nor 'p'ermanent
		return 0;
	}

	QString storePath;
	if (t == 't') {
		storePath = Ororok::tmpPlaylistsStorePath();
	} else {
		storePath = Ororok::playlistsStorePath();
	}
	QString plFilePath = storePath + "/" + uid;

	return initPlaylistWidget(uid, t, name);
}

QTabWidget * PlaylistManager::playlistsTabWidget()
{
	if (p->playlistsTabWidget == 0) {
		p->playlistsTabWidget = new QTabWidget();
		p->playlistsTabWidget->setTabsClosable(true);
		connect(p->playlistsTabWidget, SIGNAL(tabCloseRequested(int)),
				this, SLOT(tabCloseRequested(int)));

		// load playlists definitions from the settings
		QSettings * settings = Ororok::settings();
		QStringList playlistDefs = settings->value("Playlists/current").toStringList();
		settings->setValue("Playlists/current", QStringList());
		foreach (QString playlistDef, playlistDefs) {
			// split into the parts
			// detect "type"
			if (!playlistDef.startsWith("t:") && !playlistDef.startsWith("p:")) {
				continue;
			}
			QChar plType(playlistDef[0]);

			playlistDef = playlistDef.mid(2);
			// find uid
			int pos = playlistDef.indexOf(QChar(':'));
			QString name;
			QString uid;
			if (-1 == pos) {
				name = tr("Playlist");
				uid = playlistDef;
			} else {
				uid = playlistDef.left(pos);
				name = playlistDef.mid(pos+1);
			}
			//qDebug() << plType << uid << name;
			loadPlaylist(plType, uid, name);
		}
		if (playlistDefs.length() == 0) {
			createPlaylist(tr("Default"));
		}
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

PlaylistWidget * PlaylistManager::initPlaylistWidget(const QString & uid, const QChar & plType, const QString & name)
{
	PlaylistWidget * pw = new PlaylistWidget(uid, PlaylistWidget::PlaylistTemporary);
	p->playlists[uid] = pw;
	pw->setParent(p->playlistsTabWidget);
	p->playlistsTabWidget->addTab(pw, name);

	QSettings * settings = Ororok::settings();
	QStringList settingsPlaylists = settings->value("Playlists/current", QStringList()).toStringList();
	settingsPlaylists << plType + QString(":") + uid + QString(":") + name;
	settings->setValue("Playlists/current", settingsPlaylists);

	connect(pw, SIGNAL(trackPlayRequsted(const QStringList &)), this,
			SLOT(requestTrackPlay(const QStringList &)));

	return pw;
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

	// send lastfm "nowplaying" notification
	qDebug() << "send \"now playing\" notification to Last.fm:" << trackInfo[Ororok::TrackFieldTitle];
	p->lastfmScrobbler->nowPlaying(trackInfo[Ororok::TrackFieldTitle], trackInfo[Ororok::TrackFieldArtist],
			trackInfo[Ororok::TrackFieldAlbum], trackInfo[Ororok::TrackFieldLength].toUInt());
}

void PlaylistManager::tabCloseRequested(int index)
{
	qDebug() << "close tab #" << index;
}

void PlaylistManager::midTrackReached(const QStringList & trackInfo, const QDateTime & startTime)
{
	// submit track to lastfm
	// find currently playing track and submit it
	qDebug() << "submit track to lastfm:" << trackInfo[Ororok::TrackFieldTitle];
	p->lastfmScrobbler->submit(trackInfo[Ororok::TrackFieldTitle], trackInfo[Ororok::TrackFieldArtist],	trackInfo[Ororok::TrackFieldAlbum],
			trackInfo[Ororok::TrackFieldLength].toUInt(), trackInfo[Ororok::TrackFieldNo].toUInt(), startTime);
}

PlaylistManager * PlaylistManager::instance()
{
	if (PlaylistManager::inst == 0) {
		PlaylistManager::inst = new PlaylistManager();
	}

	return PlaylistManager::inst;
}
