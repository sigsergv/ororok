/*
 * settingsdialog.cpp
 *
 *  Created on: Nov 11, 2009
 *      Author: Sergey Stolyarov
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
#include "mainwindow.h"

struct SettingsDialog::Private
{
	Ui::SettingsDialog ui;
	QNetworkReply * lastfmReply;
	bool beforeClose; // set to true after OK attempt and before applying lastfm settings
	Ororok::lastfm::Auth * lastfmAuth;
};

SettingsDialog::SettingsDialog(QWidget * parent)
	: QDialog(parent)
{
	p = new Private;
	p->ui.setupUi(this);
	p->beforeClose = false;
	p->lastfmAuth = new Ororok::lastfm::Auth(this);

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

	/*
	item = p->ui.settingsGroupsTable->item(3, 0);
	item->setIcon(QIcon(":server-database.png"));
	item->setTextAlignment(Qt::AlignCenter);
	*/

	item = p->ui.settingsGroupsTable->item(3, 0);
	item->setIcon(QIcon(":preferences-desktop-keyboard.png"));
	item->setTextAlignment(Qt::AlignCenter);

	// load languages
	p->ui.uiLangCombo->addItem(tr("Use system language"), "system");
	p->ui.uiLangCombo->addItem(tr("English"), "en");
	p->ui.uiLangCombo->addItem(tr("Russian (Русский)"), "ru");

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

	// now set global hotkeys
	bool globalShortcutsEnabled = settings->value("GlobalShortcuts/enabled").toBool();
	p->ui.globalShortcutsGroup->setChecked(globalShortcutsEnabled);
	p->ui.stopShortcut->setKeySequence(QKeySequence(settings->value("GlobalShortcuts/stopTrack").toString()));
	p->ui.prevTrackShortcut->setKeySequence(QKeySequence(settings->value("GlobalShortcuts/prevTrack").toString()));
	p->ui.playPauseShortcut->setKeySequence(QKeySequence(settings->value("GlobalShortcuts/playPauseTrack").toString()));
	p->ui.nextTrackShortcut->setKeySequence(QKeySequence(settings->value("GlobalShortcuts/nextTrack").toString()));
	p->ui.lastfmLoveShortcut->setKeySequence(QKeySequence(settings->value("GlobalShortcuts/lastfmLoveTrack").toString()));

	// button box buttons are not localized properly, so use this hack
	p->ui.buttonBox->button(QDialogButtonBox::Ok)->setText(tr("OK"));
	p->ui.buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

	connectSignals();
}

SettingsDialog::~SettingsDialog()
{
	delete p;
}

int SettingsDialog::exec()
{
	// disable global shortcuts while editing preferences
	MainWindow * w = MainWindow::inst();
	w->unloadGlobalShortcuts();

	// load settings
	if (!loadSettings()) {
		return QDialog::Rejected;
	}

	int res = QDialog::exec();
	w->loadGlobalShortcuts();

	return res;
}

