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

class MainWindow::Private
{

};

MainWindow::MainWindow() :
	QMainWindow()
{
	p = new Private();
	ui.setupUi(this);
	ut = new UpdateThread(this);

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
	ui.label->setText("finished");
}

void MainWindow::updateThreadTerminated()
{
	ui.label->setText("terminated");
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
}
