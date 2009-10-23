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

	qDebug() << "rows count requested";

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

	if (role != Qt::DisplayRole) {
		return QVariant();
	}

	CollectionTreeItem * item = static_cast<CollectionTreeItem*>(index.internalPointer());

	if (item->type() == CollectionTreeItem::Artist) {
		return item->data["name"].toString();
	}

	if (item->type() == CollectionTreeItem::Album) {
		return item->data["name"].toString();
	}

	if (item->type() == CollectionTreeItem::Track) {
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

/*
void CollectionItemModel::setArtistsAlbumsMode(const QString & mask)
{
	// Sets new tree.
	// First level - artists, second - albums, third - tracks
	// Use lazy loading: load actual data only when required

	QSqlDatabase db = QSqlDatabase::database();
	QSqlQuery query(db);

	// TODO: apply filter
	p->artists.clear();
	query.prepare("SELECT COUNT(*) FROM album WHERE artist_id=-1");
	if (!query.exec()) {
		// TODO: send fail signal
		return;
	}
	query.next();

	ArtistRecord r;
	r.id = -1;
	r.name = "[Various Artists]";
	r.type = CommonRecord::Artist;
	r.albumsCount = query.value(0).toInt();
	p->artists.append(r);


	query.prepare("SELECT artist.id, artist.name, COUNT(album.id) FROM artist INNER JOIN album ON artist.id=album.artist_id GROUP BY artist.id");
	if (!query.exec()) {
		// TODO: send fail signal
		return;
	}

	while (query.next()) {
		r.type = CommonRecord::Artist;
		r.name = query.value(1).toString();
		r.id = query.value(0).toInt();
		r.albumsCount = query.value(2).toInt();
		p->artists.append(r);
	}

}

void CollectionItemModel::loadAlbumsForArtists(ArtistRecord * artist) const
{
	if (!p->artistAlbums.contains(artist->id)) {
		// albums list not found so load it from the db
		QSqlDatabase db = QSqlDatabase::database();
		QSqlQuery query(db);

		query.prepare("SELECT name FROM album WHERE artist_id=:artistId");
		query.bindValue(":artistId", artist->id);
		if (!query.exec()) {
			// TODO: send fail signal
			return;
		}
		AlbumRecordsList albums;
		int n = 0;
		while (query.next()) {
			AlbumRecord album;
			album.parent = artist;
			album.name = query.value(0).toString();
			album.type = CommonRecord::Album;
			//album.row = n;
			albums.append(album);
			n++;
		}
		p->artistAlbums[artist->id] = albums;
	}
}
*/
