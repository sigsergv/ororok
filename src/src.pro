DEFINES += OROROK_VERSION="\\\"0.0.1\\\""

HEADERS += mainwindow.h \
	application.h \
	db.h \
	updatethread.h \
	formats.h \
	collectiontreewidget.h \
	collectionitemmodel.h \
	collectiontreeitem.h \
	collectionitemdelegate.h \
	settings.h \
	cache.h \
	collectiontreefilter.h \
	playlistwidget.h \
	playlistmodel.h \
	mimetrackinfo.h \
	playlistitemdelegate.h \
	player.h \
	playlistmanager.h \
    edittreeview.h \
    filterlineedit.h \
    playingcontextwidget.h \
    lastfmcontextwidget.h \
	settingsdialog.h \
    aboutdialog.h \
    renameplaylistdialog.h \
    desktopaccess.h \
    xmlplaylistreader.h \
    services/lastfm/lastfm.h \
    services/lastfm/auth.h \
    services/lastfm/scrobbleradapter.h

SOURCES += main.cpp \
    mainwindow.cpp \
    application.cpp \
    db.cpp \
    updatethread.cpp \
    formats.cpp \
    collectiontreewidget.cpp \
    collectionitemmodel.cpp \
    collectiontreeitem.cpp \
    collectionitemdelegate.cpp \
    settings.cpp \
    cache.cpp \
    collectiontreefilter.cpp \
    playlistwidget.cpp \
    playlistmodel.cpp \
	mimetrackinfo.cpp \
	playlistitemdelegate.cpp \
	player.cpp \
	playlistmanager.cpp \
    edittreeview.cpp \
    filterlineedit.cpp \
    playingcontextwidget.cpp \
    lastfmcontextwidget.cpp \
	settingsdialog.cpp \
    aboutdialog.cpp \
    renameplaylistdialog.cpp \
    desktopaccess.cpp \
    xmlplaylistreader.cpp \
    services/lastfm/lastfm.cpp \
    services/lastfm/auth.cpp \
    services/lastfm/scrobbleradapter.cpp

FORMS += mainwindow.ui \
	testwidget.ui \
	settingsdialog.ui \
    aboutdialog.ui \
    renameplaylistdialog.ui

RESOURCES = ../resources/application.qrc
TARGET = ororok
CODECFORTR = UTF-8

QT += xml sql phonon network webkit 

INCLUDEPATH = 3rdparty/globalshortcut
include(3rdparty/globalshortcut/globalshortcut.pri)

win32 { 
	debug:DESTDIR = ../debug/
	release:DESTDIR = ../release/
}
unix {
    INCLUDEPATH += /usr/include/phonon
	CONFIG += link_pkgconfig
	PKGCONFIG += taglib
    LIBS += -llastfm
}

DESTDIR = ../
