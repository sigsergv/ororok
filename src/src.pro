HEADERS += mainwindow.h \
	db.h \
	updatethread.h \
	formats.h \
	collectiontreewidget.h \
	collectionitemmodel.h \
	collectiontreeitem.h \
	albumitemdelegate.h \
	settings.h \
	cache.h \
	collectiontreefilter.h

SOURCES += main.cpp \
    mainwindow.cpp \
    db.cpp \
    updatethread.cpp \
    formats.cpp \
    collectiontreewidget.cpp \
    collectionitemmodel.cpp \
    collectiontreeitem.cpp \
    albumitemdelegate.cpp \
    settings.cpp \
    cache.cpp \
    collectiontreefilter.cpp

FORMS += mainwindow.ui \
	testwidget.ui

TARGET = ororok
CODECFORTR = UTF-8

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
