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

protected slots:
	void applyFilter();
	void resetFilter();
	void createModel();
	void filterTextChanged(const QString & text);
	void filterEditFinished();
private:
	struct Private;
	Private * p;
};

#endif /* COLLECTIONTREEWIDGET_H_ */
