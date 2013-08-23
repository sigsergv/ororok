About Ororok
============

Idea of player is inspired by amarok 1.4 and partially by foobar2000. It could play (and organize) mp3 and 
flac files. It also handles VA-albums in a “smart” way, so such albums are not spread over collection items
but instead correctly placed in one collection “folder”.

Main goals
==========

* collections support;
* multiple playlists support;
* accessing player functions via dbus;
* scrobbling to last.fm (and possible to other services);
* loading music to iPod and other mp3 players.

What's already done
===================

* multiple playlists support;
* fast collection directories scanning;
* fast incremental update of collections;
* auto grouping albums into the virtual artist {Various Artists};
* global hotkeys support (including one for lastfm “love track”);
* loading album cover images (from album dir only for now);
* scrobbling to last.fm;
* displaying track and artist info retrieved from last.fm;
* crash-safe playlists (you will never lose tracks from playlist);
* fast collection search and filtering.

Planning future
===============

I do not plan to support “virtual” media files, like playing .CUE-files etc. 

You may report bug and feature requests using standard Github “Issue” page. 

See also list of planned enhancements in the issue tracker. 

Minimal system requirements
===========================

* Qt >= 4.8 
* linux 
* liblastfm1 (for ororok version >= 1.2) 
* liblastfm0 (for ororok version < 1.2) 
* Installation instructions

There are no binary builds so far. Just clone the repository and build it by yourself. 

Project depends on the following 3rd party software and libs:

* liblastfm 
* liblastfm, new generation 
* TagLib 


Development
===========

To build development & debug version use the following command

    qmake "CONFIG+=debug"

To build release version:

    qmake "CONFIG+=release"

