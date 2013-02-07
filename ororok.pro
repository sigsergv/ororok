!exists( src/services/lastfm/apikey.h ) {
    error( "File 'src/services/lastfm/apikey.h' is absolutely required for build. If you don't want to lastfm functions to work just copy 'src/services/lastfm/apikey.h.example' to 'src/services/lastfm/apikey.h'. Otherwise you need correct API keys to access lastfm services." )
}

TEMPLATE = subdirs
SUBDIRS += src
