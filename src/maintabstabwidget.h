/*
 * maintabstabwidget.h
 *
 *  Created on: Jul 4, 2010
 *      Author: Sergei Stolyarov
 */

#ifndef MAINTABSTABWIDGET_H_
#define MAINTABSTABWIDGET_H_

#include <QTabWidget>

struct MainTabsWidget;

class MainTabsTabWidget : public QTabWidget
{
	Q_OBJECT

public:
	MainTabsTabWidget(QWidget * parent = 0);
	~MainTabsTabWidget();
	int addTab(MainTabsWidget * page, const QString & label);

protected slots:
	void tabCloseRequested(int index);
	void tabRenamed(MainTabsWidget * page, const QString & name);
};

#endif /* MAINTABSTABWIDGET_H_ */
