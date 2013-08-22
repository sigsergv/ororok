/*
 * renameplaylistdialog.h
 *
 *  Created on: Apr 13, 2010
 *      Author: Sergey Stolyarov
 */

#ifndef RENAMEPLAYLISTDIALOG_H_
#define RENAMEPLAYLISTDIALOG_H_

#include <QDialog>

class RenamePlaylistDialog : public QDialog
{
	Q_OBJECT
public:
	RenamePlaylistDialog(QWidget * parent = 0);
	~RenamePlaylistDialog();

	void setPlaylistName(const QString & name);
	QString playlistName();
	void setPlaylistRemembered(bool r);
	bool isPlaylistRemembered();

public slots:
	void accept();

private:
	struct Private;
	Private * p;
};

#endif /* RENAMEPLAYLISTDIALOG_H_ */
