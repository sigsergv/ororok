/*
 * playlistmodel.cpp
 *
 *  Created on: Oct 28, 2009
 *      Author: Sergei Stolyarov
 */

#include <QtDebug>
#include <QtGui>

#include "playlistmodel.h"
#include "mimetrackinfo.h"

enum VisibleColumns{ VisColumnTitle=0, VisColumnArtist, VisColumnLength,
	VISIBLE_COLUMNS_NUM};

struct PlaylistModel::Private
{
	QList<QStringList> storage;
	QList<int> storageMap; // key is a visible column number, value - index in the "storage" row
};

PlaylistModel::PlaylistModel(QObject * parent)
	: QAbstractTableModel(parent)
{
	p = new Private;

	p->storageMap << Ororok::TrackFieldTitle;  // VisColumnTitle
	p->storageMap << Ororok::TrackFieldArtist; // VisColumnArtist
	p->storageMap << Ororok::TrackFieldLength; // VisColumnLength

}

int PlaylistModel::rowCount(const QModelIndex & index) const
{
	if (index.isValid()) {
		return 0;
	}
	return p->storage.count();
}

int PlaylistModel::columnCount(const QModelIndex & index) const
{
	if (index.isValid()) {
		return 0;
	}
	return VISIBLE_COLUMNS_NUM;
}

QVariant PlaylistModel::data(const QModelIndex & index, int role) const
{
	if (!index.isValid()) {
		return QVariant();
	}

	if (index.row() >= p->storage.count()) {
		return QVariant();
	}
	if (index.column() >= VISIBLE_COLUMNS_NUM || index.column() < 0) {
		return QVariant();
	}

	if (Qt::DisplayRole != role) {
		return QVariant();
	}

	QVariant res;

	switch (static_cast<VisibleColumns>(index.column())) {
	case VisColumnArtist:
	case VisColumnTitle:
		res = p->storage[index.row()].value(p->storageMap[index.column()], QString());
		break;

	case VisColumnLength: {
#if 0
			int time = p->storage[index.row()].value(p->storageMap[index.column()], "0").toInt();

			int seconds = time % 60;
			time /= 60;
			int minutes = time % 60;
			time /= 60;
			int hours = time % 24;
			if (hours) {
				res = QString("%1:%2:%3").arg(hours, 2).arg(minutes, 2).arg(seconds, 2);
			} else {
				res = QString("%1:%2").arg(minutes, 2).arg(seconds, 2);
			}
#else
			int t = p->storage[index.row()].value(p->storageMap[index.column()], "0").toInt();
			QTime time(0, 0, 0);
			time = time.addSecs(t);
//			/time.addSecs(t);
			if (time.hour()) {
				res = time.toString("HH:mm:ss");
			} else {
				res = time.toString("mm:ss");
			}
#endif
		}
		break;

	case VISIBLE_COLUMNS_NUM:
		break;
	}

	return res;
}

QVariant PlaylistModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	Q_UNUSED(orientation);

	QVariant res;

	if (Qt::DisplayRole == role) {
		switch (static_cast<VisibleColumns>(section)) {
		case VisColumnArtist:
			res = tr("Artist");
			break;
		case VisColumnLength:
			res = tr("Length");
			break;
		case VisColumnTitle:
			res = tr("Title");
			break;
		case VISIBLE_COLUMNS_NUM:
			break;
		}
	}

	return res;
}

Qt::ItemFlags PlaylistModel::flags(const QModelIndex & index) const
{
	Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);

	return Qt::ItemIsDropEnabled | defaultFlags;
}

QStringList PlaylistModel::mimeTypes() const
{
	QStringList mt;
	mt << Ororok::TRACKS_MIME;
	//mt << "text/uri-list";

	return mt;
}

bool PlaylistModel::dropMimeData(const QMimeData *data,
		Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
	if (action == Qt::IgnoreAction) {
		return true;
	}

	if (!data->hasFormat(Ororok::TRACKS_MIME)) {
		return false;
	}

	if (column > 0) {
		return false;
	}

	int beginRow;
	if (row != -1) {
		beginRow = row;
	} else if (parent.isValid()) {
		beginRow = parent.row();
	} else {
		beginRow = rowCount(QModelIndex());
	}

	QByteArray encodedData = data->data(Ororok::TRACKS_MIME);
	QDataStream stream(&encodedData, QIODevice::ReadOnly);
	QList<QStringList> newItems;
	int rows = 0;

	while (!stream.atEnd()) {
		QStringList ti;
		stream >> ti;
		newItems << ti;
		rows++;
	}

	beginInsertRows(QModelIndex(), beginRow, beginRow+rows);
	foreach (const QStringList & trackInfo, newItems) {
		p->storage.insert(beginRow, trackInfo);
		beginRow++;
	}
	endInsertRows();
	//qDebug() << "inserted rows: " << rows;
	return true;
}

