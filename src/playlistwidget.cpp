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

struct PlaylistWidget::Private
{
	QLineEdit * filter;
	QTreeView * tracksList;
	PlaylistModel * model;
	QSortFilterProxyModel * proxy;
	int activeTrack;
};

PlaylistWidget::PlaylistWidget(QWidget * parent)
	: QWidget(parent)
{
	p = new Private;

	QVBoxLayout * layout = new QVBoxLayout(this);
	p->filter = new QLineEdit(this);
	layout->addWidget(p->filter);
	p->tracksList = new QTreeView(this);
	p->tracksList->setAlternatingRowColors(true);
	p->tracksList->setRootIsDecorated(false);
	p->tracksList->setDragDropMode(QAbstractItemView::DropOnly);
	p->tracksList->setSelectionBehavior(QAbstractItemView::SelectRows);
	p->tracksList->setAcceptDrops(true);
	layout->addWidget(p->tracksList);

	p->model = new PlaylistModel(this);
	p->activeTrack = 0;

	p->proxy = new QSortFilterProxyModel(this);
	p->proxy->setSourceModel(p->model);
	p->tracksList->setModel(p->proxy);

	//p->tracksList->show();

	setLayout(layout);

	PlaylistItemDelegate * delegate = new PlaylistItemDelegate(this);
	p->tracksList->setItemDelegate(delegate);

	connect(p->tracksList, SIGNAL(doubleClicked(const QModelIndex &)),
			this, SLOT(playlistDoubleClicked(const QModelIndex &)));
}


PlaylistWidget::~PlaylistWidget()
{
	delete p;
}

QStringList PlaylistWidget::activeTrackInfo()
{
	// take selected track
	QStringList trackInfo;
	Q_FOREACH (const QModelIndex & index, p->tracksList->selectionModel()->selectedIndexes()) {
		trackInfo = index.data(PlaylistModel::ItemTrackInfoRole).toStringList();
		break;
	}

	//qDebug() << rows;
	return trackInfo;
}

void PlaylistWidget::playlistDoubleClicked(const QModelIndex & index)
{
	// doubleclicked track marked as “active track”
	QStringList trackInfo = index.data(PlaylistModel::ItemTrackInfoRole).toStringList();
	p->model->setActiveTrack(index.row());
	emit trackPlayRequsted(trackInfo);
}
