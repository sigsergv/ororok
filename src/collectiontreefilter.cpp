/*
 * collectiontreefilter.cpp
 *
 *  Created on: Oct 26, 2009
 *      Author: Sergei Stolyarov
 */

#include "collectiontreefilter.h"

CollectionTreeFilter::CollectionTreeFilter(QObject * parent)
	: QAbstractProxyModel(parent)
{

}

QModelIndex CollectionTreeFilter::mapFromSource(const QModelIndex & sourceIndex) const
{
	Q_ASSERT(false);
	// disabled for now
	return sourceIndex;
}

QModelIndex CollectionTreeFilter::mapToSource(const QModelIndex & proxyIndex) const
{
	return sourceModel()->index(proxyIndex.row(), proxyIndex.column(), parent(proxyIndex));
}

int CollectionTreeFilter::columnCount(const QModelIndex & parent) const
{
	return sourceModel()->columnCount(parent);
}

QVariant CollectionTreeFilter::data(const QModelIndex & index, int role) const
{
	return sourceModel()->data(index, role);
}

QModelIndex CollectionTreeFilter::index(int row, int column, const QModelIndex & parent) const
{
	return sourceModel()->index(row, column, parent);
}

QModelIndex CollectionTreeFilter::parent(const QModelIndex & index) const
{
	return sourceModel()->parent(index);
}

int CollectionTreeFilter::rowCount(const QModelIndex & parent) const
{
	return sourceModel()->rowCount(parent);
}



