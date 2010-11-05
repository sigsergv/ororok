/*
 * playlistwidget.cpp
 *
 *  Created on: Oct 28, 2009
 *      Author: Sergei Stolyarov
 */

#include <QtGui>

#include "playlistwidget.h"
#include "playlistmodel.h"
#include "playlistitemdelegate.h"
#include "edittreeview.h"
#include "settings.h"
#include "desktopaccess.h"
#include "mainwindow.h"
#include "renameplaylistdialog.h"

struct PlaylistWidget::Private
{
	QLineEdit * filter;
	QString uid;
	Ororok::PlaylistType type;

	QTreeView * tracksList;
	PlaylistModel * model;
	QSortFilterProxyModel * proxy;
	QMenu * tracksContextMenu;
	bool initialized;

	// actions
	QAction * deleteSelectedTracks;

	// methods
	QPushButton * createToolbarButton(QWidget * parent, const QString & icon, const QString & tooltip)
	{
		QPushButton * b = new QPushButton(QIcon(icon), QString(), parent);
		b->setFlat(true);
		b->setFocusPolicy(Qt::NoFocus);
		b->setIconSize(QSize(16,16));
		b->setMaximumSize(22, 100);
		b->setToolTip(tooltip);
		return b;
	}
};

PlaylistWidget::PlaylistWidget(QString uid, Ororok::PlaylistType t, QWidget * parent)
	: MainTabsWidget(parent)
{
	p = new Private;

	p->initialized = false;
	p->uid = uid;
	p->type = t;
	QLayout * layout;
	//
	//layout->addWidget(p->quickSearchFilter);

	// create "toolbar" widget
	QPushButton * b;

	layout = new QHBoxLayout(this);
	b = p->createToolbarButton(this, ":edit-clear-list-16x16.png", tr("Remove all tracks from playlist, SHIFT+CLICK — delete all except selected"));
	connect(b, SIGNAL(clicked()), this, SLOT(clearPlaylist()));
	layout->addWidget(b);

	b = p->createToolbarButton(this, ":edit-rename-16x16.png", tr("Rename playlist"));
	connect(b, SIGNAL(clicked()), this, SLOT(renamePlaylist()));
	layout->addWidget(b);

	b = p->createToolbarButton(this, ":shuffle-16x16.png", tr("Shuffle playlist items"));
	connect(b, SIGNAL(clicked()), this, SLOT(shufflePlaylist()));
	layout->addWidget(b);

	p->filter = new QLineEdit(this);
	layout->addWidget(p->filter);
	QWidget * tb = new QWidget(this);
	tb->setLayout(layout);

	layout = new QVBoxLayout(this);
	layout->addWidget(tb);
	p->tracksList = new EditTreeView(this);
	p->tracksList->setContextMenuPolicy(Qt::CustomContextMenu);
	p->tracksList->setAlternatingRowColors(true);
	p->tracksList->setRootIsDecorated(false);
	//p->tracksList->setDragDropMode(QAbstractItemView::DropOnly);
	p->tracksList->setSelectionBehavior(QAbstractItemView::SelectRows);
	p->tracksList->setSelectionMode(QAbstractItemView::ExtendedSelection);
	p->tracksList->setDragEnabled(true);
	p->tracksList->setDragDropMode(QAbstractItemView::DragDrop);
	p->tracksList->setAcceptDrops(true);
	layout->addWidget(p->tracksList);

	// construct playlist name
	QString storePath;
	switch (t) {
	case Ororok::PlaylistTemporary:
		storePath = Ororok::tmpPlaylistsStorePath();
		break;

	case Ororok::PlaylistPermanent:
		storePath = Ororok::playlistsStorePath();
		break;
	}

	p->model = new PlaylistModel(storePath + "/" + uid, this);

	p->proxy = new QSortFilterProxyModel(this);
	p->proxy->setSourceModel(p->model);
	p->tracksList->setModel(p->proxy);

	// create menus and actions

	p->tracksContextMenu = new QMenu(this);
	p->deleteSelectedTracks = p->tracksContextMenu->addAction(tr("Delete selected tracks"));

	// resize columns
	// get total width
	/*
	for (int i=0; i<sections; i++) {
		QString section
	}
	*/

	//p->tracksList->setColumnWidth(VisColumnLength, 20);
	//qDebug() << p->tracksList->columnWidth(2);
	//p->tracksList->show();

	setLayout(layout);
	p->tracksList->header()->setStretchLastSection(false);
	p->tracksList->header()->resizeSection(0, 200); // set width of "Length" column
	p->tracksList->header()->resizeSection(1, 200);
	p->tracksList->header()->resizeSection(2, 200);

	PlaylistItemDelegate * delegate = new PlaylistItemDelegate(this);
	p->tracksList->setItemDelegate(delegate);

	connect(p->tracksList, SIGNAL(doubleClicked(const QModelIndex &)),
			this, SLOT(playlistDoubleClicked(const QModelIndex &)));
	connect(p->tracksList, SIGNAL(deleteKeyPressed()),
			this, SLOT(deleteSelectedTracks()));
	connect(p->deleteSelectedTracks, SIGNAL(triggered()),
			this, SLOT(deleteSelectedTracks()));
	connect(p->tracksList, SIGNAL(customContextMenuRequested(const QPoint &)),
			this, SLOT(tracksContextMenu(const QPoint &)));
}


