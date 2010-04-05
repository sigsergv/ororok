/*
 * aboutdialog.cpp
 *
 *  Created on: Apr 5, 2010
 *      Author: Sergei Stolyarov
 */

#include "aboutdialog.h"
#include "settings.h"
#include "ui_aboutdialog.h"

struct AboutDialog::Private
{
	Ui::AboutDialog ui;
};

AboutDialog::AboutDialog(QWidget * parent, Qt::WindowFlags f)
	: QDialog(parent, f)
{
	p = new Private;
	p->ui.setupUi(this);

	// update labels
	QString label;
	label = p->ui.appVersionLabel->text();
	label.replace(QString("%VERSION"), Ororok::version());
	p->ui.appVersionLabel->setText(label);

	label = p->ui.qtVersionLabel->text();
	label.replace(QString("%RVERSION"), QString(qVersion())).replace(QString("%BVERSION"), QT_VERSION_STR);
	p->ui.qtVersionLabel->setText(label);

	connect(p->ui.closeButton, SIGNAL(clicked()), this, SLOT(accept()));
}

AboutDialog::~AboutDialog()
{

}

