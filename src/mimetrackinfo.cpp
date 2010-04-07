/*
 * mimetrackinfo.cpp
 *
 *  Created on: Oct 28, 2009
 *      Author: Sergei Stolyarov
 */
#include "mimetrackinfo.h"

namespace Ororok
{

QStringList emptyTrackInfo()
{
	QStringList res;
	for (int i=0; i<TrackInfoFieldsCount; i++) {
		res << QString();
	}
	return res;
}

}
