/*
 * playlistmodel.cpp
 *
 *  Created on: Oct 28, 2009
 *      Author: Sergei Stolyarov
 */

#include <QtXml>
#include <QtDebug>
#include <QtGui>
#include <QtSql>

#include "settings.h"
#include "playlistmodel.h"
#include "mimetrackinfo.h"
#include "xmlplaylistreader.h"

enum VisibleColumns{ VisColumnTitle=0, VisColumnArtist, VisColumnAlbum, VisColumnLength,
	VISIBLE_COLUMNS_NUM};

struct PlaylistModel::Private
{
	/**
	 * playlist UID, unique in the current session
	 */
	int puid;
	QString name;

	QList<QStringList> storage;
	QList<int> storageMap; // key is a visible column number, value - index in the "storage" row
	int activeTrackNum;
	ActiveTrackState activeTrackState;
	QString playlistFile;
};

PlaylistModel::PlaylistModel(const QString & playlistFile, QObject * parent)
	: QAbstractTableModel(parent)
{
	p = new Private;
	p->puid = Ororok::generateUid();
	p->activeTrackNum = -1;
	p->activeTrackState = TrackStatePlaying;
	p->playlistFile = playlistFile;

	p->storageMap << Ororok::TrackFieldTitle;  // VisColumnTitle
	p->storageMap << Ororok::TrackFieldArtist; // VisColumnArtist
	p->storageMap << Ororok::TrackFieldAlbum;  // VisColumnAlbum
	p->storageMap << Ororok::TrackFieldLength; // VisColumnLength

	// Playlist is an XML file
	// if there is no such file then playlist is empty
	// so write default playlist with default name
	QFileInfo fi(p->playlistFile);
	if (!fi.exists()) {
		p->name = QCoreApplication::translate("PlaylistWidget", "New Playlist");
		// create file
		QFile f(p->playlistFile);
		f.open(QIODevice::WriteOnly | QIODevice::Text);
		QXmlStreamWriter xml;
		xml.setDevice(&f);
		xml.writeStartDocument();
		xml.writeStartElement("playlist");
		xml.writeAttribute("version", "1.0");
		xml.writeStartElement("info");
		xml.writeTextElement("name", p->name);
		xml.writeEndElement();
		xml.writeEndDocument();
		f.close();
	} else {
		// open file, read tracks and insert them into the model
		QFile f(p->playlistFile);
		if (f.open(QIODevice::ReadOnly)) {
			Ororok::XmlPlaylistReader xpr(&f);

			QList<QStringList> playlistItems = xpr.tracks();
			p->name = xpr.name();
			f.close();

			beginInsertRows(QModelIndex(), 0, playlistItems.length()-1);
			foreach (QStringList trackInfo, playlistItems) {
				p->storage.append(trackInfo);
			}
			endInsertRows();
			qDebug() << "inserted rows" << playlistItems.length();
		}
	}
}

QString PlaylistModel::playlistName()
{
	return p->name;
}

/**
 * set playlist name in the playlist file
 * @param name new playlist name
 * @return
 */
QString PlaylistModel::setPlaylistName(const QString & name)
{
	p->name = name;
	flushPlaylistFile();
	return name;
}

void PlaylistModel::movePlaylistFile(const QString & newPath)
{
	QFile f(p->playlistFile);
	if (f.rename(newPath)) {
		p->playlistFile = newPath;
	}
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
		trackInfo[Ororok::TrackPlaylistId] = QString::number(p->puid);
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
	//Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
	Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled;

	if (!index.isValid()) {
		// allow drops only to root node
		flags |= Qt::ItemIsDropEnabled;
	}
	return flags;
}

QStringList PlaylistModel::mimeTypes() const
{
	QStringList mt;
	mt << Ororok::TRACKS_COLLECTION_IDS_MIME << Ororok::TRACKS_PLAYLIST_ITEMS_MIME;
	//mt << "text/uri-list";

	return mt;
}

QMimeData * PlaylistModel::mimeData(const QModelIndexList &indexes) const
{
	QMimeData *md = new QMimeData();
	QByteArray encodedData;
	QDataStream stream(&encodedData, QIODevice::WriteOnly);

	QList<int> rows;

	Q_FOREACH (const QModelIndex & index, indexes) {
		if (index.column() != 0) {
			continue;
		}
		rows << index.row();
	}

	stream << rows;
	//qDebug() << "Drag rows:" << rows;

	md->setData(Ororok::TRACKS_PLAYLIST_ITEMS_MIME, encodedData);
	return md;
}

