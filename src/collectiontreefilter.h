/*
 * collectiontreefilter.h
 *
 *  Created on: Oct 26, 2009
 *      Author: Sergey Stolyarov
 */

#ifndef COLLECTIONTREEFILTER_H_
#define COLLECTIONTREEFILTER_H_

#include <QSortFilterProxyModel>
//#include <QModelIndex>

class CollectionTreeFilter : public QSortFilterProxyModel
{
	Q_OBJECT
public:
	CollectionTreeFilter(int role, QObject * parent = 0);
	QMimeData * mimeData(const QModelIndexList &indexes) const;

protected:
	int matchRole;
	bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;
	void findTracksInIndexesTree(const QModelIndex & index, QModelIndexList & target) const;

};

#endif /* COLLECTIONTREEFILTER_H_ */
