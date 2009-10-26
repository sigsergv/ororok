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
	//layout->addWidget(filter);

	//widget->show();
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(p->filter);
	layout->addWidget(p->collectionTreeView);
	this->setLayout(layout);


	p->model = new CollectionItemModel(this);
	p->proxy = new CollectionTreeFilter(this);
	p->proxy->setFilterKeyColumn(0);
	//p->proxy->setDynamicSortFilter(true);
	p->proxy->setSourceModel(p->model);

	//CollectionTreeFilter *proxyModel = new CollectionTreeFilter(this);
	//proxyModel->setSourceModel(p->model);

	AlbumItemDelegate * delegate = new AlbumItemDelegate(this);
	p->collectionTreeView->setItemDelegate(delegate);

	p->collectionTreeView->setModel(p->proxy);
	//p->collectionTreeView->setModel(p->proxy);
	//p->collectionTreeView->setModel(p->model);

	connect(p->filter, SIGNAL(returnPressed()), this, SLOT(filterActivated()));

}

bool CollectionTreeWidget::reloadTree()
{
	/*
	// unset model
	p->collectionTreeView->setModel(0);
	// reload model
	p->model->reloadData();
	p->collectionTreeView->setModel(p->proxy);
	*/
	return true;
}

void CollectionTreeWidget::filterActivated()
{
	QString filterText = p->filter->text();
	qDebug() << "filter activated with text: " << filterText;
	//p->proxy->invalidate();
	p->proxy->resetSavedValuesCache();
	p->proxy->setFilterFixedString(filterText);
}
