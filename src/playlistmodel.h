/*
 * playlistmodel.h
 *
 *  Created on: Oct 28, 2009
 *      Author: Sergey Stolyarov
 */

#ifndef PLAYLISTMODEL_H_
#define PLAYLISTMODEL_H_

#include <QStandardItemModel>

class PlaylistModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	enum ItemRole {ItemTrackInfoRole=Qt::UserRole+1, ItemTrackStateRole};
	enum ActiveTrackState {TrackStateNotActive, TrackStateStopped, TrackStatePlaying, TrackStatePaused};

	PlaylistModel(const QString & playlistFile, QObject * parent = 0);
	QString playlistName();
	QString setPlaylistName(const QString & name);

	void movePlaylistFile(const QString & newPath);
	void removePlaylistFile();

	int rowCount(const QModelIndex & parent = QModelIndex()) const;
	int columnCount(const QModelIndex & parent = QModelIndex()) const;
	QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	Qt::ItemFlags flags(const QModelIndex & index) const;

	QStringList mimeTypes() const;

	QMimeData * mimeData(const QModelIndexList &indexes) const;
	bool dropMimeData(const QMimeData *data,
			Qt::DropAction action, int row, int column, const QModelIndex &parent);
	void selectActiveTrack(int n);
	bool selectActiveTrack(const QStringList & trackInfo);
	QStringList activeTrack();
	QStringList trackAfterActive();
	QStringList trackBeforeActive();
	void markActiveTrackStarted();
	void markActiveTrackStopped();
	void markActiveTrackPaused();
	void markActiveTrackPlaying();
	ActiveTrackState activeTrackState();
	bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex());
	bool insertRows (int row, int count, const QModelIndex & parent = QModelIndex());

protected:
//	bool insertTrack(int pos, const QStringList & trackInfo);
	void flushPlaylistFile();

private:
	struct Private;
	Private * p;
};

#endif /* PLAYLISTMODEL_H_ */
