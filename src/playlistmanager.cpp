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
#include "mainwindow.h"
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
	p->lastfmScrobbler = new Ororok::lastfm::ScrobblerAdapter(0);

	Player * player = Player::instance();

	connect(player, SIGNAL(trackChanged(const QStringList &)),
			this, SLOT(trackPlayingStarted(const QStringList &)));
	connect(player, SIGNAL(midTrackReached(const QStringList &, const QDateTime &)),
			this, SLOT(midTrackReached(const QStringList &, const QDateTime &)));
}

PlaylistWidget * PlaylistManager::createPlaylist()
{
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

	PlaylistManager::PlaylistInfo pi;
	pi.uid = uid;
	pi.type = 't';

	return initPlaylistWidget(pi);
}

PlaylistWidget * PlaylistManager::loadPlaylist(const PlaylistManager::PlaylistInfo & pi)
{
	if (pi.type != 't' && pi.type != 'p') {
		// unknown playlist type: neither 't'emporary nor 'p'ermanent
		return 0;
	}

	QString storePath;
	if (pi.type == 't') {
		storePath = Ororok::tmpPlaylistsStorePath();
	} else {
		storePath = Ororok::playlistsStorePath();
	}
	QString plFilePath = storePath + "/" + pi.uid;

	return initPlaylistWidget(pi);
}

/*
 * create playlists tab widget and initialize it if required
 */
