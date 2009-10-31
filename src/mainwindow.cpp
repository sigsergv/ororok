/*
 * mainwindow.cpp
 *
 *  Created on: Oct 19, 2009
 *      Author: Sergei Stolyarov
 */
#include <QtCore>
#include <QtGui>

#include "mainwindow.h"
#include "updatethread.h"
#include "collectiontreewidget.h"
#include "collectionitemmodel.h"
#include "playlistwidget.h"
#include "playlistmanager.h"

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
	QSlider * trackTimeSlider;
	QLabel * statusMessageLabel;
	QLabel * playlistInfoLabel;

	// playcontrol
	QAction * actionPlaybackPrev;
	QAction * actionPlaybackPlayPause;
	QAction * actionPlaybackStop;
	QAction * actionPlaybackNext;

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

	p->trackTimeSlider = new QSlider(Qt::Horizontal, this);
	p->trackTimeSlider->setFocusPolicy(Qt::NoFocus);
	p->trackTimeSlider->setMaximumWidth(200);
	p->trackTimeSlider->setMinimumWidth(200);
	statusBar()->addWidget(p->trackTimeSlider);

	createActions();

	// populate playControlsToolbar

	p->ui.playControlsToolbar->addAction(p->actionPlaybackPrev);
	p->ui.playControlsToolbar->addAction(p->actionPlaybackPlayPause);
	p->ui.playControlsToolbar->addAction(p->actionPlaybackStop);
	p->ui.playControlsToolbar->addAction(p->actionPlaybackNext);

	connectSignals();
	setWindowTitle(tr("Ororok — Music player and organizer"));
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
	qDebug() << "play/pause";
	// fetch active track info from current playlist
	//QStringList trackInfo = p->tmpPL->activeTrackInfo();

	//qDebug() << trackInfo;
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
