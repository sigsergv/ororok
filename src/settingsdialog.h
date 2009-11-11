/*
 * settingsdialog.h
 *
 *  Created on: Nov 11, 2009
 *      Author: Sergei Stolyarov
 */

#ifndef SETTINGSDIALOG_H_
#define SETTINGSDIALOG_H_

#include <QDialog>

class SettingsDialog : public QDialog
{
	Q_OBJECT
public:
	SettingsDialog(QWidget * parent = 0);
	~SettingsDialog();

protected slots:
	void tableSelectionChanged();

private:
	struct Private;
	Private * p;
};

#endif /* SETTINGSDIALOG_H_ */
