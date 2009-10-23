/*
 * CollectionTreeItem.h
 *
 *  Created on: Oct 24, 2009
 *      Author: Sergei Stolyarov
 */

#ifndef COLLECTIONTREEITEM_H_
#define COLLECTIONTREEITEM_H_

#include <QMap>

class CollectionTreeItem {
public:
	enum CollectionTreeItemType { Root, Album, Artist, Track };
	CollectionTreeItem(CollectionTreeItemType t, CollectionTreeItem * parent);
	~CollectionTreeItem();
	CollectionTreeItemType type();
	CollectionTreeItem * child(int n);
	CollectionTreeItem * parent();
	int childrenCount();

	int row;
	QMap<QString, QVariant> data;

private:
	struct Private;
	Private * p;

	void fetchArtists(CollectionTreeItem *);
	void fetchAlbums(CollectionTreeItem *);
	void fetchTracks(CollectionTreeItem *);
};

#endif /* COLLECTIONTREEITEM_H_ */
