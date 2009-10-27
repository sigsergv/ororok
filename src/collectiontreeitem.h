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

	bool markItemMatchString(const QString & match);

	void fetchData();

	int row;
	QMap<QString, QVariant> data;
	QString searchString;
	bool matched;

protected:
	void markChildrenMatched(bool match);

private:
	struct Private;
	Private * p;

	void fetchArtists(CollectionTreeItem *);
	void fetchAlbums(CollectionTreeItem *);
	void fetchTracks(CollectionTreeItem *);
};

#endif /* COLLECTIONTREEITEM_H_ */
