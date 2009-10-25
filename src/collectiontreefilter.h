/*
 * collectiontreefilter.h
 *
 *  Created on: Oct 26, 2009
 *      Author: Sergei Stolyarov
 */

#ifndef COLLECTIONTREEFILTER_H_
#define COLLECTIONTREEFILTER_H_

#include <QAbstractProxyModel>

class CollectionTreeFilter : public QAbstractProxyModel
{
	Q_OBJECT
public:
	CollectionTreeFilter(QObject * parent = 0);
	QModelIndex mapFromSource(const QModelIndex & sourceIndex) const;
	QModelIndex mapToSource(const QModelIndex & proxyIndex) const;
	int columnCount(const QModelIndex & parent = QModelIndex()) const;
	QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex & index) const;
	int rowCount(const QModelIndex & parent = QModelIndex()) const;

};

#endif /* COLLECTIONTREEFILTER_H_ */
