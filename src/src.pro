HEADERS += mainwindow.h \
	db.h \
	updatethread.h \
	formats.h

SOURCES += main.cpp \
    mainwindow.cpp \
    db.cpp \
    updatethread.cpp \
    formats.cpp

FORMS += mainwindow.ui

TARGET = ororok

QT += xml
QT += sql


win32 { 
	debug:DESTDIR = ../debug/
	release:DESTDIR = ../release/
}
unix {
	CONFIG += link_pkgconfig
	PKGCONFIG += taglib
}

DESTDIR = ../
