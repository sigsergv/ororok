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

	if (role != Qt::DisplayRole) {
		return QVariant();
	}

	CollectionTreeItem * item = static_cast<CollectionTreeItem*>(index.internalPointer());

	if (item->type() == CollectionTreeItem::Artist) {
		return item->data["name"].toString();
	}

	if (item->type() == CollectionTreeItem::Album) {
		QString albumTitle;
		albumTitle = item->data["name"].toString();
		// item->data["cover_path"].toString()
		return albumTitle;
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

bool CollectionItemModel::reloadData()
{
	// destroy data set
	delete p->rootTreeItem;
	// create new
	p->rootTreeItem = new CollectionTreeItem(CollectionTreeItem::Root, 0);

	return true;
}
