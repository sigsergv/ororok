/*
 * collectiontreefilter.cpp
 *
 *  Created on: Oct 26, 2009
 *      Author: Sergei Stolyarov
 */

#include <QtCore>
#include <QtDebug>

#include "collectiontreefilter.h"
#include "collectionitemmodel.h"
#include "collectiontreeitem.h"

struct CollectionTreeFilter::Private
{
	QHash<qint64,int> checkedNodes;
	int counter;
};

CollectionTreeFilter::CollectionTreeFilter(QObject * parent)
	: QSortFilterProxyModel(parent)
{
	p = new Private;
	p->counter = 0;
}

bool CollectionTreeFilter::filterAcceptsRow(int source_row, const QModelIndex & source_parent) const
{
	//qDebug() << p->counter++;
	//return true;
	//return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);

	QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
	int savedValue = p->checkedNodes.value(idx.internalId(), -1);
	if (-1 != savedValue) {
		return savedValue == 1;
	}

	bool selfMatch = QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
	if (selfMatch) {
		p->checkedNodes[idx.internalId()] = 1;
		return true;
	} else {
		p->checkedNodes[idx.internalId()] = 0;
	}

	//qDebug() << idx.internalId();
	qDebug() << p->checkedNodes.count();

	int type = idx.data(CollectionItemModel::ItemTypeRole).toInt();
	if (CollectionTreeItem::Track == type) {
		return selfMatch;
	}
	// find for all children
	bool match = false;
	int cnt = sourceModel()->rowCount(idx);
	for (int i=0; i<cnt; i++) {
		match = match || filterAcceptsRow(i, idx);
		if (match) {
			break;
		}
	}
	return match;
	// accept row if any child matches the pattern
	//
	/*
	QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
	if (sourceModel()->hasChildren(idx)) {
		return true;
	}
	*/

	return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

void CollectionTreeFilter::resetSavedValuesCache()
{
	p->counter = 0;
	p->checkedNodes.clear();
}
