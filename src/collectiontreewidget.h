/*
 * collectiontreewidget.h
 *
 *  Created on: 22.10.2009
 *      Author: Sergei Stolyarov
 */

#ifndef COLLECTIONTREEWIDGET_H_
#define COLLECTIONTREEWIDGET_H_

#include <QDockWidget>

class QLineEdit;
class QTreeView;

class CollectionTreeWidget : public QWidget
{
	Q_OBJECT
public:
	CollectionTreeWidget(QWidget * parent = 0);

protected:
	QLineEdit * filter;
	QTreeView * collectionTreeView;
};

#endif /* COLLECTIONTREEWIDGET_H_ */