bool PlaylistModel::dropMimeData(const QMimeData *data,
		Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
	if (action == Qt::IgnoreAction) {
		return true;
	}

	if (data->hasFormat(Ororok::TRACKS_COLLECTION_IDS_MIME)) {
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
		if (row > 0 && row < p->activeTrackNum) {
			p->activeTrackNum += newItems.count();
		}
		endInsertRows();
		flushPlaylistFile();
		return true;

	}

	if (data->hasFormat(Ororok::TRACKS_PLAYLIST_ITEMS_MIME)) {
		QByteArray encodedData = data->data(Ororok::TRACKS_PLAYLIST_ITEMS_MIME);

		QDataStream stream(&encodedData, QIODevice::ReadOnly);

		if (stream.atEnd()) {
			return false;
		}

		QList<int> rows;
		stream >> rows;

		// sort items if we want to keep order of tracks
		// TODO: add config option for this
		qSort(rows);

		// find position where to insert rows
		int insertCount = rows.count();

		// calculate new position of row "row"
		int insertPos = row;
		int newActiveTrackNum = p->activeTrackNum;
		int savedActiveTrackNum = p->activeTrackNum;
		int j=0;

		if (rows.contains(row-1)) {
			qDebug() << "contains";
			for (int i=row-1; i>0; i--) {
				if (!rows.contains(i)) {
					break;
				}
				j++;
			}
			insertPos -= j;
		}

		for (j=0; j<insertCount; j++) {
			 if (rows.at(j) >= insertPos) {
				 break;
			 }
		}
 		insertPos -= j;

		// delete tracks from the store (and save in temporary location)
		QList<QStringList> removedRecords;
		QListIterator<int> i(rows);
		i.toBack();

		int cr;
		while (i.hasPrevious()) {
			cr = i.previous();
			removedRecords.insert(0, p->storage.at(cr));
			removeRow(cr);
		}

		//qDebug() << "Drop lines on row:()" << row;
		//qDebug() << rows;
		//qDebug() << removedRecords;

		if (insertPos < 0) {
			return false;
		}

		if (rows.contains(savedActiveTrackNum)) {
			// if active track is among selected tracks then after inserting
			// it will be in position "insertPos+order", where "order" is a number
			// of track in the selected tracks list
			newActiveTrackNum = insertPos + rows.indexOf(savedActiveTrackNum);
		} else {
			// find total lines above "savedActiveTrackNum" before moving
			for (j=0; j<insertCount; j++) {
				if (rows.at(j) >= savedActiveTrackNum) {
					break;
				}
			}
			newActiveTrackNum -= j;
			if (insertPos <= newActiveTrackNum) {
				newActiveTrackNum += insertCount;
			}
		}

		// update current playing track position
		p->activeTrackNum = newActiveTrackNum;

		//insertRows(insertPos, rows.count());
		beginInsertRows(QModelIndex(), insertPos, insertPos + insertCount - 1);
		for (int i=0; i<insertCount; i++) {
			p->storage.insert(i+insertPos, removedRecords.at(i));
		}
		endInsertRows();
		flushPlaylistFile();

		return true;
	}

	return false;

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
	if (trackInfo[Ororok::TrackPlaylistId].toInt() != p->puid) {
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

bool PlaylistModel::removeRows(int row, int count, const QModelIndex & parent)
{
	// we also have to move pointer of currently playing song,
	// or remove it (and stop playing or switch to next track) if playing song was removed
	// currentTrack could move only
	qDebug() << "Delete " << count << "lines" << "begin from" << row;
	int lastRow = row + count - 1;
	beginRemoveRows(parent, row, lastRow);
	// delete rows from p->storage
	int offset = 0;
	for (int i=lastRow; i>=row; i--) {
		if (i < p->activeTrackNum) {
			offset++;
		}
		if (i == p->activeTrackNum) {
			qDebug() << "currently playing track deleted";
		}
		p->storage.removeAt(i);
	}
	p->activeTrackNum -= offset;
	endRemoveRows();
	flushPlaylistFile();
	return true;
}

// function inserts EMPTY rows so you have to alter them later
bool PlaylistModel::insertRows (int row, int count, const QModelIndex & parent)
{
	Q_UNUSED(parent);
	beginInsertRows(QModelIndex(), row, row + count - 1);
	for (int i=row; i<count+row; i++) {
		p->storage.insert(i, QStringList());
	}
	if (row < p->activeTrackNum) {
		p->activeTrackNum += count;
	}
	endInsertRows();
	flushPlaylistFile();
	return true;
}

int PlaylistModel::activeTrackNum()
{
	return 0;
}

// save playlist contents to playlist file
void PlaylistModel::flushPlaylistFile()
{
	QFile f(p->playlistFile);
	if (f.open(QIODevice::WriteOnly|QIODevice::Text)) {
		QXmlStreamWriter xml;
		xml.setAutoFormatting(true);
		xml.setDevice(&f);
		xml.writeStartDocument();
		xml.writeStartElement("playlist");
		xml.writeAttribute("version", "1.0");
		xml.writeStartElement("info");
		xml.writeTextElement("name", p->name);
		xml.writeEndElement();
		xml.writeStartElement("tracks");

		foreach (const QStringList & trackInfo, p->storage) {
			xml.writeStartElement("track");
			xml.writeAttribute("num", trackInfo[Ororok::TrackFieldNo]);
			xml.writeAttribute("title", trackInfo[Ororok::TrackFieldTitle]);
			xml.writeAttribute("year", trackInfo[Ororok::TrackFieldYear]);
			xml.writeAttribute("album", trackInfo[Ororok::TrackFieldAlbum]);
			xml.writeAttribute("artist", trackInfo[Ororok::TrackFieldArtist]);
			xml.writeAttribute("genre", trackInfo[Ororok::TrackFieldGenre]);
			xml.writeAttribute("length", trackInfo[Ororok::TrackFieldLength]);
			xml.writeCharacters(trackInfo[Ororok::TrackFieldPath]);
			xml.writeEndElement();
		}
		xml.writeEndElement();
		xml.writeEndDocument();
		f.close();

		// TODO: now process playlistItems
	}

}
