/*
 * settingsdialog.cpp
 *
 *  Created on: Nov 11, 2009
 *      Author: Sergei Stolyarov
 */
#include <QtDebug>
#include <QtGui>
#include <QtSql>
#include <QNetworkReply>

#include "db.h"
#include "application.h"
#include "settingsdialog.h"
#include "services/lastfm/auth.h"
#include "settings.h"
#include "ui_settingsdialog.h"

struct SettingsDialog::Private
{
	Ui::SettingsDialog ui;
	QNetworkReply * lastfmReply;
	bool beforeClose; // set to true after OK attempt and before applying lastfm settings
	ororok::lastfm::Auth * lastfmAuth;
};

SettingsDialog::SettingsDialog(QWidget * parent)
	: QDialog(parent)
{
	p = new Private;
	p->ui.setupUi(this);
	p->beforeClose = false;
	p->lastfmAuth = new ororok::lastfm::Auth(this);

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

	item = p->ui.settingsGroupsTable->item(4, 0);
	item->setIcon(QIcon(":preferences-desktop-keyboard.png"));
	item->setTextAlignment(Qt::AlignCenter);

	int w = p->ui.settingsGroupsTable->width();
	p->ui.settingsGroupsTable->setColumnWidth(0, w);

	// select first item
	p->ui.settingsGroupsTable->setCurrentCell(0, 0);

	// select first page
	p->ui.pages->setCurrentIndex(0);

	QSettings * settings = Ororok::settings();
	QString lastfmUsername = settings->value("LastFm/username").toString();
	QString lastfmSessionKey = settings->value("LastFm/sessionKey").toString();
	if (lastfmSessionKey.isEmpty() || lastfmUsername.isEmpty()) {
		// disable block
		p->ui.lastfmGroupBox->setChecked(false);
	} else {
		p->ui.lastfmGroupBox->setChecked(true);
		p->ui.lastfmUsernameLineEdit->setText(lastfmUsername);
	}
	p->ui.lastfmTestLoginProgressBar->hide();

	connectSignals();
}

SettingsDialog::~SettingsDialog()
{
	delete p;
}

int SettingsDialog::exec()
{
	// load settings
	if (!loadSettings()) {
		return QDialog::Rejected;
	}

	return QDialog::exec();
}

void SettingsDialog::accept()
{
	// check directories
	int cnt = p->ui.collectionsListWidget->count();

	QStringList newDirs;
	QHash<int, QString> existingDirs;

	for (int i=0; i<cnt; i++) {
		QListWidgetItem * item = p->ui.collectionsListWidget->item(i);
		if (!item->data(ItemRoleId).isValid()) {
			newDirs << item->text();
		} else {
			int id = item->data(ItemRoleId).toInt();
			existingDirs[id] = item->text();
		}
	}

	qDebug() << newDirs;

	// update database
	QSqlDatabase db = QSqlDatabase::database();
	QSqlQuery q(db);

	db.transaction();
	if (!q.exec("DELETE FROM collection")) {
		sqlErrorMsg(tr("Unable to clear collections list"), q.lastError());
		db.rollback();
		QDialog::reject();
		return;
	}


	q.prepare("INSERT INTO collection (id, path) VALUES (:id, :path)");
	for (QHash<int, QString>::iterator i=existingDirs.begin(); i!=existingDirs.end(); i++) {
		q.bindValue(":id", i.key());
		q.bindValue(":path", i.value());
		if (!q.exec()) {
			sqlErrorMsg(tr("Unable to insert collection dir record"), q.lastError());
			db.rollback();
			QDialog::reject();
			return;
		}
	};

	q.prepare("INSERT INTO collection (path) VALUES (:path)");
	foreach (const QString v, newDirs) {
		q.bindValue(":path", v);
		if (!q.exec()) {
			sqlErrorMsg(tr("Unable to insert collection dir record"), q.lastError());
			db.rollback();
			QDialog::reject();
			return;
		}
	}

	db.commit();

	QSettings * settings = Ororok::settings();

	// if Last.fm is turned on then try to login (if not yet logged in)
	if (p->ui.lastfmGroupBox->isChecked()) {
		// login again
		// TODO: implement more intellectual way of checking
		p->beforeClose = true;
		lastfmTestAuth();
	} else {
		// delete all lastfm related data
		settings->beginGroup("LastFm");
		settings->remove("username");
		settings->remove("sessionKey");
		settings->endGroup();
		QDialog::accept();
	}
}

