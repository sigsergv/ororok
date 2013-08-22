/*
 * collectiontreewidget.h
 *
 *  Created on: 22.10.2009
 *      Author: Sergey Stolyarov
 */

#ifndef COLLECTIONTREEWIDGET_H_
#define COLLECTIONTREEWIDGET_H_

#include <QDockWidget>
#include <QModelIndex>

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
	void dateFilterChanged(int index);
	void itemContextMenu(const QPoint & pos);
	void appendItemToCurrentPlaylist();
private:
	struct Private;
	Private * p;
};

#endif /* COLLECTIONTREEWIDGET_H_ */