void SettingsDialog::accept()
{
	QSettings * settings = Ororok::settings();

	// set global shortcuts
	settings->beginGroup("GlobalShortcuts");
	settings->setValue("enabled", p->ui.globalShortcutsGroup->isChecked());
	settings->setValue("stopTrack", p->ui.stopShortcut->keySequence().toString());
	settings->setValue("prevTrack", p->ui.prevTrackShortcut->keySequence().toString());
	settings->setValue("nextTrack", p->ui.nextTrackShortcut->keySequence().toString());
	settings->setValue("playPauseTrack", p->ui.playPauseShortcut->keySequence().toString());
	settings->setValue("lastfmLoveTrack", p->ui.lastfmLoveShortcut->keySequence().toString());
	settings->endGroup();

	// if Last.fm is turned on then try to login (if not yet logged in)
	if (p->ui.lastfmGroupBox->isChecked()) {
		QString pass = p->ui.lastfmPasswordLineEdit->text();
		QString lastfmSessionKey = settings->value("LastFm/sessionKey").toString();

		if (!pass.isEmpty() || lastfmSessionKey.isEmpty()) {
			// do not authenticate if lastfm password field is empty and session already initialized
			p->beforeClose = true;
			lastfmTestAuth();
			return;
		}
	} else {
		// delete all lastfm related data
		settings->beginGroup("LastFm");
		settings->remove("username");
		settings->remove("sessionKey");
		settings->endGroup();
	}

	// set language
	int index = p->ui.uiLangCombo->currentIndex();
	QString lang = p->ui.uiLangCombo->itemData(index).toString();
	settings->setValue("MainWindow/language", lang);

	// check directories
	int cnt = p->ui.collectionsListWidget->count();

	QStringList newDirs;
	QHash<int, QString> existingDirs;

	for (int i=0; i<cnt; i++) {
		QListWidgetItem * item = p->ui.collectionsListWidget->item(i);
		newDirs << item->text();
	}

	// update database, add/remove collection directories
	QSqlDatabase db = QSqlDatabase::database();
	QSqlQuery q(db);

	if (!q.exec("SELECT path FROM collection")) {
		sqlErrorMsg(tr("Unable to fetch collections list"), q.lastError());
		QDialog::reject();
		return;
	}

	QStringList oldDirs;
	while (q.next()) {
		oldDirs << q.value(0).toString();
	}

	oldDirs.sort();
	newDirs.sort();
	if (oldDirs.join("\n") != newDirs.join("\n")) {
		// update database because directories list has changed
		db.transaction();
		if (!q.exec("DELETE FROM collection")) {
			sqlErrorMsg(tr("Unable to clear collections list"), q.lastError());
			db.rollback();
			QDialog::reject();
			return;
		}

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
		emit(collectionsChanged());
	}

	QDialog::accept();
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

	QSettings * settings = Ororok::settings();
	QString lang = settings->value("MainWindow/language").toString();
    QStringList uiLangs = Ororok::supportedUiLangs();

    if (!lang.isEmpty() && uiLangs.contains(lang)) {
    	int index = p->ui.uiLangCombo->findData(lang);
    	if (index != -1) {
    		p->ui.uiLangCombo->setCurrentIndex(index);
    	}
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
	QString dir = QFileDialog::getExistingDirectory(this, tr("Add directory to collection"));
	if (!dir.isEmpty()) {
		// check is dir in the list already
		QDir d(dir);
		bool ok = true;

		if (currentCollectionDirs.contains(d.absolutePath(), Qt::CaseInsensitive)) {
			QMessageBox::warning(this, tr("Warning"), tr("Selected directory `%1` is already added.")
				.arg(d.absolutePath()));
			ok = false;
		}

		if (ok && currentCollectionDirs.contains(d.canonicalPath(), Qt::CaseInsensitive)) {
			QMessageBox::warning(this, tr("Warning"), tr("Selected directory `%1` is a symbolic link to already added directory `%2`.")
				.arg(d.absolutePath())
				.arg(d.canonicalPath()));
			ok = false;
		}

		QString canonicalPath = d.canonicalPath();
		// check is canonicalPath is a subfolder of already added directory
		if (ok) {
			foreach (const QString & collectionPath, currentCollectionDirs) {
				QDir collectionDir(collectionPath);
				collectionDir.setPath(collectionDir.canonicalPath());
				QString relPath = collectionDir.relativeFilePath(canonicalPath);
				if (!relPath.startsWith("..")) {

				QMessageBox::warning(this, tr("Warning"), tr("Selected directory `%1` is a subdirectory of already added directory `%2`.")
					.arg(dir)
					.arg(collectionPath));

					ok = false;
				}
			}
		}

		if (ok) {
			p->ui.collectionsListWidget->addItem(dir);
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
	Q_UNUSED(error)
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
