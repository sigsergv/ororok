/*
 * playlistwidget.h
 *
 *  Created on: Oct 28, 2009
 *      Author: Sergei Stolyarov
 */

#ifndef PLAYLISTWIDGET_H_
#define PLAYLISTWIDGET_H_

#include <QWidget>

struct QModelIndex;
struct PlaylistModel;

class PlaylistWidget : public QWidget
{
	Q_OBJECT

public:
	enum PlaylistType {PlaylistTemporary, PlaylistPermanent};

	//PlaylistWidget(QWidget * parent = 0);
	PlaylistWidget(QString uid, PlaylistType t, const QString & name, QWidget * parent = 0);
	~PlaylistWidget();
	QString uid();

	/**
	 * fetch track that is playing now or selected for playing. If there
	 * is no such track return first track of playlist. If there are no tracks
	 * in the playlist return empty QStringList object.
	 * @return
	 */
	QStringList activeTrackInfo();
	PlaylistModel * model();

signals:
	void trackPlayRequsted(const QStringList & trackInfo);

protected slots:
	void playlistDoubleClicked(const QModelIndex & index);
	void resizeEvent(QResizeEvent * event);
	void deleteSelectedTracks();
	void clearPlaylist();
	void shufflePlaylist();
	void renamePlaylist();
	void tracksContextMenu(const QPoint & pos);

signals:
	void playlistTypeChanged(const QString & uid, int newType);
	void playlistNameChanged(const QString & uid, const QString & newName);

private:
	struct Private;
	Private * p;
};

#endif /* PLAYLISTWIDGET_H_ */
