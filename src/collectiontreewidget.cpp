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

CollectionTreeWidget::CollectionTreeWidget(QWidget * parent)
		: QWidget(parent)
{
	filter = new QLineEdit(this);
	collectionTreeView = new QTreeView(this);
	collectionTreeView->setHeaderHidden(true);
	//layout->addWidget(filter);

	//widget->show();
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(filter);
	layout->addWidget(collectionTreeView);
	this->setLayout(layout);

	CollectionItemModel * model = new CollectionItemModel(this);
	collectionTreeView->setModel(model);
	//model->setArtistsAlbumsMode("");

	AlbumItemDelegate * delegate = new AlbumItemDelegate(this);
	collectionTreeView->setItemDelegate(delegate);
}
