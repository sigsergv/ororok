/*
 * mainwindow.h
 *
 *  Created on: Oct 19, 2009
 *      Author: Sergei Stolyarov
 */

#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <QMainWindow>

struct UpdateThread;
struct QFrame;

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
	void newPlaylist();
	void stopRescanCollection();
	void updateThreadStarted();
	void updateThreadFinished();
	void updateThreadTerminated();
	void refreshCollectionTree();
	void scanProgress(int);

	void playbackPlayPause();
	void playbackPrev();
	void playbackStop();
	void playbackNext();
	void trackTimeChange(qint64 time, qint64 totalTime);
	void playerRequestedNextTrack();
	void editSettings();

protected:
	QFrame * createStatusBarSection(QWidget * widget);
	void moveEvent(QMoveEvent * event);
	void resizeEvent(QResizeEvent * event);
	void closeEvent(QCloseEvent *event);

private:
	struct Private;
	Private * p;
};

#endif /* MAINWINDOW_H_ */