PlaylistWidget::~PlaylistWidget()
{
	qDebug() << "destroy playlist widget";
	delete p;
}

QString PlaylistWidget::uid()
{
	return p->uid;
}

QString PlaylistWidget::name()
{
	return p->model->playlistName();
}

PlaylistModel * PlaylistWidget::model()
{
	return p->model;
}

/**
 * tab is about to close so decide what to do: ask user etc
 */
bool PlaylistWidget::close()
{
	QMessageBox boxConfirm;
	bool doClose = false;
	if (p->type == Ororok::PlaylistTemporary) {
		boxConfirm.setText(tr("You are trying to close temporary playlist. Please choose one of the actions"));
		//boxConfirm.setInformativeText(tr("asdasd"));
		boxConfirm.addButton(tr("Do not remember and close"), QMessageBox::AcceptRole);
		boxConfirm.addButton(tr("Remember and close"), QMessageBox::YesRole);
		boxConfirm.addButton(tr("Don't close"), QMessageBox::RejectRole);
		int btn = boxConfirm.exec();
		if (QMessageBox::AcceptRole == btn) {
			// clicked [Do not remember and close] button
			// so we have to destroy playlist file
			emit deletePlaylist(p->uid, true);
			doClose = true;
		} else if (QMessageBox::YesRole == btn) {
			// clicked [Remember and close] button
			// change playlist type, remove it from the settings and close
			emit playlistTypeChanged(p->uid, Ororok::PlaylistPermanent);
			emit deletePlaylist(p->uid, false);
		} else {
			// clicked [Don't close] button
			doClose = false;
		}
	} else if (p->type == Ororok::PlaylistPermanent) {
		QSettings * settings = Ororok::settings();
		settings->beginGroup("MainWindow");
		bool ask = settings->value("askPermanentPlaylistCloseConfirmation", true).toBool();
		settings->endGroup();

		if (ask) {
			boxConfirm.setText(tr("You are going to close permanent playlist. I.e. it's saved and could be restored later via menu."));
			boxConfirm.addButton(tr("Close"), QMessageBox::AcceptRole);
			boxConfirm.addButton(tr("Close and never ask later"), QMessageBox::YesRole);
			boxConfirm.addButton(tr("Don't close"), QMessageBox::RejectRole);
			int btn = boxConfirm.exec();
			if (QMessageBox::AcceptRole == btn) {
				// clicked [Close] button
				emit deletePlaylist(p->uid, false);
				doClose = true;
			} else if (QMessageBox::YesRole == btn) {
				// clicked [Close and never ask later] button
				settings->beginGroup("MainWindow");
				settings->setValue("askPermanentPlaylistCloseConfirmation", false);
				settings->endGroup();
				emit deletePlaylist(p->uid, false);
				doClose = true;
			} else {
				doClose = false;
			}
		} else {
			doClose = true;
		}
	}

	return doClose;
}

