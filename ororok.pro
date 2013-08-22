!exists( src/services/lastfm/apikey.h ) {
    error( "File 'src/services/lastfm/apikey.h' is absolutely required for build. If you don't want to lastfm functions to work just copy 'src/services/lastfm/apikey.h.example' to 'src/services/lastfm/apikey.h'. Otherwise you need correct API keys to access lastfm services." )
}

TEMPLATE = subdirs
SUBDIRS += src
DATADIR =$$PREFIX/share

update-translations.commands = lupdate src/src.pro
update-translations-clean.commands = lupdate -noobsolete src/src.pro
compile-translations.commands = lrelease src/src.pro
QMAKE_EXTRA_TARGETS = update-translations compile-translations update-translations-clean

translations.depends = compile-translations
translations.path = $$DATADIR/ororok/translations
translations.files = translations/*.qm

INSTALLS += translations
