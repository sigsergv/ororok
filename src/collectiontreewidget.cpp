/*
 * collectiontreewidget.cpp
 *
 *  Created on: 22.10.2009
 *      Author: Sergei Stolyarov
 */
#include <QtGui>

#include "collectiontreewidget.h"
#include "collectionitemmodel.h"
#include "albumitemdelegate.h"

struct CollectionTreeWidget::Private
{
	QLineEdit * filter;
	QTreeView * collectionTreeView;
	CollectionItemModel * model;
};

CollectionTreeWidget::CollectionTreeWidget(QWidget * parent)
		: QWidget(parent)
{
	p = new Private;

	p->filter = new QLineEdit(this);
	p->collectionTreeView = new QTreeView(this);
	p->collectionTreeView->setHeaderHidden(true);
	//layout->addWidget(filter);

	//widget->show();
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(p->filter);
	layout->addWidget(p->collectionTreeView);
	this->setLayout(layout);

	p->model = new CollectionItemModel(this);
	p->collectionTreeView->setModel(p->model);
	//model->setArtistsAlbumsMode("");

	AlbumItemDelegate * delegate = new AlbumItemDelegate(this);
	p->collectionTreeView->setItemDelegate(delegate);
}

bool CollectionTreeWidget::reloadTree()
{
	// unset model
	p->collectionTreeView->setModel(0);
	// reload model
	p->model->reloadData();
	p->collectionTreeView->setModel(p->model);
	return true;
}
