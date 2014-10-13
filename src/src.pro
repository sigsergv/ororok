DEFINES += OROROK_VERSION="\\\"1.5\\\""

HEADERS += mainwindow.h \
    ororok.h \
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
    maintabstabwidget.h \
    maintabswidget.h \
    edittreeview.h \
    filterlineedit.h \
    playingcontextwidget.h \
    lastfmcontextwidget.h \
    settingsdialog.h \
    aboutdialog.h \
    renameplaylistdialog.h \
    desktopaccess.h \
    xmlplaylistreader.h \
    shortcutedit.h \
    services/lastfm/lastfm.h \
    services/lastfm/auth.h \
    services/lastfm/scrobbleradapter.h \
    services/lastfm/networkaccessmanager.h

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
    maintabstabwidget.cpp \
    maintabswidget.cpp \
    edittreeview.cpp \
    filterlineedit.cpp \
    playingcontextwidget.cpp \
    lastfmcontextwidget.cpp \
    settingsdialog.cpp \
    aboutdialog.cpp \
    renameplaylistdialog.cpp \
    desktopaccess.cpp \
    xmlplaylistreader.cpp \
    shortcutedit.cpp \
    services/lastfm/lastfm.cpp \
    services/lastfm/auth.cpp \
    services/lastfm/scrobbleradapter.cpp \
    services/lastfm/networkaccessmanager.cpp

FORMS += mainwindow.ui \
    settingsdialog.ui \
    aboutdialog.ui \
    renameplaylistdialog.ui

CODECFORTR = UTF-8
TRANSLATIONS = ../translations/ororok_ru.ts

RESOURCES = ../resources/application.qrc
TARGET = ororok
CODECFORTR = UTF-8

QT += gui widgets webkit webkitwidgets xml sql multimedia network x11extras

INCLUDEPATH += 3rdparty/globalshortcut
include(3rdparty/globalshortcut/globalshortcut.pri)

win32 { 
	debug:DESTDIR = ../debug/
	release:DESTDIR = ../release/
}

CONFIG(release, release|debug) {
    message(RELEASE)
    DEFINES += QT_NO_DEBUG_OUTPUT
}


unix {
	INCLUDEPATH += /usr/include/phonon
	CONFIG += link_pkgconfig
	PKGCONFIG += taglib
	LIBS += -llastfm -lX11

	#VARIABLES
	isEmpty(PREFIX) {
	PREFIX = /usr
	}
	BINDIR = $$PREFIX/bin
	DATADIR =$$PREFIX/share

	DEFINES += DATADIR=\\\"$$DATADIR\\\" PKGDATADIR=\\\"$$PKGDATADIR\\\"

	#MAKE INSTALL

	target.path =$$BINDIR

	desktop.path = $$DATADIR/applications
	desktop.files += ../ororok.desktop

	#service.path = $$DATADIR/dbus-1/services
	#service.files += $${TARGET}.service

	icon16.path = $$DATADIR/icons/hicolor/16x16/apps/
	icon16.files = ../icons/16x16/ororok.png

	icon32.path = $$DATADIR/icons/hicolor/32x32/apps/
	icon32.files = ../icons/32x32/ororok.png

	icon48.path = $$DATADIR/icons/hicolor/48x48/apps/
	icon48.files = ../icons/48x48/ororok.png

	INSTALLS += target desktop icon16 icon32 icon48
}

DESTDIR = ../
