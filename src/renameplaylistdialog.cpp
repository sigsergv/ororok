/*
 * renameplaylistdialog.cpp
 *
 *  Created on: Apr 13, 2010
 *      Author: Sergey Stolyarov
 */

#include "renameplaylistdialog.h"
#include "ui_renameplaylistdialog.h"

struct RenamePlaylistDialog::Private {
	Ui::RenamePlaylistDialog ui;
};

RenamePlaylistDialog::RenamePlaylistDialog(QWidget * parent)
	: QDialog(parent)
{
	p = new Private;
	p->ui.setupUi(this);
	connect(p->ui.okButton, SIGNAL(clicked()), this, SLOT(accept()));
	connect(p->ui.cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

RenamePlaylistDialog::~RenamePlaylistDialog()
{
	delete p;
}

void RenamePlaylistDialog::setPlaylistName(const QString & name)
{
	p->ui.playlistNameLineEdit->setText(name);
}

QString RenamePlaylistDialog::playlistName()
{
	return p->ui.playlistNameLineEdit->text();
}

void RenamePlaylistDialog::setPlaylistRemembered(bool r)
{
	p->ui.remeberPlaylistCheckbox->setChecked(r);
}

bool RenamePlaylistDialog::isPlaylistRemembered()
{
	return p->ui.remeberPlaylistCheckbox->isChecked();
}

void RenamePlaylistDialog::accept()
{
	QDialog::accept();
}

