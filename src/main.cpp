/*
 * main.cpp
 *
 *  Created on: Oct 19, 2009
 *      Author: Sergei Stolyarov
 */

#include <QtGui>
#include <QtSql>
#include <QtDebug>
#include <stdlib.h>

#include "mainwindow.h"
#include "settings.h"
#include "db.h"

int main(int argv, char *_args[])
{
	// init rand
	srand(QDateTime::currentDateTime().toUTC().toTime_t());

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
	QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QApplication app(argv, _args);
	app.setQuitOnLastWindowClosed(true);

    // load localization
    QTranslator translator;
    translator.load("ororok_" + Ororok::uiLang(), Ororok::uiLangsPath());
    app.installTranslator(&translator);

	QStringList args = QCoreApplication::arguments();
	if (args.size() > 1 && ( args[1] == "--help" || args[1] == "-h" )) {
		QTextStream qout(stdout);
		qout << "Arguments:\n    --disable-lastfm-submit    Disable submit to last.fm\n"
				<< "    --disable-lastfm-lookup    Disable playing tracks and artists last.fm lookup\n";
		return 0;
	}

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
