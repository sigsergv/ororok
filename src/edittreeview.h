/*
 * edittreeview.h
 *
 *  Created on: Mar 13, 2010
 *      Author: Sergei Stolyarov
 */

#ifndef EDITTREEVIEW_H_
#define EDITTREEVIEW_H_

#include <QtGui>

class EditTreeView : public QTreeView
{
	Q_OBJECT

public:
	EditTreeView(QWidget * parent = 0);

protected:
	void keyPressEvent (QKeyEvent * event);

signals:
	void deleteKeyPressed();
};

#endif /* EDITTREEVIEW_H_ */
