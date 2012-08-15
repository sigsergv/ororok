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
	QList<CollectionTreeItem*> childItems;
	CollectionTreeItem * parentItem;
	int rowsCount;
};

bool CollectionTreeItemCmp(const CollectionTreeItem * a1, const CollectionTreeItem * a2)
{
	return QString::compare(a1->data["name"].toString(), a2->data["name"].toString(), Qt::CaseInsensitive) < 0;
}

CollectionTreeItem::CollectionTreeItem(CollectionTreeItemType t, CollectionTreeItem * parent)
	: row(0)
{
	p = new Private;
	p->type = t;
	p->parentItem = parent;
	p->rowsCount = -1;
	quickSearchMatched = true;
	datePeriodMatched = true;
	searchString = "";
}

CollectionTreeItem::~CollectionTreeItem()
{
	// TODO: delete all elements from p->childItems
	QList<CollectionTreeItem*> itemsToDelete;

	Q_FOREACH (CollectionTreeItem * item, p->childItems) {
		delete item;
	}
	p->childItems.clear();
	delete p;

	//qDebug() << ">> deleted tree item";
}

CollectionTreeItem::CollectionTreeItemType CollectionTreeItem::type()
{
	return p->type;
}

CollectionTreeItem * CollectionTreeItem::child(int n)
{
	return p->childItems.at(n);
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
bool CollectionTreeItem::markItemsMatchQuickSearchString(const QString & match)
{
	if (searchString.contains(match, Qt::CaseInsensitive)) {
		// mark this and all child nodes as quickSearchMatched
		//qDebug() << "quickSearchMatched" << searchString;
		markChildrenMatched(true);
		return true;
	}

	bool m = false;
	foreach (CollectionTreeItem * item, p->childItems) {
		m = item->markItemsMatchQuickSearchString(match) || m;
	}
	quickSearchMatched = m;
	return m;
}

bool CollectionTreeItem::markItemsMatchDatePeriod(int age)
{
    if (p->type == Root || p->type == Artist) {
		bool m = false;
		foreach (CollectionTreeItem * item, p->childItems) {
			m = item->markItemsMatchDatePeriod(age) || m;
		}
		datePeriodMatched = m;
	} else if (p->type == Album) {
		int modtime = data["modtime"].toInt();
		if (modtime <= 0) {
			datePeriodMatched = true;
		} else {
			//qDebug() << data["name"] << data["modtime"];
			datePeriodMatched = modtime > age;
		}
		// mark tracks if album is quickSearchMatched
		foreach (CollectionTreeItem * item, p->childItems) {
			item->datePeriodMatched = datePeriodMatched;
		}
	}
	return datePeriodMatched;

}

/**
 * mark all children as quickSearchMatched/unmatched
 * @param match
 * @param item
 */
void CollectionTreeItem::markChildrenMatched(bool match)
{
	quickSearchMatched = match;
	foreach (CollectionTreeItem * item, p->childItems) {
		item->markChildrenMatched(match);
	}
}

/**
 * restore letters items after putting quicksearch or date marks
 * @return
 */
void CollectionTreeItem::restoreLetterItems() {
	if (p->type != Root) {
		return;
	}

	QHash<QChar, bool> visible;
	QHash<QChar, CollectionTreeItem*> letters;

	// walk through the artists
	foreach (CollectionTreeItem * item, p->childItems) {
		//qDebug() << item->data["name"].toString();
		QChar letter = item->data["name"].toString().at(0).toUpper();

		if (item->data["id"].toInt() == -2) {
			// this is a letter "album"
			letters[letter] = item;
		} else {
			// remember it if it's not marked
			if (item->quickSearchMatched && item->datePeriodMatched) {
				visible[letter] = true;
			}
		}
	}

	Q_FOREACH(QChar letter, visible.keys())
	{
		if (!letters.contains(letter)) {
			continue;
		}
		letters[letter]->datePeriodMatched = true;
		letters[letter]->quickSearchMatched = true;
	}
}


void CollectionTreeItem::fetchArtists(CollectionTreeItem * parent)
{
	// load all artists from the database
	QSqlDatabase db = QSqlDatabase::database();
	QSqlQuery query(db);

	p->childItems.clear();

	// TODO: apply quickSearchFilter
	query.prepare("SELECT COUNT(*) FROM album WHERE artist_id=-1");
	if (!query.exec()) {
		// TODO: send fail signal?
		return;
	}
	query.next();

	//int n = 0;

	CollectionTreeItem * artist;

	// sort manually because sqlite sorting is horrible
	query.prepare("SELECT artist.id, artist.name, COUNT(album.id) FROM artist "
			"INNER JOIN album ON artist.id=album.artist_id GROUP BY artist.id");
	if (!query.exec()) {
		// TODO: send fail signal?
		return;
	}

	QString artistName;

	QList<CollectionTreeItem*> items;

	while (query.next()) {
		//n++;
		artist = new CollectionTreeItem(Artist, parent);
		artist->data["id"] = query.value(0);
		artist->data["name"] = query.value(1);
		artist->searchString = query.value(1).toString();
		artist->fetchData();

		/*
		// take first non-space character from artist's name
		*/

		items.append(artist);
	}

	// sort p->childItems
	qSort(items.begin(), items.end(), CollectionTreeItemCmp);

	// add letters
	QChar prevLetter;
	QList< QPair<int,QChar> > letterPos;
	int n = 0;
	foreach (const CollectionTreeItem * item, items) {
		QChar letter = item->data["name"].toString().at(0).toUpper();
		if (letter != prevLetter) {
			letterPos.append(QPair<int,QChar>(n, letter));
			prevLetter = letter;
		}
		n++;
	}

	// add "Various Artists"
	artist = new CollectionTreeItem(Artist, parent);
	artist->data["id"] = -1;
	artist->data["name"] = "{Various Artists}";
	artist->searchString = "{Various Artists}";
	artist->fetchData();
	items.insert(0, artist);

	QPair<int,QChar> e;
	int offset = 1;
	foreach (e, letterPos) {
		artist = new CollectionTreeItem(Artist, parent);
		artist->data["id"] = -2;
		artist->data["name"] = e.second;
		items.insert(e.first + offset, artist);
		offset++;
	}

	p->childItems = items;
	p->rowsCount = items.length();
}

/**
 * fetch all albums for given artist
 */
void CollectionTreeItem::fetchAlbums(CollectionTreeItem * parent)
{
	QSqlDatabase db = QSqlDatabase::database();
	QSqlQuery query(db);

	query.prepare("SELECT album.id, album.name, image.path, album.year FROM album LEFT JOIN image ON "
			"album.image_id=image.id WHERE artist_id=:artistId ORDER BY album.year");
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
		album->data["artist_id"] = parent->data["id"];
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
		p->childItems.append(album);
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

	query.prepare("SELECT t.id, t.title, t.filename, t.track, t.length, g.name, d.path, ar.name, d.modtime FROM track t "
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

	bool va_album = false;
	if (parent->type() == CollectionTreeItem::Album
			&& parent->data["artist_id"].toString() == QString("-1"))
	{
		va_album = true;
	}

    int maxModtime = 0;
	int modtime = 0;

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
		//track->data["album_artist_id"] = album_artist_id;

        modtime = query.value(8).toInt();
		if (modtime > maxModtime) {
			maxModtime = modtime;
		}
		track->data["modtime"] = modtime;

		track->searchString = query.value(1).toString();
		if (va_album) {
			track->searchString += QString(" ") + track->data["artist"].toString();
		}
		//track->fetchData();
		//qDebug() << track->searchString;
		p->childItems.append(track);
		n++;
	}

    parent->data["modtime"] = maxModtime;
	p->rowsCount = n;
}
