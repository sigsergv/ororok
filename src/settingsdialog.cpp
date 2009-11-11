/*
 * settingsdialog.cpp
 *
 *  Created on: Nov 11, 2009
 *      Author: Sergei Stolyarov
 */
#include <QtDebug>

#include "settingsdialog.h"
#include "ui_settingsdialog.h"

struct SettingsDialog::Private
{
	Ui::SettingsDialog ui;

};

SettingsDialog::SettingsDialog(QWidget * parent)
	: QDialog(parent)
{
	p = new Private;
	p->ui.setupUi(this);

	// set icons in the settings list

	QTableWidgetItem * item;
	p->ui.settingsGroupsTable->setIconSize(QSize(32,32));

	item = p->ui.settingsGroupsTable->item(0, 0);
	item->setIcon(QIcon(":preferences-other.png"));
	item->setTextAlignment(Qt::AlignCenter);

	item = p->ui.settingsGroupsTable->item(1, 0);
	item->setIcon(QIcon(":tools-media-optical-format.png"));
	item->setTextAlignment(Qt::AlignCenter);

	item = p->ui.settingsGroupsTable->item(2, 0);
	item->setIcon(QIcon(":preferences-system-network.png"));
	item->setTextAlignment(Qt::AlignCenter);

	item = p->ui.settingsGroupsTable->item(3, 0);
	item->setIcon(QIcon(":server-database.png"));
	item->setTextAlignment(Qt::AlignCenter);

	int w = p->ui.settingsGroupsTable->width();
	p->ui.settingsGroupsTable->setColumnWidth(0, w);

	// select first item
	p->ui.settingsGroupsTable->setCurrentCell(0, 0);

	// select first page
	p->ui.pages->setCurrentIndex(0);

	// connect signals
	//connect(p->ui.settingsGroupsTable, SIGNAL(cellActivated(int,int)), this, SLOT(cellActivated(int, int)));
	connect(p->ui.settingsGroupsTable, SIGNAL(itemSelectionChanged()), this, SLOT(tableSelectionChanged()));
}

SettingsDialog::~SettingsDialog()
{
	delete p;
}

void SettingsDialog::tableSelectionChanged()
{
	QTableWidgetItem * item = p->ui.settingsGroupsTable->selectedItems().first();
	p->ui.pages->setCurrentIndex(item->row());
}
