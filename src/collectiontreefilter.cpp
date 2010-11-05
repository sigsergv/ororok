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

CollectionTreeFilter::CollectionTreeFilter(int role, QObject * parent) :
	QSortFilterProxyModel(parent), matchRole(role) {
}

bool CollectionTreeFilter::filterAcceptsRow(int source_row,
		const QModelIndex & source_parent) const {
	QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
	bool res = idx.data(matchRole).toBool();
	if (matchRole == CollectionItemModel::ItemDatePeriodMatchedRole) {
		//qDebug() << res;
	}
	return res;
}
