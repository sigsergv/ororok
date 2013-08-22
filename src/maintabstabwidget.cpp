/*
 * maintabstabwidget.cpp
 *
 *  Created on: Jul 4, 2010
 *      Author: Sergey Stolyarov
 */

#include <QtGui>
#include <QtDebug>

#include "maintabstabwidget.h"
#include "maintabswidget.h"
#include "playlistwidget.h"
#include "playlistmodel.h"
#include "playlistmanager.h"
#include "settings.h"


MainTabsTabWidget::MainTabsTabWidget(QWidget * parent)
	: QTabWidget(parent)
{
	setTabsClosable(true);

	// load all playlists
	PlaylistManager * pm = PlaylistManager::instance();
	QList<Ororok::PlaylistInfo> items = pm->loadPlaylistItems();

	QSettings * settings = Ororok::settings();
	QStringList lastTrackInfo = settings->value("Playlists/lastTrack").toStringList();
	QString lastTrackPlaylistUid;
	int lastTrackNum = -1;
	if (lastTrackInfo.size() == 2) {
		lastTrackPlaylistUid = lastTrackInfo[0];
		lastTrackNum = lastTrackInfo[1].toUInt();
	}

	foreach (const Ororok::PlaylistInfo & pi, items) {
		PlaylistWidget * pw = pm->loadPlaylist(pi);
		pw->setParent(this);
		addTab(pw, pw->name());

		if (pi.uid == lastTrackPlaylistUid && lastTrackNum > -1) {
			pw->model()->selectActiveTrack(lastTrackNum);
			pw->model()->markActiveTrackPaused();
		}
	}

	if (items.length() == 0) {
		PlaylistWidget * pw = pm->createPlaylist();
		pw->setParent(this);
		addTab(pw, pw->name());
	}

	connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(tabCloseRequested(int)));
}

MainTabsTabWidget::~MainTabsTabWidget()
{

}

int MainTabsTabWidget::addTab(MainTabsWidget * page, const QString & label)
{
	connect(page, SIGNAL(pageTitleChanged(MainTabsWidget *, const QString &)),
			this, SLOT(tabRenamed(MainTabsWidget *, const QString &)));
	return QTabWidget::addTab(page, label);
}

void MainTabsTabWidget::tabCloseRequested(int index)
{
	MainTabsWidget * tabWidget = qobject_cast<MainTabsWidget*>(widget(index));
	//MainTabsWidget * tabWidget = dynamic_cast<MainTabsWidget*>(widget(index));
	// ask widget to close
	if (tabWidget->close()) {
		removeTab(index);
		delete tabWidget;
	}
}

void MainTabsTabWidget::tabRenamed(MainTabsWidget * page, const QString & name)
{
	int index = indexOf(page);
	if (-1 == index) {
		return;
	}
	setTabText(index, name);
}
