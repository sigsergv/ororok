/*
 * filterlineedit.cpp
 *
 *  Created on: Apr 18, 2010
 *      Author: Sergei Stolyarov
 */

#include <QKeyEvent>
#include <QtDebug>

#include "filterlineedit.h"

FilterLineEdit::FilterLineEdit(QWidget * parent)
	: QLineEdit(parent)
{
}

void FilterLineEdit::keyPressEvent(QKeyEvent * event)
{
	if (event->key() == Qt::Key_Escape) {
		emit escapeKeyPressed();
	}
	QLineEdit::keyPressEvent(event);
}
