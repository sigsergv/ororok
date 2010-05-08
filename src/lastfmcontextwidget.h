/*
 * lastfmcontextwidget.h
 *
 *  Created on: May 8, 2010
 *      Author: Sergei Stolyarov
 */

#ifndef LASTFMCONTEXTWIDGET_H_
#define LASTFMCONTEXTWIDGET_H_

#include <QWidget>

struct QStringList;

class LastfmContextWidget : public QWidget
{
	Q_OBJECT
public:
	LastfmContextWidget(QWidget * parent = 0, Qt::WindowFlags f = 0);
	~LastfmContextWidget();

public slots:
	void playerTrackStarted(const QStringList & trackInfo);

protected slots:
	void trackGetInfoRequestFinished();

private:
	struct Private;
	Private * p;
};

#endif /* LASTFMCONTEXTWIDGET_H_ */
