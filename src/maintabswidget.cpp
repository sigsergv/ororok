/*
 * maintabswidget.cpp
 *
 *  Created on: Jul 4, 2010
 *      Author: Sergey Stolyarov
 */

#include <QtDebug>

#include "maintabswidget.h"

MainTabsWidget::MainTabsWidget(QWidget * parent)
	: QWidget(parent)
{

}

MainTabsWidget::~MainTabsWidget()
{

}

bool MainTabsWidget::close()
{
	return true;
}
