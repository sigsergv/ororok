/*
 * maintabswidget.h
 *
 *  Created on: Jul 4, 2010
 *      Author: Sergey Stolyarov
 */

#ifndef MAINTABSWIDGET_H_
#define MAINTABSWIDGET_H_

#include <QWidget>

class MainTabsWidget : public QWidget
{
	Q_OBJECT
public:
	MainTabsWidget(QWidget * parent = 0);
	~MainTabsWidget();
	virtual bool close();
	//virtual QString name() = 0;

signals:
	void pageTitleChanged(MainTabsWidget * page, const QString & name);
};

#endif /* MAINTABSWIDGET_H_ */
