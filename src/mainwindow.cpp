/*
 * mainwindow.cpp
 *
 *  Created on: Oct 19, 2009
 *      Author: Sergei Stolyarov
 */
#include <QtCore>
#include <QtGui>
#include <Phonon/SeekSlider>
#include <Phonon/VolumeSlider>
#include <Phonon/MediaObject>

#include "mainwindow.h"
#include "updatethread.h"
#include "collectiontreewidget.h"
#include "collectionitemmodel.h"
#include "playlistwidget.h"
#include "playlistmanager.h"
#include "player.h"

#include "ui_mainwindow.h"

const QString STATUSBAR_QSS("QStatusBar{ "
		"padding: 0;"
		"}"
		"QStatusBar::item { border: 1px solid %1; padding: 0;}"
		);

struct MainWindow::Private
{
	Ui::MainWindow ui;
	UpdateThread * ut;
	CollectionTreeWidget * ctw;
	QProgressBar * pb;
	//QFrame * pbSection;

	// statusbar widgets
	QLabel * statusMessageLabel;
	QLabel * playlistInfoLabel;

	// playcontrol
	QAction * actionPlaybackPrev;
	QAction * actionPlaybackPlayPause;
	QAction * actionPlaybackStop;
	QAction * actionPlaybackNext;

	// track progress
	Phonon::SeekSlider * trackProgressSlider;
	QLabel * trackTimeLabel;

	// volume
	Phonon::VolumeSlider * volumeSlider;

	PlaylistWidget * tmpPL;

};

MainWindow::MainWindow() :
	QMainWindow()
{
	QPalette palette = QApplication::palette();
	QColor borderColor = palette.color(QPalette::Mid);
	QString qss = STATUSBAR_QSS.arg(borderColor.name());

	p = new Private();
	p->ui.setupUi(this);
	p->ut = new UpdateThread(this);
	p->ctw = new CollectionTreeWidget(this);
	p->ui.collectionDock->setWidget(p->ctw);

	// initialize PlaylistManager
	PlaylistManager * pm = PlaylistManager::instance();

	this->setCentralWidget(pm->playlistsTabWidget());

	//statusBar()->layout()->setContentsMargins(0, 0, 10, 0);
	statusBar()->setSizeGripEnabled(false);
	statusBar()->setStyleSheet(qss);
	//statusBar()->addWidget(new QLabel("Test messsage"));

	// create statusbar sections
	p->pb = new QProgressBar(this);
	p->pb->hide();
	statusBar()->addWidget(p->pb, 1);

	p->statusMessageLabel = new QLabel(this);
	statusBar()->addWidget(p->statusMessageLabel, 1);

	p->playlistInfoLabel = new QLabel(this);
	statusBar()->addWidget(p->playlistInfoLabel);

	statusBar()->addWidget(new QLabel(this)); // placeholder

	Player * player = Player::instance();

	p->trackProgressSlider = player->seekSlider();
	p->trackProgressSlider->setFocusPolicy(Qt::NoFocus);
	//p->trackProgressSlider->setMaximumWidth(200);
	//p->trackProgressSlider->setMinimumWidth(200);
	p->trackTimeLabel = new QLabel(this);
	p->volumeSlider = player->volumeSlider();

	createActions();

	// populate playControlsToolbar
	p->ui.playControlsToolbar->addAction(p->actionPlaybackPrev);
	p->ui.playControlsToolbar->addAction(p->actionPlaybackPlayPause);
	p->ui.playControlsToolbar->addAction(p->actionPlaybackStop);
	p->ui.playControlsToolbar->addAction(p->actionPlaybackNext);

	// populate trackProgressToolbar
	p->ui.trackProgressToolbar->layout()->setSpacing(3);
	p->ui.trackProgressToolbar->addWidget(p->trackProgressSlider);
	p->ui.trackProgressToolbar->addWidget(p->trackTimeLabel);

	// populate volumeToolbar
	p->ui.volumeToolbar->addWidget(p->volumeSlider);

	connectSignals();
	setWindowTitle(tr("Ororok — Music player and organizer"));
	trackTimeChange(0, 0);

	// Debug code. test playlists
	pm->playlist("new-playlist");
}

void MainWindow::rescanCollection()
{
	// launch UpdateThread
	p->pb->show();
	p->ut->start();
}

void MainWindow::stopRescanCollection()
{
	p->ut->softStop();
}

void MainWindow::updateThreadStarted()
{
	//p->ui.label->setText("started");
}

