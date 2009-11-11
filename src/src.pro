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
	collectiontreefilter.h \
	playlistwidget.h \
	playlistmodel.h \
	mimetrackinfo.h \
	playlistitemdelegate.h \
	player.h \
	playlistmanager.h \
	settingsdialog.h

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
    collectiontreefilter.cpp \
    playlistwidget.cpp \
    playlistmodel.cpp \
	mimetrackinfo.cpp \
	playlistitemdelegate.cpp \
	player.cpp \
	playlistmanager.cpp \
	settingsdialog.cpp

FORMS += mainwindow.ui \
	testwidget.ui \
	settingsdialog.ui

RESOURCES = ../resources/application.qrc
TARGET = ororok
CODECFORTR = UTF-8

QT += xml
QT += sql
QT += phonon

win32 { 
	debug:DESTDIR = ../debug/
	release:DESTDIR = ../release/
}
unix {
	CONFIG += link_pkgconfig
	PKGCONFIG += taglib
}

DESTDIR = ../
