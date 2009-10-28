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

#include "ui_mainwindow.h"

struct MainWindow::Private
{
	Ui::MainWindow ui;
	UpdateThread * ut;
	CollectionTreeWidget * ctw;
	QProgressBar * pb;
};

MainWindow::MainWindow() :
	QMainWindow()
{
	p = new Private();
	p->ui.setupUi(this);
	p->ut = new UpdateThread(this);
	p->ctw = new CollectionTreeWidget(this);
	p->ui.collectionDock->setWidget(p->ctw);

	// create progressbar
	p->pb = new QProgressBar(this);
	statusBar()->addPermanentWidget(p->pb);
	p->pb->hide();

	// create default playlist tab
	PlaylistWidget * defaultPlaylist = new PlaylistWidget(this);
	p->ui.playlistTabs->addTab(defaultPlaylist, tr("Default"));

	setWindowTitle(tr("Ororok — Music player and organizer"));
	createActions();
	connectSignals();
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

void MainWindow::createActions()
{
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
}
