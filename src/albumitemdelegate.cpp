/*
 * albumitemdelegate.cpp
 *
 *  Created on: Oct 24, 2009
 *      Author: Sergei Stolyarov
 */
#include <QtGui>
#include <QtDebug>

#include "albumitemdelegate.h"
#include "collectiontreeitem.h"
#include "collectionitemmodel.h"
#include "settings.h"
#include "cache.h"

AlbumItemDelegate::AlbumItemDelegate(QObject *parent)
//		: QAbstractItemDelegate(parent)
		: QItemDelegate(parent)
{

}

void AlbumItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	// get note type
	int itemType = index.data(CollectionItemModel::ItemTypeRole).toInt();

	//qDebug() << "type: " << itemType;

	if (itemType == CollectionTreeItem::Album) {
		// display album name and other info
		QPalette palette = QApplication::palette();
		QBrush b;
		QRect baseRect = option.rect;

		if (option.state & QStyle::State_Selected) {
			// draw selected frame
			painter->setPen(QPen(palette.color(QPalette::HighlightedText)));
			b = palette.highlight();
		} else {
			// draw not-selected frame
			painter->setPen(QPen(palette.color(QPalette::Text)));
			b = palette.brush(QPalette::Active, QPalette::Base);
		}
		painter->fillRect(option.rect, b);

		QRect r(baseRect);

		// draw icon
		QString coverPath = index.data(CollectionItemModel::ItemAlbumCoverRole).toString();
		if (!coverPath.isEmpty()) {
			QPixmap icon = Ororok::cachedImage(coverPath, Ororok::ImageSizeIcon);
			if (!icon.isNull()) {
				QPoint iconPos(r.topLeft());
				iconPos += QPoint(Ororok::ALBUM_ITEM_PADDING, Ororok::ALBUM_ITEM_PADDING);
				painter->drawPixmap(iconPos, icon);
			}
		}

		// draw icon frame
		r.adjust(Ororok::ALBUM_ITEM_PADDING, Ororok::ALBUM_ITEM_PADDING, 0, 0);
		r.setWidth(Ororok::ALBUM_ICON_SIZE);
		r.setHeight(Ororok::ALBUM_ICON_SIZE);
		painter->drawRect(r);

		// draw album title
		QPoint textPos;

		textPos.setX(baseRect.topLeft().x() + Ororok::ALBUM_ICON_SIZE + Ororok::ALBUM_ITEM_PADDING*3);
		textPos.setY((baseRect.topLeft().y() + baseRect.bottomLeft().y())/2);
		painter->drawText(textPos, index.data(Qt::DisplayRole).toString());
		return;
	}
	QItemDelegate::paint(painter, option, index);
}

QSize AlbumItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
	int itemType = index.data(CollectionItemModel::ItemTypeRole).toInt();
	QSize s = QItemDelegate::sizeHint(option, index);

	if (itemType == CollectionTreeItem::Album) {
		// change height
		s.setHeight(Ororok::ALBUM_ICON_SIZE+2*Ororok::ALBUM_ITEM_PADDING+1);
	}

	return s;
}
