/*
 * collectiontreefilter.cpp
 *
 *  Created on: Oct 26, 2009
 *      Author: Sergei Stolyarov
 */

#include <QtCore>
#include <QtDebug>

#include "collectiontreefilter.h"
#include "collectionitemmodel.h"
#include "collectiontreeitem.h"
#include "mimetrackinfo.h"

CollectionTreeFilter::CollectionTreeFilter(int role, QObject * parent) :
	QSortFilterProxyModel(parent), matchRole(role) {
}

bool CollectionTreeFilter::filterAcceptsRow(int source_row,
		const QModelIndex & source_parent) const {
	QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
	bool res = idx.data(matchRole).toBool();
	if (matchRole == CollectionItemModel::ItemDatePeriodMatchedRole) {
		//qDebug() << res;
	}
	return res;
}

QMimeData * CollectionTreeFilter::mimeData(const QModelIndexList &indexes) const
{
	//QMimeData * md = QSortFilterProxyModel::mimeData(indexes);

	QByteArray encodedData;
	QDataStream stream(&encodedData, QIODevice::WriteOnly);
	// we should form list of tracks, but list of indexes may contain albums or artists
	// so we have to extract tracks

	QModelIndexList trackIndices;

	Q_FOREACH (const QModelIndex & index, indexes) {
		if (!index.isValid()) {
			continue;
		}
		if (index.data(CollectionItemModel::ItemTypeRole) == CollectionTreeItem::Track) {
			trackIndices << index;
			continue;
		}
		findTracksInIndexesTree(index, trackIndices);
	}

	QStringList ids;

	Q_FOREACH (const QModelIndex & index, trackIndices) {
		// [trackPath, trackNo, title, album, year, album, artist, genre]
		int id = index.data(CollectionItemModel::ItemDbIdRole).toInt();
		stream << id;
	}
	QMimeData *md = new QMimeData();
	md->setData(Ororok::TRACKS_COLLECTION_IDS_MIME, encodedData);

	qDebug() << "total indexes" << trackIndices.count();
	return md;
}

void CollectionTreeFilter::findTracksInIndexesTree(const QModelIndex & index, QModelIndexList & target) const
{
	if (index.data(CollectionItemModel::ItemTypeRole) == CollectionTreeItem::Track) {
		target << index;
		return;
	}

	//CollectionTreeItem * ix;
	//ix = static_cast<CollectionTreeItem*>(index.internalPointer());

    int itemType = index.data(CollectionItemModel::ItemTypeRole).toInt();
    if (itemType == CollectionTreeItem::Artist
            && index.data(CollectionItemModel::ItemArtistLetterRole).toBool())
    {
        // we should not scan "letter" tree items, otherwise application will crash
        return;
    }
    int count = index.model()->rowCount(index);

	for (int i=0; i<count; i++) {
		findTracksInIndexesTree(index.child(i, 0), target);
	}
}

