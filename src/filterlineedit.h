/*
 * filterlineedit.h
 *
 *  Created on: Apr 18, 2010
 *      Author: Sergey Stolyarov
 */

#ifndef FILTERLINEEDIT_H_
#define FILTERLINEEDIT_H_

#include <QLineEdit>

struct QResizeEvent;

class FilterLineEdit : public QLineEdit
{
	Q_OBJECT
public:
	FilterLineEdit(QWidget * parent = 0);
    void setClearButtonTooltip(const QString &);

protected:
	void keyPressEvent(QKeyEvent * event);
    void resizeEvent(QResizeEvent *);

signals:
	void escapeKeyPressed();

protected slots:
    void updateClearButton(const QString &);

private:
    struct Private;
    Private * p;
};

#endif /* FILTERLINEEDIT_H_ */
