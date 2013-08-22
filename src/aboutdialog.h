/*
 * aboutdialog.h
 *
 *  Created on: Apr 5, 2010
 *      Author: Sergey Stolyarov
 */

#ifndef ABOUTDIALOG_H_
#define ABOUTDIALOG_H_

#include <QDialog>

class AboutDialog : public QDialog
{
	Q_OBJECT
public:
	AboutDialog(QWidget * parent = 0, Qt::WindowFlags f = 0);
	~AboutDialog();

private:
	struct Private;
	Private * p;
};

#endif /* ABOUTDIALOG_H_ */
