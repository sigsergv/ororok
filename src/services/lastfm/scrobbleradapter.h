/*
 * scrobbleradapter.h
 *
 *  Created on: Mar 31, 2010
 *      Author: Sergei Stolyarov
 */
#include <QObject>

struct QString;

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

	void nowPlaying(const QString & title, const QString & artist, const QString & album, int duration);

private:
	struct Private;
	Private * p;


};
}
}
