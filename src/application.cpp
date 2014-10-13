/*
 * application.cpp
 *
 *  Created on: Nov 12, 2009
 *      Author: Sergey Stolyarov
 */

#include <QtGui>
#include <QtWidgets>

#include "application.h"

void Ororok::warning(const QString & title, const QString & message)
{
	QMessageBox::warning(QApplication::activeWindow(), title, message);
}

void Ororok::critical(const QString & title, const QString & message)
{
	QMessageBox::critical(QApplication::activeWindow(), title, message);
}
