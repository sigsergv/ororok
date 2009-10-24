/*
 * CollectionTreeItem.cpp
 *
 *  Created on: Oct 24, 2009
 *      Author: Sergei Stolyarov
 */

#include <QHash>
#include <QtSql>
#include <QtDebug>

#include "collectiontreeitem.h"

struct CollectionTreeItem::Private
{
	CollectionTreeItemType type;
	QHash<int, CollectionTreeItem*> childItems;
	CollectionTreeItem * parentItem;
	int row;
	int rowsCount;
};

CollectionTreeItem::CollectionTreeItem(CollectionTreeItemType t, CollectionTreeItem * parent)
{
	p = new Private;
	p->type = t;
	p->parentItem = parent;
	p->rowsCount = -1;
}

CollectionTreeItem::~CollectionTreeItem()
{
	// TODO: delete all elements from p->childItems
	delete p;
}

CollectionTreeItem::CollectionTreeItemType CollectionTreeItem::type()
{
	return p->type;
}

CollectionTreeItem * CollectionTreeItem::child(int n)
{
	return p->childItems[n];
}

CollectionTreeItem * CollectionTreeItem::parent()
{
	return p->parentItem;
}

/**
 * Return number of child elements.
 *
 * @return
 */
int CollectionTreeItem::childrenCount()
{
	if (p->rowsCount == -1) {
		// fetch child items now
		qDebug() << "fetching items";
		if (p->type == Artist) {
			qDebug() << "fetch albums";
			// child elements are albums
			int artistId = p->parentItem->data["id"].toInt();
			fetchAlbums(this);
		} else if (p->type == Album) {
			// child elements are tracks
			qDebug() << "fetch tracks";
			fetchTracks(this);
		} else if (p->type == Root) {
			// child elements are artists
			qDebug() << "fetch artists";
			fetchArtists(this);
		}
	}

	return p->rowsCount;
}

void CollectionTreeItem::fetchArtists(CollectionTreeItem * parent)
{
	// load all artists from the database
	QSqlDatabase db = QSqlDatabase::database();
	QSqlQuery query(db);

	p->childItems.clear();

	// TODO: apply filter
	query.prepare("SELECT COUNT(*) FROM album WHERE artist_id=-1");
	if (!query.exec()) {
		// TODO: send fail signal?
		return;
	}
	query.next();

	int n = 0;

	CollectionTreeItem * artist;
	artist = new CollectionTreeItem(Artist, parent);
	artist->data["id"] = -1;
	artist->data["name"] = "{Variouse Artists}";
	artist->row = n;
	p->childItems[n] = artist;

	query.prepare("SELECT artist.id, artist.name, COUNT(album.id) FROM artist INNER JOIN album ON artist.id=album.artist_id GROUP BY artist.id");
	if (!query.exec()) {
		// TODO: send fail signal?
		return;
	}

	while (query.next()) {
		n++;
		artist = new CollectionTreeItem(Artist, parent);
		artist->data["id"] = query.value(0);
		artist->data["name"] = query.value(1);
		artist->row = n;
		p->childItems[n] = artist;
	}

	p->rowsCount = n + 1;
}

/**
 * fetch all albums for given artist
 */
void CollectionTreeItem::fetchAlbums(CollectionTreeItem * parent)
{
	QSqlDatabase db = QSqlDatabase::database();
	QSqlQuery query(db);

	query.prepare("SELECT album.id, album.name, image.path FROM album LEFT JOIN image ON "
			"album.image_id=image.id WHERE artist_id=:artistId ");
	query.bindValue(":artistId", parent->data["id"]);

	if (!query.exec()) {
		// TODO: send fail signal?
		return;
	}

	CollectionTreeItem * album;

	int n = 0;

	while (query.next()) {
		album = new CollectionTreeItem(Album, parent);
		album->data.clear();
		album->data["id"] = query.value(0);
		album->data["name"] = query.value(1);
		album->data["cover_path"] = query.value(2);
		album->row = n;
		p->childItems[n] = album;
		n++;
	}

	p->rowsCount = n;
}

/**
 *fetch tracks for selected album
 * @param parent
 */
void CollectionTreeItem::fetchTracks(CollectionTreeItem * parent)
{
	QSqlDatabase db = QSqlDatabase::database();
	QSqlQuery query(db);

	query.prepare("SELECT id, title, filename, track FROM track WHERE album_id=:albumId");
	query.bindValue(":albumId", parent->data["id"]);

	if (!query.exec()) {
		// TODO: send fail signal?
		return;
	}

	CollectionTreeItem * track;

	int n = 0;

	while (query.next()) {
		track = new CollectionTreeItem(Track, parent);
		track->data.clear();
		track->data["id"] = query.value(0);
		track->data["title"] = query.value(1);
		track->data["filename"] = query.value(2);
		track->data["track"] = query.value(3);
		track->row = n;
		p->childItems[n] = track;
		n++;
	}

	p->rowsCount = n;
}
