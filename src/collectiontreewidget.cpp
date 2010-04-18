/*
 * collectiontreewidget.cpp
 *
 *  Created on: 22.10.2009
 *      Author: Sergei Stolyarov
 */
#include <QtGui>

#include "filterlineedit.h"
#include "collectiontreewidget.h"
#include "collectionitemmodel.h"
#include "collectionitemdelegate.h"
#include "collectiontreefilter.h"

struct CollectionTreeWidget::Private
{
	QLineEdit * filter;
	QPushButton * filterResetButton;
	QTreeView * collectionTreeView;
	CollectionItemModel * model;
	CollectionTreeFilter * proxy;
	QTimer * filterTimer;
};

CollectionTreeWidget::CollectionTreeWidget(QWidget * parent)
		: QWidget(parent)
{
	p = new Private;

	QLayout * filterLayout = new QHBoxLayout();
	p->filterResetButton = new QPushButton(this);
	p->filterResetButton->setIcon(QIcon(":edit-clear-locationbar-rtl-16x16.png"));
	p->filterResetButton->setFlat(true);
	p->filterResetButton->setMaximumWidth(22);
	p->filterResetButton->setFocusPolicy(Qt::NoFocus);
	p->filterResetButton->hide();
	p->filterResetButton->setToolTip(tr("Click to reset collection filter"));

	p->filter = new FilterLineEdit(this);
	filterLayout->addWidget(p->filter);
	filterLayout->addWidget(p->filterResetButton);

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
	layout->addLayout(filterLayout);
	layout->addWidget(p->collectionTreeView);
	this->setLayout(layout);

	createModel();

	CollectionItemDelegate * delegate = new CollectionItemDelegate(this);
	p->collectionTreeView->setItemDelegate(delegate);

	//p->collectionTreeView->setModel(p->proxy);
	//p->collectionTreeView->setModel(p->model);

	p->filterTimer = new QTimer(this);

	connect(p->filter, SIGNAL(textChanged(const QString &)),
			this, SLOT(filterTextChanged(const QString &)));
	connect(p->filterTimer, SIGNAL(timeout()),
			this, SLOT(filterEditFinished()));
	connect(p->filterResetButton, SIGNAL(clicked()),
			this, SLOT(resetFilter()));
	connect(p->filter, SIGNAL(escapeKeyPressed()),
			this, SLOT(resetFilter()));
}

bool CollectionTreeWidget::reloadTree()
{
	createModel();
	return true;
}

void CollectionTreeWidget::applyFilter()
{
	QString filterText = p->filter->text();
	qDebug() << "filter activated with text: " << filterText;
	//p->proxy->invalidate();
	p->model->markItemsMatchString(filterText);
	p->proxy->setFilterFixedString(filterText);
}

void CollectionTreeWidget::resetFilter()
{
	p->filter->setText("");
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

void CollectionTreeWidget::filterTextChanged(const QString & text)
{
	bool filterEmpty = text.isEmpty();
	p->filterResetButton->setVisible(!filterEmpty);
	p->filterTimer->stop();
	p->filterTimer->start(500);
}

void CollectionTreeWidget::filterEditFinished()
{
	p->filterTimer->stop();
	applyFilter();
}
