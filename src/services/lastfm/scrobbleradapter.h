/*
 * scrobbleradapter.h
 *
 *  Created on: Mar 31, 2010
 *      Author: Sergei Stolyarov
 */
#include <QObject>

struct QString;
struct QDateTime;

namespace Ororok
{
namespace lastfm
{

class ScrobblerAdapter : public QObject
{
	Q_OBJECT
public:
	ScrobblerAdapter(QObject * parent);
	~ScrobblerAdapter();

	void nowPlaying(const QString & title, const QString & artist, const QString & album, uint duration);
	void love(const QString & title, const QString & artist, const QString & album);
	void submit(const QString & title, const QString & artist, const QString & album,
			uint duration, uint trackNum, QDateTime timeStarted);

private:
	struct Private;
	Private * p;


};
}
}
