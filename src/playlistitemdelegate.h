/*
 * playlistitemdelegate.h
 *
 *  Created on: Oct 30, 2009
 *      Author: Sergei Stolyarov
 */

#ifndef PLAYLISTITEMDELEGATE_H_
#define PLAYLISTITEMDELEGATE_H_

#include <QItemDelegate>

class PlaylistItemDelegate : public QItemDelegate
{
	Q_OBJECT
public:
	PlaylistItemDelegate(QObject *parent = 0);
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index ) const;

private:
	struct Private;
	Private * p;
};

#endif /* PLAYLISTITEMDELEGATE_H_ */
