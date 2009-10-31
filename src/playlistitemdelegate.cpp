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
	QLinearGradient g(0, 0, 0, 1);
	// TODO: set color from appliaction style
	g.setColorAt(0, QColor::fromRgb(200, 200, 200));
	g.setColorAt(0.5, QColor::fromRgb(255, 255, 255));
	g.setColorAt(1, QColor::fromRgb(200, 200, 200));
	g.setCoordinateMode(QGradient::ObjectBoundingMode);
	p->activeItemBgBrush = QBrush(g);
	//p->activeItemBgBrush.setStyle(Qt::LinearGradientPattern);
	//p->activeItemBgBrush.setColor(QColor(200, 200, 200));
}

void PlaylistItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	const QStyle * style = QApplication::style();
	const QRect & baseRect = option.rect;
	painter->save();

	QRect displayRect(baseRect);

	//painter->fillRect(QRect(100,100,200,200), p->activeItemBgBrush);

	if (index.data(PlaylistModel::ItemTrackStateRole).toBool()) {
		// draw custom background
		painter->fillRect(baseRect, p->activeItemBgBrush);

		// draw active track border (top and bottom lines)
		QRect r(baseRect);
		r.adjust(0, 1, 0, 0);
		QLine topLine(r.topLeft(), r.topRight());
		QLine bottomLine(r.bottomLeft(), r.bottomRight());
		painter->setPen(p->activeItemBorderPen);
		painter->drawLine(topLine);
		painter->drawLine(bottomLine);

		// adjust rect for text
		displayRect.adjust(0, 2, 0, 0);

		// set font style
		QFont f = painter->font();
		f.setItalic(true);
		painter->setFont(f);
	}

	displayRect.adjust(3, 0, 0, 0);
	painter->drawText(displayRect, index.data(Qt::DisplayRole).toString());

	painter->restore();
	// draw text
	//QItemDelegate::paint(painter, option, index);
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
