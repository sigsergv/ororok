/*
 * mainwindow.h
 *
 *  Created on: Oct 19, 2009
 *      Author: Sergei Stolyarov
 */

#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <QMainWindow>

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
	void refreshCollectionTree();
	void scanProgress(int);

private:
	struct Private;
	Private * p;
};

#endif /* MAINWINDOW_H_ */
