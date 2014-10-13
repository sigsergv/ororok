/*
 * playlistitemdelegate.cpp
 *
 *  Created on: Oct 30, 2009
 *      Author: Sergey Stolyarov
 */

#include <QtGui>
#include <QtWidgets>
#include <QtDebug>

#include "playlistitemdelegate.h"
#include "playlistmodel.h"


struct PlaylistItemDelegate::Private
{
	QPen activeItemBorderPen;
	QBrush activeItemBgBrush;
	QBrush stateIconBgBrush;
	QBrush activeSelecteItemBgBrush;
};

PlaylistItemDelegate::PlaylistItemDelegate(QObject * parent)
	: QItemDelegate(parent)
{
	p = new Private;
	QLinearGradient g(0, 0, 0, 1);
	// TODO: set color from appliaction style
	g.setColorAt(0, QColor::fromRgb(255, 255, 255));
	g.setColorAt(0.5, QColor::fromRgb(200, 200, 200));
	g.setColorAt(1, QColor::fromRgb(255, 255, 255));
	g.setCoordinateMode(QGradient::ObjectBoundingMode);
	p->activeItemBgBrush = QBrush(g);

	g.setColorAt(0, QApplication::palette().color(QPalette::Highlight));
	//g.setColorAt(0.5, QColor::fromRgb(255, 255, 255));
	g.setColorAt(1, QApplication::palette().color(QPalette::Highlight));
	p->activeSelecteItemBgBrush = QBrush(g);

	p->stateIconBgBrush = QBrush(QColor(255, 0, 0));
}

void PlaylistItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	const QRect & baseRect = option.rect;
	painter->save();
	painter->setClipRect(baseRect, Qt::ReplaceClip);

	QRect displayRect(baseRect);

	PlaylistModel::ActiveTrackState state =
			static_cast<PlaylistModel::ActiveTrackState>(
					index.data(PlaylistModel::ItemTrackStateRole).toInt());

	if (PlaylistModel::TrackStateNotActive != state) {
		// draw custom background
		if (option.state & QStyle::State_Selected) {
			painter->setPen(QPen(QApplication::palette().color(QPalette::HighlightedText)));
			painter->fillRect(baseRect, p->activeSelecteItemBgBrush);
		} else {
			painter->fillRect(baseRect, p->activeItemBgBrush);
		}

		if (index.column() == 0) {
			QImage stateImage;

			// draw track playing state icon in the first column
			switch (state) {
			case PlaylistModel::TrackStatePaused:
				stateImage.load(":track-state-paused.png");
				break;

			case PlaylistModel::TrackStatePlaying:
				stateImage.load(":track-state-playing.png");
				break;

			case PlaylistModel::TrackStateStopped:
				stateImage.load(":track-state-stopped.png");
				break;
				
			case PlaylistModel::TrackStateNotActive:
				break;
			}

			QRect r(baseRect);
			r.adjust(0, 0, 0, -1);
			QPoint sp(r.topLeft());
			sp += QPoint(2, 0);
			sp.setY(r.center().y() - stateImage.height()/2 + 1);

			painter->drawImage(sp, stateImage);

			displayRect.adjust(stateImage.width()+1, 0, 0, 0);

		}
		// adjust rect for text
		//displayRect.adjust(0, 2, 0, 0);

		// set font style: italic
		QFont f = painter->font();
		f.setItalic(true);
		painter->setFont(f);
	} else if (option.state & QStyle::State_Selected) {
		// if item is selected draw the selected item background
		painter->setPen(QPen(QApplication::palette().color(QPalette::HighlightedText)));
		painter->fillRect(baseRect, QApplication::palette().highlight());
	}

	displayRect.adjust(3, 0, 0, 0);
	painter->drawText(displayRect, Qt::TextSingleLine | Qt::TextDontClip,
			index.data(Qt::DisplayRole).toString());

	painter->restore();
}
/*
QSize PlaylistItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
	QSize s = QItemDelegate::sizeHint(option, index);

	if (PlaylistModel::TrackStateNotActive != index.data(PlaylistModel::ItemTrackStateRole).toInt()) {
		// increase height of the active line
		//s += QSize(0, 4);
	}

	return s;
}
*/
