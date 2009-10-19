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
    if (!QSqlDatabase::drivers().contains("QSQLITE")) {
        QMessageBox::critical(0, "Unable to load database", "This application needs the SQLITE driver");
        return 1;
    }

    initDb();

    QApplication app(argv, args);
    MainWindow win;
    //win.show();
    // maximize, for debug purposes
    win.showMaximized();

    return app.exec();
}
