/*
 * playlistwidget.cpp
 *
 *  Created on: Oct 28, 2009
 *      Author: Sergei Stolyarov
 */

#include <QtGui>

#include "playlistwidget.h"
#include "playlistmodel.h"

struct PlaylistWidget::Private
{
	QLineEdit * filter;
	QTreeView * tracksList;
	PlaylistModel * model;
	QSortFilterProxyModel * proxy;
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

	p->proxy = new QSortFilterProxyModel(this);
	p->proxy->setSourceModel(p->model);
	p->tracksList->setModel(p->proxy);

	//p->tracksList->show();

	setLayout(layout);
}


PlaylistWidget::~PlaylistWidget()
{
	delete p;
}
