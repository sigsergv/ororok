/*
 * maintabstabwidget.cpp
 *
 *  Created on: Jul 4, 2010
 *      Author: Sergei Stolyarov
 */

#include <QtGui>
#include <QtDebug>

#include "maintabstabwidget.h"
#include "maintabswidget.h"
#include "playlistwidget.h"
#include "playlistmanager.h"

MainTabsTabWidget::MainTabsTabWidget(QWidget * parent)
	: QTabWidget(parent)
{
	setTabsClosable(true);

	// load all playlists
	PlaylistManager * pm = PlaylistManager::instance();
	QList<Ororok::PlaylistInfo> items = pm->loadPlaylistItems();

	foreach (const Ororok::PlaylistInfo & pi, items) {
		PlaylistWidget * pw = pm->loadPlaylist(pi);
		pw->setParent(this);
		addTab(pw, pw->name());
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
