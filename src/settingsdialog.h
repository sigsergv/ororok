/*
 * settingsdialog.h
 *
 *  Created on: Nov 11, 2009
 *      Author: Sergei Stolyarov
 */

#ifndef SETTINGSDIALOG_H_
#define SETTINGSDIALOG_H_

#include <QDialog>

struct QSqlError;

class SettingsDialog : public QDialog
{
	Q_OBJECT
public:
	SettingsDialog(QWidget * parent = 0);
	~SettingsDialog();
	int exec();

public slots:
	void accept();

protected:
	bool loadSettings();
	void connectSignals();

protected slots:
	void tableSelectionChanged();
	void addCollectionDir();
	void removeCollectionDir();
	void lastfmTestAuth();

	void lastfmSuccessAuth(const QString & name, const QString & sessionKey);
	void lastfmFailedAuth(int error, const QString & errorText);

protected:
	void sqlErrorMsg(const QString & message, const QSqlError & error);

private:
	enum ItemRole { ItemRoleId = Qt::UserRole+1 };
	struct Private;
	Private * p;
};

#endif /* SETTINGSDIALOG_H_ */
