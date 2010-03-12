/*
 * edittreeview.cpp
 *
 *  Created on: Mar 13, 2010
 *      Author: Sergei Stolyarov
 */

#include "edittreeview.h"

EditTreeView::EditTreeView(QWidget * parent)
	: QTreeView(parent)
{

}

void EditTreeView::keyPressEvent (QKeyEvent * event)
{
	if (event->key() == Qt::Key_Delete) {
		// emit DELETE signal
		emit deleteKeyPressed();
	} else {
		QAbstractItemView::keyPressEvent(event);
	}
}