bool SettingsDialog::loadSettings()
{
	QSqlDatabase db = QSqlDatabase::database();
	QSqlQuery q(db);

	if (!q.exec("SELECT id, path FROM collection")) {
		Ororok::critical(tr("Error"), tr("Unable to load collections list: database error. \n%1")
				.arg(q.lastError().text()));
		return false;
	}

	while (q.next()) {
		QString path = q.value(1).toString();
		QListWidgetItem * item = new QListWidgetItem();
		item->setText(path);
		item->setData(ItemRoleId, q.value(0));
		p->ui.collectionsListWidget->addItem(item);
	}

	return true;
}

void SettingsDialog::connectSignals()
{
	connect(p->ui.settingsGroupsTable, SIGNAL(itemSelectionChanged()),
			this, SLOT(tableSelectionChanged()));
	connect(p->ui.addCollectionDirButton, SIGNAL(pressed()),
			this, SLOT(addCollectionDir()));
	connect(p->ui.removeCollectionDirButton, SIGNAL(pressed()),
			this, SLOT(removeCollectionDir()));
	connect(p->ui.lastfmTestAuthButton, SIGNAL(clicked()),
			this, SLOT(lastfmTestAuth()));
	connect(p->lastfmAuth, SIGNAL(success(const QString &, const QString &)),
			this, SLOT(lastfmSuccessAuth(const QString &, const QString &)));
	connect(p->lastfmAuth, SIGNAL(failed(int, const QString &)),
			this, SLOT(lastfmFailedAuth(int, const QString &)));
}

void SettingsDialog::tableSelectionChanged()
{
	QTableWidgetItem * item = p->ui.settingsGroupsTable->selectedItems().first();
	p->ui.pages->setCurrentIndex(item->row());
}

void SettingsDialog::addCollectionDir()
{
	QStringList currentCollectionDirs;
	int cnt = p->ui.collectionsListWidget->count();
	for (int i=0; i<cnt; i++) {
		QListWidgetItem * item = p->ui.collectionsListWidget->item(i);
		QDir d(item->text());
		currentCollectionDirs << d.canonicalPath();
	}

	// open Select folder dialog
	QFileDialog fd(this);

	fd.setFileMode(QFileDialog::Directory);
	fd.setFilter(QDir::AllDirs);

	if (fd.exec()) {
		QStringList alreadyAddedDirs;

		foreach (QString dir, fd.selectedFiles()) {
			// check is dir in the list already
			QDir d(dir);
			dir = d.canonicalPath();

			if (currentCollectionDirs.contains(dir, Qt::CaseInsensitive)) {
				alreadyAddedDirs << dir;
				continue;
			}

			p->ui.collectionsListWidget->addItem(dir);
		}

		if (alreadyAddedDirs.count()) {
			QMessageBox::warning(this, tr("Warning"), tr("The following directories already included in collection:\n%1")
					.arg(alreadyAddedDirs.join("\n")));
		}
	}
}

void SettingsDialog::removeCollectionDir()
{
	// remove selected
	QList<QListWidgetItem*> items = p->ui.collectionsListWidget->selectedItems();
	foreach (QListWidgetItem * item, items) {
		int row = p->ui.collectionsListWidget->row(item);
		delete p->ui.collectionsListWidget->takeItem(row);
	}
}

void SettingsDialog::lastfmTestAuth()
{
	QString user = p->ui.lastfmUsernameLineEdit->text();
	QString pass = p->ui.lastfmPasswordLineEdit->text();

	p->ui.lastfmTestAuthButton->setEnabled(false);

	p->lastfmAuth->authenticate(user, pass);

	p->ui.lastfmTestLoginProgressBar->show();
	p->ui.lastfmTestAuthButton->hide();
	p->ui.lastfmTestAuthButton->setEnabled(true);
}

void SettingsDialog::lastfmSuccessAuth(const QString & name, const QString & sessionKey)
{
	p->ui.lastfmTestLoginProgressBar->hide();
	p->ui.lastfmTestAuthButton->show();

	// do not save anything during the test
	if (p->beforeClose) {
		QSettings * settings = Ororok::settings();
		settings->beginGroup("LastFm");
		settings->setValue("username", name);
		settings->setValue("sessionKey", sessionKey);
		settings->endGroup();

		// we're in the middle of dialog accepting so close window
		QDialog::accept();
	}
}

void SettingsDialog::lastfmFailedAuth(int error, const QString & errorText)
{
	p->ui.lastfmTestLoginProgressBar->hide();
	p->ui.lastfmTestAuthButton->show();

	QMessageBox::critical(this, tr("Last.fm authentication error"), errorText);
}

void SettingsDialog::sqlErrorMsg(const QString & message, const QSqlError & error)
{
	QMessageBox::critical(this, tr("Error"), tr("%1: database error\n\n%2")
			.arg(message)
			.arg(error.text()));
}
