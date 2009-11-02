/*
 * collectionitemmodel.cpp
 *
 *  Created on: Oct 22, 2009
 *      Author: Sergei Stolyarov
 */

#include <QtDebug>
#include <QtSql>

#include "collectionitemmodel.h"
#include "collectiontreeitem.h"
#include "mimetrackinfo.h"

struct CollectionItemModel::Private
{
	CollectionTreeItem * rootTreeItem;
};


CollectionItemModel::CollectionItemModel(QObject * parent)
	: QAbstractItemModel(parent)
{
	p = new Private;
	// create empty tree
	p->rootTreeItem = new CollectionTreeItem(CollectionTreeItem::Root, 0);
	p->rootTreeItem->fetchData();
}

QModelIndex CollectionItemModel::index(int row, int column, const QModelIndex & parent) const
{
	if (!hasIndex(row, column, parent)) {
		return QModelIndex();
	}

	CollectionTreeItem * parentItem;

	if (!parent.isValid()) {
		// i.e. top-level element
		parentItem = p->rootTreeItem;
	} else {
		parentItem = static_cast<CollectionTreeItem*>(parent.internalPointer());
	}

	CollectionTreeItem * item = parentItem->child(row);

	if (item) {
		return createIndex(row, 0, item);
	} else {
		return QModelIndex();
	}
}

QModelIndex CollectionItemModel::parent(const QModelIndex & index) const
{
	if (!index.isValid()) {
		return QModelIndex();
	}

	CollectionTreeItem * item = static_cast<CollectionTreeItem*>(index.internalPointer());
	CollectionTreeItem * parentItem = item->parent();

	if (!parentItem || parentItem == p->rootTreeItem) {
		return QModelIndex();
	}

	return createIndex(parentItem->row, 0, parentItem);
}

int CollectionItemModel::rowCount(const QModelIndex & parent) const
{
	CollectionTreeItem * parentItem;

	//qDebug() << "rows count requested";

	if (!parent.isValid()) {
		parentItem = p->rootTreeItem;
	} else {
		parentItem = static_cast<CollectionTreeItem*>(parent.internalPointer());
	}

	return parentItem->childrenCount();
}

int CollectionItemModel::columnCount(const QModelIndex &) const
{
	return 1;
}

QVariant CollectionItemModel::data(const QModelIndex & index, int role) const
{
	if (!index.isValid()) {
		return QVariant();
	}

	CollectionTreeItem * item = static_cast<CollectionTreeItem*>(index.internalPointer());
	int itemType = item->type();

	if (ItemTypeRole == role) {
		return item->type();
	}

	if (ItemAlbumCoverRole == role && CollectionTreeItem::Album == itemType) {
		return item->data["cover_path"];
	}

	if (ItemAlbumNameRole == role && CollectionTreeItem::Album == itemType) {
		return item->data["name"];
	}

	if (ItemQuickSearchMatchedRole == role) {
		// return QString object that contain
		return item->matched;
	}

	if (ItemTrackInfoRole == role && CollectionTreeItem::Track == itemType) {
		QStringList ti;
		// four reserved positions
		ti << QString() << QString() << QString() << QString();
		ti << QString("%1/%2").arg(item->data.value("path").toString())
				.arg(item->data.value("filename").toString());
		ti << item->data.value("track", "0").toString();
		ti << item->data.value("title", "").toString();
		ti << item->data.value("year", "").toString();
		CollectionTreeItem * parent = item->parent();
		if (CollectionTreeItem::Album == parent->type()) {
			ti << parent->data.value("name", "").toString();
		} else {
			ti << "";
		}

		ti << item->data.value("artist", "").toString();
		ti << item->data.value("genre", "").toString();
		ti << item->data.value("length", "0").toString();

		return ti;
	}

	if (role != Qt::DisplayRole) {
		return QVariant();
	}

	if (CollectionTreeItem::Artist == itemType) {
		return item->data["name"].toString();
	}

	if (CollectionTreeItem::Album == itemType) {
		QString albumTitle;
		int year = item->data["year"].toInt();
		if (year > 0) {
			albumTitle += QString("%1 - ").arg(year);
		}
		albumTitle += item->data["name"].toString();
		// item->data["cover_path"].toString()
		return albumTitle;
	}

	if (CollectionTreeItem::Track == itemType) {
		QString trackTitle;
		int trackNum = item->data["track"].toInt();
		if (trackNum != 0) {
			trackTitle = QString("%1 - ").arg(trackNum);
		}
		trackTitle += item->data["title"].toString();
		return trackTitle;
	}

	return QString("type: %1").arg(item->type());
}

void CollectionItemModel::markItemsMatchString(const QString & match)
{
	p->rootTreeItem->markItemMatchString(match);
}

Qt::ItemFlags CollectionItemModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);

	if (index.isValid()) {
		return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
	} else {
		return Qt::ItemIsDropEnabled | defaultFlags;
	}
}

QStringList CollectionItemModel::mimeTypes() const
{
	QStringList mt;

	mt << Ororok::TRACKS_MIME;

	return mt;
}

QMimeData * CollectionItemModel::mimeData(const QModelIndexList &indexes) const
{
	QMimeData *md = new QMimeData();
	QByteArray encodedData;
	QDataStream stream(&encodedData, QIODevice::WriteOnly);
	// we should form list of tracks, but list of indexes may contain albums or artists
	// so we have to extract tracks

	QModelIndexList trackIndexes;

	Q_FOREACH (const QModelIndex & index, indexes) {
		if (!index.isValid()) {
			continue;
		}
		if (index.data(CollectionItemModel::ItemTypeRole) == CollectionTreeItem::Track) {
			trackIndexes << index;
			continue;
		}
		findTracksInIndexesTree(index, trackIndexes);
	}

	Q_FOREACH (const QModelIndex & index, trackIndexes) {
		// [trackPath, trackNo, title, album, year, album, artist, genre]
		QStringList trackInfo = index.data(CollectionItemModel::ItemTrackInfoRole).toStringList();
		//stream << Ororok::serializeTrackInfo(trackInfo);
		stream << trackInfo;
	}
	md->setData(Ororok::TRACKS_MIME, encodedData);

	return md;
}

void CollectionItemModel::findTracksInIndexesTree(const QModelIndex & index, QModelIndexList & target) const
{
	if (index.data(CollectionItemModel::ItemTypeRole) == CollectionTreeItem::Track) {
		target << index;
		return;
	}

	int count = rowCount(index);

	for (int i=0; i<count; i++) {
		findTracksInIndexesTree(index.child(i, 0), target);
	}
}

bool CollectionItemModel::reloadData()
{
	// destroy data set
	delete p->rootTreeItem;
	// create new
	p->rootTreeItem = new CollectionTreeItem(CollectionTreeItem::Root, 0);
	return true;
}
