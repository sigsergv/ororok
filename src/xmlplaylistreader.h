/*
 * xmlplaylistreader.h
 *
 *  Created on: Apr 16, 2010
 *      Author: Sergey Stolyarov
 */

#ifndef XMLPLAYLISTREADER_H_
#define XMLPLAYLISTREADER_H_

#include <QStringList>

struct QIODevice;

namespace Ororok
{

class XmlPlaylistReader
{
public:
	XmlPlaylistReader(QIODevice * device);
	~XmlPlaylistReader();
	QList<QStringList> tracks();
	QString name();

private:
	struct Private;
	Private * p;

	void readPlaylist();
	void readInfo();
	void readTracks();
};

}
#endif /* XMLPLAYLISTREADER_H_ */
