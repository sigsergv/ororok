/*
 * auth.cpp
 *
 *  Created on: Mar 31, 2010
 *      Author: Sergey Stolyarov
 */
#include <QtDebug>
#include <QString>
#include <QNetworkReply>

#include "lastfm.h"
#include "auth.h"

struct Ororok::lastfm::Auth::Private
{
	QNetworkReply * lastfmReply;
};

Ororok::lastfm::Auth::Auth(QObject * parent)
	: QObject(parent)
{
	p = new Private;
}

Ororok::lastfm::Auth::~Auth()
{
	delete p;
}

void Ororok::lastfm::Auth::authenticate(const QString & username, const QString & password)
{
	::lastfm::ws::Username = username;

	QMap<QString, QString> params;
    params["method"] = "auth.getMobileSession";
	params["username"] = username;
    params["password"] = password;
    //params["authToken"] = ::lastfm::md5( (username + ::lastfm::md5(password.toUtf8()) ).toUtf8() );

	p->lastfmReply = ::lastfm::ws::post(params);

	connect(p->lastfmReply, SIGNAL(finished()), this, SLOT(lastfmQueryFinished()));
}

void Ororok::lastfm::Auth::reset()
{
	::lastfm::ws::SessionKey.clear();
	::lastfm::ws::Username.clear();
}

void Ororok::lastfm::Auth::lastfmQueryFinished()
{
	Ororok::lastfm::Response lfr(Ororok::lastfm::parseReply(p->lastfmReply));
    if (lfr.error()) {
		// cleanup username and session data
		reset();
		emit failed(lfr.error(), lfr.errorText());
		return;
	}

    ::lastfm::XmlQuery lfm;
    lfm.parse(lfr.data);
	// replace username, just in case
	::lastfm::ws::Username = lfm["session"]["name"].text();
	// remember session key, it's important to re-use it in the
	// future, we don't want to login every time when application
	// starts
	::lastfm::ws::SessionKey = lfm["session"]["key"].text();

	emit success(::lastfm::ws::Username, ::lastfm::ws::SessionKey);
}
