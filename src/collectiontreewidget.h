/*
 * collectiontreewidget.h
 *
 *  Created on: 22.10.2009
 *      Author: Sergei Stolyarov
 */

#ifndef COLLECTIONTREEWIDGET_H_
#define COLLECTIONTREEWIDGET_H_

#include <QDockWidget>

class CollectionTreeWidget : public QWidget
{
	Q_OBJECT
public:
	CollectionTreeWidget(QWidget * parent = 0);
	bool reloadTree();

protected:
	struct Private;
	Private * p;
};

#endif /* COLLECTIONTREEWIDGET_H_ */
