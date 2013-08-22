/*
 * auth.h
 *
 *  Created on: Mar 31, 2010
 *      Author: Sergey Stolyarov
 */

#ifndef AUTH_H_
#define AUTH_H_

#include <QObject>

struct QString;

namespace Ororok
{
namespace lastfm
{

class Auth : public QObject
{
	Q_OBJECT
public:
	Auth(QObject * parent = 0);
	~Auth();
	void authenticate(const QString & username, const QString & password);
	void reset();

protected slots:
	void lastfmQueryFinished();

signals:
	void success(const QString & name, const QString & sessionKey);
	void failed(int errorCode, const QString & error);

private:
	struct Private;
	Private * p;
};

}
}

#endif /* AUTH_H_ */
