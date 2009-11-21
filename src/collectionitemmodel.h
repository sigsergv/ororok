/*
 * collectionitemmodel.h
 *
 *  Created on: Oct 22, 2009
 *      Author: Sergei Stolyarov
 */

#ifndef COLLECTIONITEMMODEL_H_
#define COLLECTIONITEMMODEL_H_

#include <QAbstractItemModel>

struct ArtistRecord;

class CollectionItemModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	enum ItemRole {ItemTypeRole=Qt::UserRole+1, ItemAlbumCoverRole, ItemAlbumNameRole,
		ItemQuickSearchMatchedRole, ItemTrackInfoRole, ItemDbIdRole};

	CollectionItemModel(QObject * parent);

	QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex & index) const;
	int rowCount(const QModelIndex & parent = QModelIndex()) const;
	int columnCount(const QModelIndex & parent = QModelIndex()) const;
	QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
	void markItemsMatchString(const QString & match);
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QStringList mimeTypes() const;
	QMimeData * mimeData(const QModelIndexList &indexes) const;

	bool reloadData();

protected:
	void findTracksInIndexesTree(const QModelIndex & index, QModelIndexList & target) const;

private:
	struct Private;
	Private * p;
};
#endif /* COLLECTIONITEMMODEL_H_ */