void MainWindow::updateThreadFinished()
{
	UpdateThread::UpdateThreadError ret = p->ut->errorCode();

	if (ret != UpdateThread::NoError) {
		// error occured
		//p->ui.label->setText(UpdateThread::errorToText(ret));
	} else {
		//p->ui.label->setText("finished");
	}
	p->pb->hide();
}

void MainWindow::updateThreadTerminated()
{
	//p->ui.label->setText("terminated");
	p->pb->hide();
}

void MainWindow::refreshCollectionTree()
{
	p->ctw->reloadTree();
}

void MainWindow::scanProgress(int progress)
{
	if (progress < 0) {
		progress = 0;
	}
	if (progress > 100) {
		progress = 100;
	}

	p->pb->setValue(progress);
}

void MainWindow::playbackPlayPause()
{
	Player * player = Player::instance();
	PlaylistManager * pm = PlaylistManager::instance();

	// if player is playing something right now then pause play
	if (Phonon::PlayingState == player->state()) {
		// now mark track as paused in the corresponding playlist
		pm->requestTrackPause();
		return;
	}

	// if player is in paused state the resume playing
	if (Phonon::PausedState == player->state()) {
		// now mark active paused track as playing
		pm->requestTrackResume();
		return;
	}

	// if there is no playing/paused tracks fetch active track
	// from the currently displayed playlist and play it


	qDebug() << "play/pause";
	// fetch active track info from current playlist
	//QStringList trackInfo = p->tmpPL->activeTrackInfo();

	//qDebug() << trackInfo;
}

void MainWindow::playbackPrev()
{

}

void MainWindow::playbackStop()
{

}


void MainWindow::playbackNext()
{

}

void MainWindow::trackTimeChange(qint64 time, qint64 totalTime)
{
	Q_UNUSED(totalTime);
	// set track time label
	QTime displayTime(0, (time / 60000) % 60, (time / 1000) % 60);
	p->trackTimeLabel->setText(displayTime.toString("mm:ss"));
}

void MainWindow::createActions()
{
	p->actionPlaybackPrev = new QAction("Play previous track", this);
	p->actionPlaybackPrev->setIcon(QIcon(":/play-control-prev.png"));

	p->actionPlaybackPlayPause = new QAction("Play/Pause", this);
	p->actionPlaybackPlayPause->setIcon(QIcon(":play-control-playpause.png"));

	p->actionPlaybackStop = new QAction("Stop", this);
	p->actionPlaybackStop->setIcon(QIcon(":play-control-stop.png"));

	p->actionPlaybackNext = new QAction("Play next track", this);
	p->actionPlaybackNext->setIcon(QIcon(":play-control-next.png"));
}

void MainWindow::connectSignals()
{
	connect(p->ui.actionQuit, SIGNAL(triggered()), this, SLOT(close()));
	connect(p->ui.actionRescanCollection, SIGNAL(triggered()), this, SLOT(rescanCollection()));
	connect(p->ui.actionReloadCollectionTree, SIGNAL(triggered()), this, SLOT(refreshCollectionTree()));
	connect(p->ut, SIGNAL(started()), this, SLOT(updateThreadStarted()));
	connect(p->ut, SIGNAL(finished()), this, SLOT(updateThreadFinished()));
	connect(p->ut, SIGNAL(terminated()), this, SLOT(updateThreadTerminated()));
	connect(p->ut, SIGNAL(progressPercentChanged(int)), this, SLOT(scanProgress(int)));

	connect(p->actionPlaybackPlayPause, SIGNAL(triggered()), this, SLOT(playbackPlayPause()));
	connect(p->actionPlaybackPrev, SIGNAL(triggered()), this, SLOT(playbackPrev()));
	connect(p->actionPlaybackStop, SIGNAL(triggered()), this, SLOT(playbackStop()));
	connect(p->actionPlaybackNext, SIGNAL(triggered()), this, SLOT(playbackNext()));

	Player * player = Player::instance();

	connect(player, SIGNAL(trackTimeChanged(qint64, qint64)), this, SLOT(trackTimeChange(qint64, qint64)));
}

QFrame * MainWindow::createStatusBarSection(QWidget * widget)
{
	QFrame * f = new QFrame(this);
	f->setFrameShape(QFrame::Box);
	QHBoxLayout * layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(-1);
	f->setLayout(layout);
	layout->addWidget(widget);

	return f;
}
