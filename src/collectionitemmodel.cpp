/*
 * collectionitemmodel.cpp
 *
 *  Created on: Oct 22, 2009
 *      Author: Sergei Stolyarov
 */

#include <QStringList>
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

	if (ItemDbIdRole == role) {
		return item->data["id"];
	}

	if (ItemAlbumCoverRole == role && CollectionTreeItem::Album == itemType) {
		return item->data["cover_path"];
	}

	if (ItemAlbumNameRole == role && CollectionTreeItem::Album == itemType) {
		return item->data["name"];
	}

	if (ItemArtistLetterRole == role && CollectionTreeItem::Artist == itemType) {
		if (item->data["id"].toInt() == -2) {
			return true;
		} else {
			return false;
		}
	}

	if (ItemQuickSearchMatchedRole == role) {
		// return QString object that contain
		return item->quickSearchMatched;
	}

    if (ItemDatePeriodMatchedRole == role) {
		return item->datePeriodMatched;
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

	if (role == Qt::ToolTipRole) {
		// create tooltip text
		QString tip;

		QSqlDatabase db = QSqlDatabase::database();
		QSqlQuery query(db);

		switch (item->type()) {
		case CollectionTreeItem::Album: {
			// display in the tooltip:
			// album name, year, number of tracks, [cover], [tracks]

			query.prepare("SELECT track.track, track.title, track.length, genre.name, dir.modtime FROM track "
					//"LEFT JOIN artist ON artist.id=track.artist_id "
					"LEFT JOIN genre ON track.genre_id=genre.id "
					"LEFT JOIN dir ON track.dir_id=dir.id "
					"WHERE album_id=:albumId");
			query.bindValue(":albumId", item->data.value("id", "-1"));
			if (!query.exec()) {
				return "";
			}

			int tracks_cnt = 0;
			int total_len = 0;
			QSet<QString> genres;

			int modtime = 0;
			while (query.next()) {
				//query.value(1)
				tracks_cnt++;
				total_len += query.value(2).toInt();
				genres.insert(query.value(3).toString());
				int lt = query.value(4).toInt();
				if (lt > modtime) {
					modtime = lt;
				}
			}
			QDateTime dt = QDateTime::fromTime_t(modtime);
			QString modtime_str = dt.toString("yyyy-MM-dd h:mm");

			QString total_len_str = QString("%1").arg(total_len % 60, 2, 10, QChar('0'));
			total_len /= 60;
			total_len_str = QString::number(total_len) + QString(":") + total_len_str;

			tip = "<table border=\"0\"><tr>";

			if (!item->data.value("cover_path", "").toString().isEmpty()) {
				tip += QString("<td><img src=\"%1\" width=\"150\"></td>")
						.arg(item->data.value("cover_path", "").toString());
			}

			tip += "<td valign=\"top\">";

			QStringList genres_list = genres.toList();
			tip += tr("<!--ctree: album tooltip--><div><em><strong>%1</strong></em></div>"
					"<div>&nbsp;<!--empty line--><div>"
					"<div>Total tracks/length: <strong><em>%2 / %3</em></strong></div>"
					"<div>Genres: <strong><em>%4</em></strong></div>"
					"<div>Modified: <strong><em>%5</em></strong></div></div>")
				.arg(item->data.value("name", tr("Unknown album name")).toString())
				.arg(tracks_cnt)
				.arg(total_len_str)
				.arg(genres_list.join(", "))
				.arg(modtime_str);

			tip += tr("</td></tr></table><!--ctree: end of album tooltip-->");

			}
			break;

		case CollectionTreeItem::Track: {
			// display track name, length, genre, performer
			query.prepare("SELECT t.id, d.modtime FROM track t "
					"LEFT JOIN dir d ON d.id=t.dir_id "
					"WHERE t.id=:trackId");
			query.bindValue(":trackId", item->data.value("id", "-1"));

			if (!query.exec()) {
				return "";
			}

			int modtime = 0;
			QString modtime_str;

			if (query.next()) {
				// query.value(1)
				modtime = query.value(1).toInt();
				QDateTime dt = QDateTime::fromTime_t(modtime);
				modtime_str = dt.toString("yyyy-MM-dd h:mm");
			}

			int track_len = item->data.value("length", "0").toInt();

			QString track_len_str = QString("%1").arg(track_len % 60, 2, 10, QChar('0'));
			track_len /= 60;
			track_len_str = QString::number(track_len) + QString(":") + track_len_str;

			tip = tr("<!--ctree: artist tooltip--><div><strong><em>%1</em></strong></div>"
					"<div>by <strong><em>%2</em></strong></div>"
					"<div>Length: <strong><em>%3</em></strong></div>"
					"<div>Genre: <strong><em>%4</em></strong></div>"
					"<div>Added: <strong><em>%5</em></strong></div></div>")
					.arg(item->data.value("title", tr("Empty title")).toString())
					.arg(item->data.value("artist", tr("Unknown artist")).toString())
					.arg(track_len_str)
					.arg(item->data.value("genre", tr("Unknown genre")).toString())
					.arg(modtime_str);
			}
			break;

		case CollectionTreeItem::Artist:
			break;

		case CollectionTreeItem::Root:
			break;
		}

		// strip html comments because they are not parsed by richtext engine
		return tip.replace(QRegExp("<!--[^>]+-->"), "");
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
		CollectionTreeItem * parent = item->parent();
		if (CollectionTreeItem::Album == parent->type()
				&& parent->data["artist_id"].toString() == QString("-1"))
		{
			trackTitle += QString("%1 - ").arg( item->data["artist"].toString() );
		}
		trackTitle += item->data["title"].toString();
		return trackTitle;
	}

	return QString("type: %1").arg(item->type());
}

