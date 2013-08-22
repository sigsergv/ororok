/*
 * filterlineedit.h
 *
 *  Created on: Apr 18, 2010
 *      Author: Sergey Stolyarov
 */

#ifndef FILTERLINEEDIT_H_
#define FILTERLINEEDIT_H_

#include <QLineEdit>

class FilterLineEdit : public QLineEdit
{
	Q_OBJECT
public:
	FilterLineEdit(QWidget * parent = 0);

protected:
	void keyPressEvent(QKeyEvent * event);

signals:
	void escapeKeyPressed();
};

#endif /* FILTERLINEEDIT_H_ */
