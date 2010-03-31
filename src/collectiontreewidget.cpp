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
#include "collectiontreefilter.h"

struct CollectionTreeWidget::Private
{
	QLineEdit * filter;
	QTreeView * collectionTreeView;
	CollectionItemModel * model;
	CollectionTreeFilter * proxy;
};

CollectionTreeWidget::CollectionTreeWidget(QWidget * parent)
		: QWidget(parent)
{
	p = new Private;

	p->filter = new QLineEdit(this);
	p->collectionTreeView = new QTreeView(this);
	p->collectionTreeView->setHeaderHidden(true);
	p->collectionTreeView->setDragEnabled(true);
	p->collectionTreeView->setAcceptDrops(false);
	p->collectionTreeView->setDropIndicatorShown(true);
	p->collectionTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	p->collectionTreeView->setDragDropMode(QAbstractItemView::DragOnly);
	p->collectionTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);

	p->proxy = 0;
	p->model = 0;
	//layout->addWidget(filter);

	//widget->show();
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(p->filter);
	layout->addWidget(p->collectionTreeView);
	this->setLayout(layout);

	createModel();

	CollectionItemDelegate * delegate = new CollectionItemDelegate(this);
	p->collectionTreeView->setItemDelegate(delegate);

	//p->collectionTreeView->setModel(p->proxy);
	//p->collectionTreeView->setModel(p->model);

	connect(p->filter, SIGNAL(returnPressed()), this, SLOT(filterActivated()));

}

bool CollectionTreeWidget::reloadTree()
{
	createModel();
	return true;
}

void CollectionTreeWidget::filterActivated()
{
	QString filterText = p->filter->text();
	qDebug() << "filter activated with text: " << filterText;
	//p->proxy->invalidate();
	p->model->markItemsMatchString(filterText);
	p->proxy->setFilterFixedString(filterText);
}

void CollectionTreeWidget::createModel()
{
	CollectionItemModel * newModel = new CollectionItemModel(this);
	newModel->setSupportedDragActions(Qt::CopyAction | Qt::MoveAction);
	CollectionTreeFilter * newProxy = new CollectionTreeFilter(this);
	newProxy->setFilterKeyColumn(0);
	//newProxy->setSupportedDragActions(Qt::CopyAction);
	//p->proxy->setDynamicSortFilter(true);
	newProxy->setSourceModel(newModel);

	p->collectionTreeView->setModel(newProxy);
	delete p->proxy;
	delete p->model;
	p->proxy = newProxy;
	p->model = newModel;

}