QTabWidget * PlaylistManager::playlistsTabWidget()
{
	if (p->playlistsTabWidget == 0) {
		p->playlistsTabWidget = new QTabWidget();
		p->playlistsTabWidget->setTabsClosable(true);
		connect(p->playlistsTabWidget, SIGNAL(tabCloseRequested(int)),
				this, SLOT(tabCloseRequested(int)));

		QList<PlaylistManager::PlaylistInfo> items = loadPlaylistItems();

		foreach (const PlaylistManager::PlaylistInfo & pi, items) {
			loadPlaylist(pi);
		}

		if (items.length() == 0) {
			createPlaylist();
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

PlaylistWidget * PlaylistManager::initPlaylistWidget(const PlaylistManager::PlaylistInfo & pi)
{
	PlaylistWidget * pw = new PlaylistWidget(pi.uid, PlaylistWidget::PlaylistTemporary);
	p->playlists[pi.uid] = pw;
	pw->setParent(p->playlistsTabWidget);
	p->playlistsTabWidget->addTab(pw, pw->name());

	QSettings * settings = Ororok::settings();
	QStringList settingsPlaylists = settings->value("Playlists/current", QStringList()).toStringList();
	settingsPlaylists << pi.type + QString(":") + pi.uid + QString(":") + pw->name();
	settings->setValue("Playlists/current", settingsPlaylists);

	connect(pw, SIGNAL(trackPlayRequsted(const QStringList &)), this,
			SLOT(requestTrackPlay(const QStringList &)));
	connect(pw, SIGNAL(playlistTypeChanged(const QString &, int)),
			this, SLOT(playlistTypeChanged(const QString &, int)));
	connect(pw, SIGNAL(playlistNameChanged(const QString &, const QString &)),
			this, SLOT(playlistNameChanged(const QString &, const QString &)));
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

	p->lastfmScrobbler->nowPlaying(trackInfo[Ororok::TrackFieldTitle], trackInfo[Ororok::TrackFieldArtist],
			trackInfo[Ororok::TrackFieldAlbum], trackInfo[Ororok::TrackFieldLength].toUInt());
}

void PlaylistManager::tabCloseRequested(int index)
{
	qDebug() << "close tab #" << index;
}

void PlaylistManager::lastfmLoveCurrentTrack()
{
	// find playlist that owns currently playing track and process it
	PlaylistWidget *  targetPlaylist = 0;

	Q_FOREACH (PlaylistWidget * pw, p->playlists) {
		if (PlaylistModel::TrackStatePlaying == pw->model()->activeTrackState()) {
			targetPlaylist = pw;
			break;
		}
	}

	if (targetPlaylist) {
		PlaylistModel * model = targetPlaylist->model();
		QStringList activeTrack = model->activeTrack();
		p->lastfmScrobbler->love(activeTrack[Ororok::TrackFieldTitle], activeTrack[Ororok::TrackFieldArtist],
				activeTrack[Ororok::TrackFieldAlbum]);
	}

}

void PlaylistManager::midTrackReached(const QStringList & trackInfo, const QDateTime & startTime)
{
	// submit track to lastfm
	// find currently playing track and submit it
	qDebug() << "submit track to lastfm:" << trackInfo[Ororok::TrackFieldTitle];
	p->lastfmScrobbler->submit(trackInfo[Ororok::TrackFieldTitle], trackInfo[Ororok::TrackFieldArtist],	trackInfo[Ororok::TrackFieldAlbum],
			trackInfo[Ororok::TrackFieldLength].toUInt(), trackInfo[Ororok::TrackFieldNo].toUInt(), startTime);
}

void PlaylistManager::playlistTypeChanged(const QString & uid, int newType)
{
	if (newType != PlaylistWidget::PlaylistPermanent && newType != PlaylistWidget::PlaylistTemporary) {
		return;
	}

	// load playlists, alter put back
	QList<PlaylistManager::PlaylistInfo> items = loadPlaylistItems();
	QMutableListIterator<PlaylistManager::PlaylistInfo> i(items);

	bool altered = false;
	while (i.hasNext()) {
		PlaylistManager::PlaylistInfo & pi = i.next();
		if (pi.uid == uid) {
			pi.type = newType;
			altered = true;
			break;
		}
	}
	if (altered) {
		// change playlist type
		savePlaylistItems(items);
	}
}

void PlaylistManager::playlistNameChanged(const QString & uid, const QString & newName)
{
	// load playlists, alter, put back
	QList<PlaylistManager::PlaylistInfo> items = loadPlaylistItems();
	QMutableListIterator<PlaylistManager::PlaylistInfo> i(items);

	bool altered = false;
	while (i.hasNext()) {
		PlaylistManager::PlaylistInfo & pi = i.next();
		if (pi.uid == uid) {
			altered = true;
			break;
		}
	}
	if (altered) {
		savePlaylistItems(items);
		// find tab and update tab title
		// TODO: change also tab icon
		int cnt = p->playlistsTabWidget->count();
		for (int i=0; i<cnt; i++) {
			PlaylistWidget * w = qobject_cast<PlaylistWidget*>(p->playlistsTabWidget->widget(i));
			if (w->uid() == uid) {
				p->playlistsTabWidget->setTabText(i, newName);
				break;
			}
		}
	}
}


PlaylistManager * PlaylistManager::instance()
{
	if (PlaylistManager::inst == 0) {
		PlaylistManager::inst = new PlaylistManager();
	}

	return PlaylistManager::inst;
}

QList<PlaylistManager::PlaylistInfo> PlaylistManager::loadPlaylistItems()
{
	QList<PlaylistManager::PlaylistInfo> items;

	QSettings * settings = Ororok::settings();
	QStringList playlistDefs = settings->value("Playlists/current").toStringList();
	settings->setValue("Playlists/current", QStringList());
	foreach (QString playlistDef, playlistDefs) {
		// split into the parts
		// detect "type"
		if (!playlistDef.startsWith("t:") && !playlistDef.startsWith("p:")) {
			continue;
		}
		PlaylistManager::PlaylistInfo pi;

		pi.type = playlistDef[0];

		playlistDef = playlistDef.mid(2);

		// find puid
		int pos = playlistDef.indexOf(QChar(':'));
		if (-1 == pos) {
			pi.uid = playlistDef;
		} else {
			pi.uid = playlistDef.left(pos);
		}
		items.append(pi);
	}

	return items;
}

void PlaylistManager::savePlaylistItems(const QList<PlaylistManager::PlaylistInfo> & items)
{
	QListIterator<PlaylistManager::PlaylistInfo> i(items);
	QSettings * settings = Ororok::settings();
	QStringList playlistDefs = settings->value("Playlists/current").toStringList();
	QStringList values;
	while(i.hasNext()) {
		const PlaylistManager::PlaylistInfo & pi = i.next();
		QString v;
		v = QString("%1:%2").arg(pi.type).arg(pi.uid);
		values << v;
	}
	settings->setValue("Playlists/current", values);
}
