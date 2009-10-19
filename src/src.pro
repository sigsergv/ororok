HEADERS += mainwindow.h \
	db.h \
	updatethread.h

SOURCES += main.cpp \
    mainwindow.cpp \
    db.cpp \
    updatethread.cpp

FORMS += mainwindow.ui

TARGET = ororok

QT += xml
QT += sql

win32 { 
    debug:DESTDIR = ../debug/
    release:DESTDIR = ../release/
}
else:DESTDIR = ../
