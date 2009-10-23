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

class MainWindow::Private
{

};

MainWindow::MainWindow() :
	QMainWindow()
{
	p = new Private();
	ui.setupUi(this);
	ut = new UpdateThread(this);
	CollectionTreeWidget * cwt = new CollectionTreeWidget(this);
	ui.collectionDock->setWidget(cwt);
	createActions();
	connectSignals();
}

void MainWindow::rescanCollection()
{
	// launch UpdateThread
	ut->start();
}

void MainWindow::stopRescanCollection()
{
	ut->softStop();
}

void MainWindow::updateThreadStarted()
{
	ui.label->setText("started");
}

void MainWindow::updateThreadFinished()
{
	UpdateThread::UpdateThreadError ret = ut->errorCode();

	if (ret != UpdateThread::NoError) {
		// error occured
		ui.label->setText(UpdateThread::errorToText(ret));
	} else {
		ui.label->setText("finished");
	}
}

void MainWindow::updateThreadTerminated()
{
	ui.label->setText("terminated");
}

void MainWindow::scanProgress(int progress)
{
	if (progress < 0) {
		progress = 0;
	}
	if (progress > 100) {
		progress = 100;
	}

	ui.progressBar->setValue(progress);
}

void MainWindow::createActions()
{
}

void MainWindow::connectSignals()
{
	connect(ui.actionQuit, SIGNAL(triggered()), this, SLOT(close()));
	connect(ui.actionRescanCollection, SIGNAL(triggered()), this, SLOT(rescanCollection()));
	connect(ui.pb1, SIGNAL(pressed()), this, SLOT(rescanCollection()));
	connect(ui.pb2, SIGNAL(pressed()), this, SLOT(stopRescanCollection()));
	connect(ut, SIGNAL(started()), this, SLOT(updateThreadStarted()));
	connect(ut, SIGNAL(finished()), this, SLOT(updateThreadFinished()));
	connect(ut, SIGNAL(terminated()), this, SLOT(updateThreadTerminated()));
	connect(ut, SIGNAL(progressPercentChanged(int)), this, SLOT(scanProgress(int)));
}
