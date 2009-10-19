/*
 * mainwindow.h
 *
 *  Created on: Oct 19, 2009
 *      Author: Sergei Stolyarov
 */

#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <QMainWindow>
#include "ui_mainwindow.h"

class UpdateThread;

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	MainWindow();

protected:
	void createActions();
	void connectSignals();

protected slots:
	void rescanCollection();
	void stopRescanCollection();
	void updateThreadStarted();
	void updateThreadFinished();
	void updateThreadTerminated();

private:
	class Private;
	Private * p;
	Ui::MainWindow ui;
	UpdateThread * ut;
};

#endif /* MAINWINDOW_H_ */
