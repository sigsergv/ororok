/*
 * playlistmodel.cpp
 *
 *  Created on: Oct 28, 2009
 *      Author: Sergei Stolyarov
 */

#include <QtDebug>
#include <QtGui>
#include <QtSql>

#include "settings.h"
#include "playlistmodel.h"
#include "mimetrackinfo.h"

enum VisibleColumns{ VisColumnTitle=0, VisColumnArtist, VisColumnAlbum, VisColumnLength,
	VISIBLE_COLUMNS_NUM};

struct PlaylistModel::Private
{
	int uid;
	QList<QStringList> storage;
	QList<int> storageMap; // key is a visible column number, value - index in the "storage" row
	int activeTrackNum;
	ActiveTrackState activeTrackState;
};

PlaylistModel::PlaylistModel(QObject * parent)
	: QAbstractTableModel(parent)
{
	p = new Private;
	p->uid = Ororok::generateUid();
	p->activeTrackNum = -1;
	p->activeTrackState = TrackStatePlaying;

	p->storageMap << Ororok::TrackFieldTitle;  // VisColumnTitle
	p->storageMap << Ororok::TrackFieldArtist; // VisColumnArtist
	p->storageMap << Ororok::TrackFieldAlbum;  // VisColumnAlbum
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

	if (ItemTrackInfoRole == role) {
		QStringList trackInfo = p->storage[index.row()];
		trackInfo[Ororok::TrackPlaylistId] = QString::number(p->uid);
		trackInfo[Ororok::TrackNumInPlaylist] = QString::number(index.row());;
		return trackInfo;
	}

	if (ItemTrackStateRole == role) {
		if (index.row() != p->activeTrackNum) {
			return TrackStateNotActive;
		}
		return p->activeTrackState;
	}

	if (Qt::DisplayRole != role) {
		return QVariant();
	}

	QVariant res;

	switch (static_cast<VisibleColumns>(index.column())) {
	case VisColumnArtist:
	case VisColumnTitle:
	case VisColumnAlbum:
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
		case VisColumnAlbum:
			res = tr("Album");
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
	mt << Ororok::TRACKS_COLLECTION_IDS_MIME;
	//mt << "text/uri-list";

	return mt;
}

bool PlaylistModel::dropMimeData(const QMimeData *data,
		Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
	if (action == Qt::IgnoreAction) {
		return true;
	}

	if (!data->hasFormat(Ororok::TRACKS_COLLECTION_IDS_MIME)) {
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

	QByteArray encodedData = data->data(Ororok::TRACKS_COLLECTION_IDS_MIME);
	qDebug() << encodedData;
	QDataStream stream(&encodedData, QIODevice::ReadOnly);
	QStringList newItemsIds;
	QList<QStringList> newItems;

	while (!stream.atEnd()) {
		int id;
		stream >> id;
		newItemsIds << QString::number(id);
	}

	QSqlDatabase db = QSqlDatabase::database();
	QSqlQuery query(db);

	QString queryStr = QString("SELECT t.id, t.title, t.filename, t.track, t.length, g.name, d.path, " // 0-6
			"ar.name, t.year, al.name, t.length "
			"FROM track t "
			"LEFT JOIN genre g ON g.id=t.genre_id "
			"LEFT JOIN artist ar ON ar.id=t.artist_id "
			"LEFT JOIN album al ON al.id=t.album_id "
			"LEFT JOIN dir d ON d.id=t.dir_id WHERE t.id IN (%1)").arg(newItemsIds.join(", "));

	if (!query.exec(queryStr)) {
		// TODO: display warning or something like
		qDebug() << "query failed:" << query.lastError().text();
		return false;
	}

	int rows = 0;
	while (query.next()) {
		QStringList ti;
		// reserved 4 fields
		ti << QString() << QString() << QString() << QString();

		// filepath
		ti << QString("%1/%2")
				.arg(query.value(6).toString())
				.arg(query.value(2).toString());
		ti << query.value(3).toString(); // num
		ti << query.value(1).toString(); // track title
		ti << query.value(8).toString(); // year
		ti << query.value(9).toString(); // album
		ti << query.value(7).toString(); // artist
		ti << query.value(5).toString(); // genre
		ti << query.value(10).toString(); // length
		newItems << ti;
		rows++;
	}

	beginInsertRows(QModelIndex(), beginRow, beginRow+rows-1);
	foreach (const QStringList & trackInfo, newItems) {
		// fetch track from db
		p->storage.insert(beginRow, trackInfo);
		beginRow++;
	}
	endInsertRows();
	//qDebug() << "inserted rows: " << rows;
	return true;
}

void PlaylistModel::selectActiveTrack(int n)
{
	int oldActiveTrackNum = p->activeTrackNum;

	if (n < 0 || n > rowCount(QModelIndex())) {
		return;
	}

	p->activeTrackNum = n;

	// emit data change signal
	emit dataChanged(index(oldActiveTrackNum,0), index(oldActiveTrackNum,VISIBLE_COLUMNS_NUM-1));
	emit dataChanged(index(n,0), index(n,VISIBLE_COLUMNS_NUM-1));
}

bool PlaylistModel::selectActiveTrack(const QStringList & trackInfo)
{
	if (trackInfo[Ororok::TrackPlaylistId].toInt() != p->uid) {
		// not our track, ignore
		return false;
	}

	int n = trackInfo[Ororok::TrackNumInPlaylist].toInt();
	selectActiveTrack(n);
	return true;
}

QStringList PlaylistModel::activeTrack()
{
	QStringList trackInfo;
	if (p->activeTrackNum >= 0) {
		trackInfo = index(p->activeTrackNum, 0).data(ItemTrackInfoRole).toStringList();
	} else {
		// take first track
		trackInfo = index(0, 0).data(ItemTrackInfoRole).toStringList();
	}
	return trackInfo;
}

QStringList PlaylistModel::trackAfterActive()
{
	QStringList trackInfo;

	if (p->activeTrackNum >= 0) {
		trackInfo = index(p->activeTrackNum+1, 0).data(ItemTrackInfoRole).toStringList();;
	}

	return trackInfo;
}

QStringList PlaylistModel::trackBeforeActive()
{
	QStringList trackInfo;

	if (p->activeTrackNum >= 0) {
		trackInfo = index(p->activeTrackNum-1, 0).data(ItemTrackInfoRole).toStringList();;
	}

	return trackInfo;
}

void PlaylistModel::markActiveTrackStarted()
{
	p->activeTrackState = TrackStatePlaying;
	if (p->activeTrackNum >= 0) {
		emit dataChanged(index(p->activeTrackNum, 0), index(p->activeTrackNum, VISIBLE_COLUMNS_NUM-1));
	}
}

void PlaylistModel::markActiveTrackStopped()
{
	p->activeTrackState = TrackStateStopped;
	if (p->activeTrackNum >= 0) {
		emit dataChanged(index(p->activeTrackNum, 0), index(p->activeTrackNum, VISIBLE_COLUMNS_NUM-1));
	}
	p->activeTrackNum = -1;
}

void PlaylistModel::markActiveTrackPaused()
{
	p->activeTrackState = TrackStatePaused;
	if (p->activeTrackNum >= 0) {
		emit dataChanged(index(p->activeTrackNum, 0), index(p->activeTrackNum, VISIBLE_COLUMNS_NUM-1));
	}
}

void PlaylistModel::markActiveTrackPlaying()
{
	p->activeTrackState = TrackStatePlaying;
	if (p->activeTrackNum >= 0) {
		emit dataChanged(index(p->activeTrackNum, 0), index(p->activeTrackNum, VISIBLE_COLUMNS_NUM-1));
	}
}
PlaylistModel::ActiveTrackState PlaylistModel::activeTrackState()
{
	return p->activeTrackState;
}
