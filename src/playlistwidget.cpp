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

struct PlaylistWidget::Private
{
	QLineEdit * filter;
	QTreeView * tracksList;
	PlaylistModel * model;
	QSortFilterProxyModel * proxy;
	bool initialized;
};

PlaylistWidget::PlaylistWidget(QWidget * parent)
	: QWidget(parent)
{
	p = new Private;

	p->initialized = false;
	QVBoxLayout * layout = new QVBoxLayout(this);
	p->filter = new QLineEdit(this);
	layout->addWidget(p->filter);
	p->tracksList = new EditTreeView(this);
	p->tracksList->setAlternatingRowColors(true);
	p->tracksList->setRootIsDecorated(false);
	p->tracksList->setDragDropMode(QAbstractItemView::DropOnly);
	p->tracksList->setSelectionBehavior(QAbstractItemView::SelectRows);
	p->tracksList->setSelectionMode(QAbstractItemView::ExtendedSelection);
	p->tracksList->setAcceptDrops(true);
	layout->addWidget(p->tracksList);

	p->model = new PlaylistModel(this);

	p->proxy = new QSortFilterProxyModel(this);
	p->proxy->setSourceModel(p->model);
	p->tracksList->setModel(p->proxy);

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
	Q_FOREACH (const QModelIndex & index, p->tracksList->selectionModel()->selectedIndexes()) {
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
	qDebug() << "delete selected tracks";
}
