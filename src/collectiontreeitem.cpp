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
	matched = true;
	searchString = "";
}

CollectionTreeItem::~CollectionTreeItem()
{
	// TODO: delete all elements from p->childItems
	QHashIterator<int, CollectionTreeItem*> i(p->childItems);
	QList<CollectionTreeItem*> itemsToDelete;

	while (i.hasNext()) {
		itemsToDelete << *i.next();
	}
	p->childItems.clear();
	Q_FOREACH (CollectionTreeItem * item, itemsToDelete) {
		delete item;
	}
	delete p;

	qDebug() << ">> deleted tree item";
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
	return p->rowsCount;
	/*
	if (p->rowsCount == -1) {
		// fetch child items now
		//qDebug() << "fetching items";
		if (p->type == Artist) {
			//qDebug() << "fetch albums";
			// child elements are albums
			int artistId = p->parentItem->data["id"].toInt();
			fetchAlbums(this);
		} else if (p->type == Album) {
			// child elements are tracks
			//qDebug() << "fetch tracks";
			fetchTracks(this);
		} else if (p->type == Root) {
			// child elements are artists
			//qDebug() << "fetch artists";
			fetchArtists(this);
		}
	}

	return p->rowsCount;
	*/
}

void CollectionTreeItem::fetchData()
{
	if (p->type == Artist) {
		//qDebug() << "fetch albums";
		// child elements are albums
		fetchAlbums(this);
	} else if (p->type == Album) {
		// child elements are tracks
		//qDebug() << "fetch tracks";
		fetchTracks(this);
	} else if (p->type == Root) {
		// child elements are artists
		//qDebug() << "fetch artists";
		fetchArtists(this);
	}
}


/**
 * mark all items that match given string "match"
 * @param match
 */
bool CollectionTreeItem::markItemMatchString(const QString & match)
{
	if (searchString.contains(match, Qt::CaseInsensitive)) {
		// mark this and all child nodes as matched
		//qDebug() << "matched" << searchString;
		markChildrenMatched(true);
		return true;
	}

	bool m = false;
	for (QHash<int, CollectionTreeItem*>::iterator i = p->childItems.begin();
			i != p->childItems.end(); i++)
	{
		m = (*i)->markItemMatchString(match) || m;
	}
	matched = m;
	return m;
}

/**
 * mark all children as matched/unmatched
 * @param match
 * @param item
 */
void CollectionTreeItem::markChildrenMatched(bool match)
{
	matched = match;
	for (QHash<int, CollectionTreeItem*>::iterator i = p->childItems.begin();
			i != p->childItems.end(); i++)
	{
		(*i)->markChildrenMatched(match);
	}
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
	artist->data["name"] = "{Various Artists}";
	artist->searchString = "{Various Artists}";
	artist->fetchData();
	artist->row = n;
	p->childItems[n] = artist;

	query.prepare("SELECT artist.id, artist.name, COUNT(album.id) FROM artist "
			"INNER JOIN album ON artist.id=album.artist_id GROUP BY artist.id ORDER BY artist.name");
	if (!query.exec()) {
		// TODO: send fail signal?
		return;
	}

	QChar prevLetter;
	QString artistName;

	CollectionTreeItem * letterArtist;
	while (query.next()) {
		n++;
		artist = new CollectionTreeItem(Artist, parent);
		artist->data["id"] = query.value(0);
		artist->data["name"] = query.value(1);
		artist->searchString = query.value(1).toString();
		artist->row = n;
		artist->fetchData();

		// take first non-space character from artist's name
		artistName = query.value(1).toString().trimmed();
		QChar curLetter = artistName.at(0).toUpper();
		if (curLetter != prevLetter) {
			prevLetter = curLetter;
			// append new "virtual" artist
			letterArtist = new CollectionTreeItem(Artist, parent);
			letterArtist->data["id"] = -2;
			letterArtist->data["name"] = curLetter;
			letterArtist->row = n;
			artist->row = n + 1;
			p->childItems[n] = letterArtist;
			n++;
		}

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

	query.prepare("SELECT album.id, album.name, image.path, album.year FROM album LEFT JOIN image ON "
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
		album->data["year"] = query.value(3);
		album->fetchData();
		int year = query.value(3).toInt();
		QString name = query.value(1).toString();
		if (year > 0) {
			album->searchString = QString("%1 %2").arg(year).arg(name);
		} else {
			album->searchString = name;
		}
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

	query.prepare("SELECT t.id, t.title, t.filename, t.track, t.length, g.name, d.path, ar.name FROM track t "
			"LEFT JOIN genre g ON g.id=t.genre_id "
			"LEFT JOIN artist ar ON ar.id=t.artist_id "
			"LEFT JOIN dir d ON d.id=t.dir_id WHERE album_id=:albumId");
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
		track->data["length"] = query.value(4);
		track->data["genre"] = query.value(5);
		track->data["path"] = query.value(6);
		track->data["artist"] = query.value(7);
		track->searchString = query.value(1).toString();
		//track->fetchData();
		//qDebug() << track->searchString;
		track->row = n;
		p->childItems[n] = track;
		n++;
	}

	p->rowsCount = n;
}
