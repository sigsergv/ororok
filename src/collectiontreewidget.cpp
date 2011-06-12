/*
 * collectiontreewidget.cpp
 *
 *  Created on: 22.10.2009
 *      Author: Sergei Stolyarov
 */
#include <QtGui>

#include "filterlineedit.h"
#include "playlistmanager.h"
#include "playlistmodel.h"
#include "playlistwidget.h"
#include "collectiontreewidget.h"
#include "collectionitemmodel.h"
#include "collectionitemdelegate.h"
#include "collectiontreefilter.h"

struct CollectionTreeWidget::Private
{
	QLineEdit * quickSearchFilter;
	QComboBox * dateFilterCombo;
	QPushButton * filterResetButton;
	QTreeView * collectionTreeView;
	CollectionItemModel * model;
	CollectionTreeFilter * quickSearchProxy;
	CollectionTreeFilter * dateFilterProxy;
	QTimer * filterTimer;
	QMenu * itemContextMenu;
	QAction * appendToCurrentPlaylist;
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

	p->quickSearchFilter = new FilterLineEdit(this);
	filterLayout->addWidget(p->quickSearchFilter);
	filterLayout->addWidget(p->filterResetButton);

	p->collectionTreeView = new QTreeView(this);
	p->collectionTreeView->setHeaderHidden(true);
	p->collectionTreeView->setDragEnabled(true);
	p->collectionTreeView->setAcceptDrops(false);
	p->collectionTreeView->setDropIndicatorShown(true);
	p->collectionTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
	p->collectionTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	p->collectionTreeView->setDragDropMode(QAbstractItemView::DragOnly);
	p->collectionTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);

	p->dateFilterCombo = new QComboBox(this);
	p->dateFilterCombo->addItem(tr("Show all"), -1);
	p->dateFilterCombo->addItem(tr("Show added in the last week only"), 7);
	p->dateFilterCombo->addItem(tr("Show added in the last month only"), 30);
	p->dateFilterCombo->addItem(tr("Show added in the last 3 months only"), 90);

	p->quickSearchProxy = 0;
	p->dateFilterProxy = 0;

	p->model = 0;
	//layout->addWidget(quickSearchFilter);

	//widget->show();
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(p->dateFilterCombo);
	layout->addLayout(filterLayout);
	layout->addWidget(p->collectionTreeView);
	this->setLayout(layout);

	createModel();

	CollectionItemDelegate * delegate = new CollectionItemDelegate(this);
	p->collectionTreeView->setItemDelegate(delegate);

	//p->collectionTreeView->setModel(p->quickSearchProxy);
	//p->collectionTreeView->setModel(p->model);

	// init menus and actions
	p->itemContextMenu = new QMenu(this);
	p->appendToCurrentPlaylist = p->itemContextMenu->addAction(tr("Append to current playlist"));

	p->filterTimer = new QTimer(this);

	connect(p->quickSearchFilter, SIGNAL(textChanged(const QString &)),
			this, SLOT(filterTextChanged(const QString &)));
	connect(p->filterTimer, SIGNAL(timeout()),
			this, SLOT(filterEditFinished()));
	connect(p->filterResetButton, SIGNAL(clicked()),
			this, SLOT(resetFilter()));
	connect(p->quickSearchFilter, SIGNAL(escapeKeyPressed()),
			this, SLOT(resetFilter()));
	connect(p->dateFilterCombo, SIGNAL(currentIndexChanged(int)),
			this, SLOT(dateFilterChanged(int)));
	connect(p->collectionTreeView, SIGNAL(customContextMenuRequested(const QPoint &)),
			this, SLOT(itemContextMenu(const QPoint &)));
	connect(p->appendToCurrentPlaylist, SIGNAL(triggered()),
			this, SLOT(appendItemToCurrentPlaylist()));
}

bool CollectionTreeWidget::reloadTree()
{
	createModel();
	return true;
}

void CollectionTreeWidget::applyFilter()
{
	QString filterText = p->quickSearchFilter->text();
	qDebug() << "filter activated with text: " << filterText;
	//p->quickSearchProxy->invalidate();
	p->model->markItemsMatchQuickSearchString(filterText);
	p->quickSearchProxy->setFilterFixedString(filterText);
}

void CollectionTreeWidget::resetFilter()
{
	p->quickSearchFilter->setText("");
}

void CollectionTreeWidget::createModel()
{
	CollectionItemModel * newModel = new CollectionItemModel(this);
	newModel->setSupportedDragActions(Qt::CopyAction | Qt::MoveAction);
	CollectionTreeFilter * newQuickSearchProxy = new CollectionTreeFilter(CollectionItemModel::ItemQuickSearchMatchedRole, this);
	CollectionTreeFilter * newDateFilterProxy = new CollectionTreeFilter(CollectionItemModel::ItemDatePeriodMatchedRole, this);

	newQuickSearchProxy->setFilterKeyColumn(0);
	//newProxy->setSupportedDragActions(Qt::CopyAction);
	//p->quickSearchProxy->setDynamicSortFilter(true);
	newQuickSearchProxy->setSourceModel(newModel);
	newDateFilterProxy->setSourceModel(newQuickSearchProxy);

	//p->collectionTreeView->setModel(newQuickSearchProxy);
	p->collectionTreeView->setModel(newDateFilterProxy);

	delete p->quickSearchProxy;
	delete p->model;
	p->quickSearchProxy = newQuickSearchProxy;
	p->dateFilterProxy = newDateFilterProxy;
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

void CollectionTreeWidget::dateFilterChanged(int index)
{
    if (index == -1) {
		return;
	}

	QVariant data = p->dateFilterCombo->itemData(index, Qt::UserRole);
	bool bOk;
	int days = data.toInt(&bOk);
	if (!bOk || days < 0) {
		days = -1;
	}
	p->model->markItemsMatchDatePeriod(days);
	p->dateFilterProxy->setFilterFixedString(""); // value doesn't matter
	//p->quickSearchProxy->setFilterFixedString(""); // value doesn't matter

}

void CollectionTreeWidget::itemContextMenu(const QPoint & pos)
{
	p->itemContextMenu->popup(p->collectionTreeView->viewport()->mapToGlobal(pos));
}

void CollectionTreeWidget::appendItemToCurrentPlaylist()
{
	QItemSelectionModel * sm = p->collectionTreeView->selectionModel();
	// use CollectionItemModel::mimeData to fetch mime data for selected items
	// and the use PlaylistModel::dropMimeData to append to current playlist

	// we have to translate indexes!
	QModelIndexList translated;
	foreach (const QModelIndex & index, sm->selectedIndexes()) {
		QModelIndex newIndex = p->quickSearchProxy->mapToSource(p->dateFilterProxy->mapToSource(index));
		translated.append(newIndex);
	}

	QMimeData * md = p->model->mimeData(translated);
	PlaylistManager * pm = PlaylistManager::instance();
	PlaylistWidget * apw = pm->activePlaylist();
	if (!apw) {
		return;
	}
	apw->model()->dropMimeData(md, Qt::CopyAction, -1, 0, QModelIndex());
}
