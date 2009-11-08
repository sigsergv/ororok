/*
 * playlistmodel.h
 *
 *  Created on: Oct 28, 2009
 *      Author: Sergei Stolyarov
 */

#ifndef PLAYLISTMODEL_H_
#define PLAYLISTMODEL_H_

#include <QStandardItemModel>

class PlaylistModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	enum ItemRole {ItemTrackInfoRole=Qt::UserRole+1, ItemTrackStateRole};
	enum ActiveTrackState {TrackStateNotSelected, TrackStateStopped, TrackStatePlaying, TrackStatePaused};

	PlaylistModel(QObject * parent = 0);
	int rowCount(const QModelIndex & parent = QModelIndex()) const;
	int columnCount(const QModelIndex & parent = QModelIndex()) const;
	QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	Qt::ItemFlags flags(const QModelIndex & index) const;

	QStringList mimeTypes() const;
	bool dropMimeData(const QMimeData *data,
			Qt::DropAction action, int row, int column, const QModelIndex &parent);
	void selectActiveTrack(int n);
	bool selectActiveTrack(const QStringList & trackInfo);
	void markActiveTrackStarted();
	void markActiveTrackStopped();
	void markActiveTrackPaused();
	void markActiveTrackPlaying();
	ActiveTrackState activeTrackState();

protected:
	bool insertTrack(int pos, const QStringList & trackInfo);

private:
	struct Private;
	Private * p;
};

#endif /* PLAYLISTMODEL_H_ */
