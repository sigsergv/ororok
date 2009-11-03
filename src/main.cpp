/*
 * main.cpp
 *
 *  Created on: Oct 19, 2009
 *      Author: Sergei Stolyarov
 */

#include <QtGui>
#include <QtSql>

#include "mainwindow.h"
#include "db.h"

int main(int argv, char *args[])
{
	QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QApplication app(argv, args);
    app.setApplicationName("Ororok");
    app.setQuitOnLastWindowClosed(true);

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
    win.show();
    // maximize, for debug purposes
    //win.showMaximized();

    return app.exec();
}
