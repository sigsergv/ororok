/*
 * playlistitemdelegate.cpp
 *
 *  Created on: Oct 30, 2009
 *      Author: Sergei Stolyarov
 */

#include <QtGui>
#include <QtDebug>

#include "playlistitemdelegate.h"
#include "playlistmodel.h"


struct PlaylistItemDelegate::Private
{
	QPen activeItemBorderPen;
	QBrush activeItemBgBrush;
};

PlaylistItemDelegate::PlaylistItemDelegate(QObject * parent)
	: QItemDelegate(parent)
{
	p = new Private;
	p->activeItemBorderPen.setWidth(2);
	p->activeItemBgBrush.setStyle(Qt::SolidPattern);
	p->activeItemBgBrush.setColor(QColor(200, 200, 200));
}

void PlaylistItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if (index.data(PlaylistModel::ItemTrackStateRole).toBool()) {
		painter->save();

		const QRect & baseRect = option.rect;
		QRect r(baseRect);
		r.adjust(0, 1, 0, 0);

		// draw fancy item
		painter->setPen(p->activeItemBorderPen);

		painter->fillRect(baseRect, p->activeItemBgBrush);
		QLine topLine(r.topLeft(), r.topRight());
		QLine bottomLine(r.bottomLeft(), r.bottomRight());
		painter->drawLine(topLine);
		painter->drawLine(bottomLine);

		QRect displayRect(baseRect);
		displayRect.adjust(3, 2, 0, 0);
		painter->drawText(displayRect, index.data(Qt::DisplayRole).toString());

		painter->restore();
		return;
	}
	QItemDelegate::paint(painter, option, index);
}

QSize PlaylistItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
	QSize s = QItemDelegate::sizeHint(option, index);

	if (index.data(PlaylistModel::ItemTrackStateRole).toBool()) {
		// add some pixels
		s += QSize(0, 4);
	}

	return s;
}
