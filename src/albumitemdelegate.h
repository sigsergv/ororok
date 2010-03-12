/*
 * albumitemdelegate.h
 *
 *  Created on: Oct 24, 2009
 *      Author: Sergei Stolyarov
 */

#ifndef ALBUMITEMDELEGATE_H_
#define ALBUMITEMDELEGATE_H_

#include <QItemDelegate>
//#include <QAbstractItemDelegate>

//class AlbumItemDelegate : public QAbstractItemDelegate
/**
 * Class for drawing collection tree elements
 */
class AlbumItemDelegate : public QItemDelegate
{
	Q_OBJECT
public:
	AlbumItemDelegate(QObject *parent = 0);
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index ) const;

};

#endif /* ALBUMITEMDELEGATE_H_ */
