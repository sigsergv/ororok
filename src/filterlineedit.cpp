/*
 * filterlineedit.cpp
 *
 *  Created on: Apr 18, 2010
 *      Author: Sergey Stolyarov
 */

#include <QToolButton>
#include <QStyle>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QtDebug>

#include "filterlineedit.h"

const int ICONBUTTON_SIZE = 16;

struct FilterLineEdit::Private
{
    QToolButton * clearButton;
};

FilterLineEdit::FilterLineEdit(QWidget * parent)
	: QLineEdit(parent)
{
    p = new Private;

    p->clearButton = new QToolButton(this);
    p->clearButton->setIcon(QIcon(":edit-clear-locationbar-rtl-16x16.png"));
    p->clearButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");
    p->clearButton->setCursor(Qt::ArrowCursor);
    p->clearButton->setFocusPolicy(Qt::NoFocus);
    p->clearButton->hide();

    QSize size(ICONBUTTON_SIZE + 6, ICONBUTTON_SIZE + 2);

    // Note KDE does not reserve space for the highlight color
    if (style()->inherits("OxygenStyle")) {
        size = size.expandedTo(QSize(24, 0));
    }

    QMargins margins = textMargins();
    if (layoutDirection() == Qt::LeftToRight)
        margins.setRight(size.width());
    else
        margins.setLeft(size.width());

    setTextMargins(margins);

    connect(p->clearButton, SIGNAL(clicked()), 
        this, SLOT(clear()));
    connect(this, SIGNAL(textChanged(const QString &)), 
        this, SLOT(updateClearButton(const QString)));

    qDebug() << "layout" << layout();
}

void FilterLineEdit::setClearButtonTooltip(const QString & text)
{
    p->clearButton->setToolTip(text);
}

void FilterLineEdit::keyPressEvent(QKeyEvent * event)
{
	if (event->key() == Qt::Key_Escape) {
		emit escapeKeyPressed();
	}
	QLineEdit::keyPressEvent(event);
}

void FilterLineEdit::updateClearButton(const QString & text)
{
    p->clearButton->setVisible(!text.isEmpty());
}

void FilterLineEdit::resizeEvent(QResizeEvent *)
{
    QSize sz = p->clearButton->sizeHint();
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    p->clearButton->move(rect().right() - frameWidth - sz.width(),
        (rect().bottom() + 1 - sz.height())/2);
}
