/*
 * collectiontreefilter.h
 *
 *  Created on: Oct 26, 2009
 *      Author: Sergei Stolyarov
 */

#ifndef COLLECTIONTREEFILTER_H_
#define COLLECTIONTREEFILTER_H_

#include <QSortFilterProxyModel>

class CollectionTreeFilter : public QSortFilterProxyModel
{
	Q_OBJECT
public:
	CollectionTreeFilter(QObject * parent = 0);
	void resetSavedValuesCache();
	//QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;

protected:
	bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;

private:
	struct Private;
	Private * p;
};

#endif /* COLLECTIONTREEFILTER_H_ */
