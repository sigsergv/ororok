/*
 * collectionitemdelegate.h
 *
 *  Created on: Oct 24, 2009
 *      Author: Sergei Stolyarov
 */

#ifndef COLLECTIONITEMDELEGATE_H_
#define COLLECTIONITEMDELEGATE_H_

#include <QItemDelegate>
//#include <QAbstractItemDelegate>

//class CollectionItemDelegate : public QAbstractItemDelegate
/**
 * Class for drawing collection tree elements
 */
class CollectionItemDelegate : public QItemDelegate
{
	Q_OBJECT
public:
	CollectionItemDelegate(QObject *parent = 0);
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index ) const;

};

#endif /* COLLECTIONITEMDELEGATE_H_ */