QStringList PlaylistWidget::activeTrackInfo()
{
	// take selected track
	QStringList trackInfo;
	Q_FOREACH (const QModelIndex & index, p->tracksList->selectionModel()->selectedRows()) {
		trackInfo = index.data(PlaylistModel::ItemTrackInfoRole).toStringList();
		break;
	}

	return trackInfo;
}

void PlaylistWidget::playlistDoubleClicked(const QModelIndex & index)
{
	QStringList trackInfo = index.data(PlaylistModel::ItemTrackInfoRole).toStringList();
	//markActiveTrackStarted(trackInfo);
	emit trackPlayRequsted(trackInfo);
}

void PlaylistWidget::resizeEvent(QResizeEvent * event)
{
	QWidget::resizeEvent(event);
	// TODO: resize columns to fit size
}

void PlaylistWidget::deleteSelectedTracks()
{
	// find selected tracks in the model and delete them!
	// take selected track
	//QStringList trackInfo;
	QList<int> rows;
	Q_FOREACH (const QModelIndex & index, p->tracksList->selectionModel()->selectedRows()) {
		rows << index.row();
	}
	// sort "rows" and delete items from bottom to top
	qSort(rows);
	QListIterator<int> i(rows);
	i.toBack();
	while (i.hasPrevious()) {
		p->model->removeRow(i.previous());
	}
}

void PlaylistWidget::clearPlaylist()
{
	if (Ororok::isShiftKeyPressed()) {
		// crop playlist
		qDebug() << "crop playlist";
	} else {
		// delete all items
		p->model->removeRows(0, p->model->rowCount());
	}
}

void PlaylistWidget::shufflePlaylist()
{

}

void PlaylistWidget::renamePlaylist()
{
	// ask user about new playlist name
	// also ask him to remember playlist
	RenamePlaylistDialog r(MainWindow::inst());
	bool remembered = p->type == Ororok::PlaylistPermanent;
	r.setPlaylistName(name());
	r.setPlaylistRemembered(remembered);
	if (r.exec()) {
		QString newName = r.playlistName();
		QString oldName = name();

		if (remembered == r.isPlaylistRemembered() && oldName == newName) {
			// no changes
			return;
		}
		if (remembered != r.isPlaylistRemembered()) {
			// playlist type changed
			p->type = r.isPlaylistRemembered() ? Ororok::PlaylistPermanent : Ororok::PlaylistTemporary;
			// we have to move our playlist file to another location
			QString storePath;
			switch (p->type) {
			case Ororok::PlaylistTemporary:
				storePath = Ororok::tmpPlaylistsStorePath();
				break;

			case Ororok::PlaylistPermanent:
				storePath = Ororok::playlistsStorePath();
				break;
			}

			p->model->movePlaylistFile(storePath + "/" + p->uid);

			emit playlistTypeChanged(p->uid, p->type);
		}
		if (oldName != newName) {
			// playlist name changed
			p->model->setPlaylistName(newName);
			emit playlistNameChanged(p->uid, newName);
			emit pageTitleChanged(this, newName);
		}
	}

}

void PlaylistWidget::tracksContextMenu(const QPoint & pos)
{
	// if there is no tracks then disable some actions
	bool tracksListEmpty = p->model->rowCount() == 0;

	p->deleteSelectedTracks->setEnabled(!tracksListEmpty);
	p->tracksContextMenu->popup(p->tracksList->viewport()->mapToGlobal(pos));
}
