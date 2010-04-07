/*
 * playlistwidget.cpp
 *
 *  Created on: Oct 28, 2009
 *      Author: Sergei Stolyarov
 */

#include <QtGui>

#include "playlistwidget.h"
#include "playlistmodel.h"
#include "playlistitemdelegate.h"
#include "edittreeview.h"
#include "settings.h"

struct PlaylistWidget::Private
{
	QLineEdit * filter;
	QTreeView * tracksList;
	PlaylistModel * model;
	QSortFilterProxyModel * proxy;
	QMenu * tracksContextMenu;
	bool initialized;

	// actions
	QAction * deleteSelectedTracks;
};

PlaylistWidget::PlaylistWidget(QString uid, PlaylistWidget::PlaylistType t, QWidget * parent)
	: QWidget(parent)
{
	p = new Private;

	p->initialized = false;
	QVBoxLayout * layout = new QVBoxLayout(this);
	p->filter = new QLineEdit(this);
	layout->addWidget(p->filter);
	p->tracksList = new EditTreeView(this);
	p->tracksList->setContextMenuPolicy(Qt::CustomContextMenu);
	p->tracksList->setAlternatingRowColors(true);
	p->tracksList->setRootIsDecorated(false);
	//p->tracksList->setDragDropMode(QAbstractItemView::DropOnly);
	p->tracksList->setSelectionBehavior(QAbstractItemView::SelectRows);
	p->tracksList->setSelectionMode(QAbstractItemView::ExtendedSelection);
	p->tracksList->setDragEnabled(true);
	p->tracksList->setDragDropMode(QAbstractItemView::DragDrop);
	p->tracksList->setAcceptDrops(true);
	layout->addWidget(p->tracksList);

	// construct playlist name
	QString storePath;
	switch (t) {
	case PlaylistTemporary:
		storePath = Ororok::tmpPlaylistsStorePath();
		break;

	case PlaylistPermanent:
		storePath = Ororok::playlistsStorePath();
		break;
	}

	p->model = new PlaylistModel(storePath + "/" + uid, this);

	p->proxy = new QSortFilterProxyModel(this);
	p->proxy->setSourceModel(p->model);
	p->tracksList->setModel(p->proxy);

	// create menus and actions

	p->tracksContextMenu = new QMenu(this);
	p->deleteSelectedTracks = p->tracksContextMenu->addAction(tr("Delete selected tracks"));

	// resize columns
	// get total width
	/*
	for (int i=0; i<sections; i++) {
		QString section
	}
	*/

	//p->tracksList->setColumnWidth(VisColumnLength, 20);
	//qDebug() << p->tracksList->columnWidth(2);
	//p->tracksList->show();

	setLayout(layout);
	p->tracksList->header()->setStretchLastSection(false);
	p->tracksList->header()->resizeSection(0, 200); // set width of "Length" column
	p->tracksList->header()->resizeSection(1, 200);
	p->tracksList->header()->resizeSection(2, 200);

	PlaylistItemDelegate * delegate = new PlaylistItemDelegate(this);
	p->tracksList->setItemDelegate(delegate);

	connect(p->tracksList, SIGNAL(doubleClicked(const QModelIndex &)),
			this, SLOT(playlistDoubleClicked(const QModelIndex &)));
	connect(p->tracksList, SIGNAL(deleteKeyPressed()),
			this, SLOT(deleteSelectedTracks()));
	connect(p->deleteSelectedTracks, SIGNAL(triggered()),
			this, SLOT(deleteSelectedTracks()));
	connect(p->tracksList, SIGNAL(customContextMenuRequested(const QPoint &)),
			this, SLOT(tracksContextMenu(const QPoint &)));
}


PlaylistWidget::~PlaylistWidget()
{
	delete p;
}

PlaylistModel * PlaylistWidget::model()
{
	return p->model;
}

QStringList PlaylistWidget::activeTrackInfo()
{
	// take selected track
	QStringList trackInfo;
	Q_FOREACH (const QModelIndex & index, p->tracksList->selectionModel()->selectedRows()) {
		trackInfo = index.data(PlaylistModel::ItemTrackInfoRole).toStringList();
		break;
	}

	//qDebug() << rows;
	return trackInfo;
}

void PlaylistWidget::playlistDoubleClicked(const QModelIndex & index)
{
	QStringList trackInfo = index.data(PlaylistModel::ItemTrackInfoRole).toStringList();
	//markActiveTrackStarted(trackInfo);
	emit trackPlayRequsted(trackInfo);
}

void PlaylistWidget::resizeEvent(QResizeEvent * event)
{
	QWidget::resizeEvent(event);
	//qDebug() << p->tracksList->width();
	// TODO: resize columns to fit size
}

void PlaylistWidget::deleteSelectedTracks()
{
	// find selected tracks in the model and delete them!
	// take selected track
	//QStringList trackInfo;
	QList<int> rows;
	Q_FOREACH (const QModelIndex & index, p->tracksList->selectionModel()->selectedRows()) {
		rows << index.row();
	}
	// sort "rows" and delete items from bottom to top
	qSort(rows);
	QListIterator<int> i(rows);
	i.toBack();
	while (i.hasPrevious()) {
		p->model->removeRow(i.previous());
	}
}

void PlaylistWidget::tracksContextMenu(const QPoint & pos)
{
	// if there is no tracks then disable some actions
	bool tracksListEmpty = p->model->rowCount() == 0;

	p->deleteSelectedTracks->setEnabled(!tracksListEmpty);
	p->tracksContextMenu->popup(p->tracksList->viewport()->mapToGlobal(pos));
}