void CollectionItemModel::markItemsMatchQuickSearchString(const QString & match)
{
	p->rootTreeItem->markItemsMatchQuickSearchString(match);
	p->rootTreeItem->restoreLetterItems();
}

void CollectionItemModel::markItemsMatchDatePeriod(int days)
{
	int age = 0;

	if (days >= 0) {
		// calculate min age
		QDateTime d = QDateTime::currentDateTime();
		d.setTime(QTime(0, 0, 0));
		// subtract specified amount of days
		age = d.toTime_t() - days * 86400;
	}

	qDebug() << age << days;
	p->rootTreeItem->markItemsMatchDatePeriod(age);
	p->rootTreeItem->restoreLetterItems();
}

Qt::ItemFlags CollectionItemModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);

	if (index.isValid()) {
		if (index.data(ItemArtistLetterRole).toBool()) {
			return defaultFlags;
		}
		return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
	} else {
		return Qt::ItemIsDropEnabled | defaultFlags;
	}
}

QStringList CollectionItemModel::mimeTypes() const
{
	QStringList mt;

	mt << Ororok::TRACKS_COLLECTION_IDS_MIME;

	return mt;
}

QMimeData * CollectionItemModel::mimeData(const QModelIndexList &indexes) const
{
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

	QStringList ids;

	Q_FOREACH (const QModelIndex & index, trackIndexes) {
		// [trackPath, trackNo, title, album, year, album, artist, genre]
		int id = index.data(CollectionItemModel::ItemDbIdRole).toInt();
		stream << id;
	}
	QMimeData *md = new QMimeData();
	md->setData(Ororok::TRACKS_COLLECTION_IDS_MIME, encodedData);

	qDebug() << "total indexes" << trackIndexes.count();
	return md;
}

void CollectionItemModel::findTracksInIndexesTree(const QModelIndex & index, QModelIndexList & target) const
{
	if (index.data(CollectionItemModel::ItemTypeRole) == CollectionTreeItem::Track) {
		target << index;
		return;
	}

	CollectionTreeItem * ix;
	ix = static_cast<CollectionTreeItem*>(index.internalPointer());

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
