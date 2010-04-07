/*
 * main.cpp
 *
 *  Created on: Oct 19, 2009
 *      Author: Sergei Stolyarov
 */

#include <QtGui>
#include <QtSql>
#include <stdlib.h>

#include "mainwindow.h"
#include "settings.h"
#include "db.h"

int main(int argv, char *args[])
{
	// init rand
	srand(QDateTime::currentDateTime().toUTC().toTime_t());

	QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QApplication app(argv, args);
	app.setQuitOnLastWindowClosed(true);

	Ororok::initSettings();

    // load application settings

    if (!QSqlDatabase::drivers().contains("QSQLITE")) {
        QMessageBox::critical(0, "Unable to load database", "This application needs the SQLITE driver");
        return 1;
    }

    QSqlError err = initDb();
    if (err.type() != QSqlError::NoError) {
    	QMessageBox::critical(0, "fatal database error", err.text());
    	return 1;
    }

    MainWindow win;
    QApplication::setActiveWindow(&win);
    win.show();
    // maximize, for debug purposes
    //win.showMaximized();

    return app.exec();
}
