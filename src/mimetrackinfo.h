/*
 * mimetrackinfo.h
 *
 *  Created on: Oct 28, 2009
 *      Author: Sergey Stolyarov
 */

#ifndef MIMETRACKINFO_H_
#define MIMETRACKINFO_H_

#include <QStringList>

namespace Ororok
{
// trackinfo "structure" is usually filled in the file "collectionitemmodel.cpp",
// in the function "data()"

enum TrackInfoField { TrackPlaylistId=0, TrackNumInPlaylist,
	TrackReservedDataField3, TrackReservedDataField4,
	TrackFieldPath, TrackFieldNo, TrackFieldTitle, TrackFieldYear,
	TrackFieldAlbum, TrackFieldArtist, TrackFieldGenre, TrackFieldLength,
	TrackInfoFieldsCount
	};

const QString TRACKS_MIME("application/x-ororok-tracks");
const QString TRACKS_COLLECTION_IDS_MIME("application/x-ororok-collection-track-ids");
const QString TRACKS_PLAYLIST_ITEMS_MIME("application/x-ororok-playlist-items");

QStringList emptyTrackInfo();

}

#endif /* MIMETRACKINFO_H_ */
