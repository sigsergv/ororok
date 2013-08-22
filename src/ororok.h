/*
 * ororok.h
 *
 *  Created on: Jul 4, 2010
 *      Author: Sergey Stolyarov
 */

#ifndef OROROK_H_
#define OROROK_H_

namespace Ororok
{

enum PlaylistType {PlaylistTemporary = 't', PlaylistPermanent = 'p'};

struct PlaylistInfo {
	QString uid;
	Ororok::PlaylistType type;
};

}

#endif /* OROROK_H_ */
