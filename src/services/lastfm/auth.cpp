/*
 * auth.cpp
 *
 *  Created on: Mar 31, 2010
 *      Author: Sergei Stolyarov
 */
#include <QtDebug>
#include <QString>
#include <QNetworkReply>

#include "lastfm.h"
#include "auth.h"

struct ororok::lastfm::Auth::Private
{
	QNetworkReply * lastfmReply;
};

ororok::lastfm::Auth::Auth(QObject * parent)
	: QObject(parent)
{
	p = new Private;
}

ororok::lastfm::Auth::~Auth()
{
	delete p;
}

void ororok::lastfm::Auth::authenticate(const QString & username, const QString & password)
{
	::lastfm::ws::Username = username;

	QMap<QString, QString> params;
	params["method"] = "auth.getMobileSession";
	params["username"] = username;
	params["authToken"] = ::lastfm::md5( (username + ::lastfm::md5(password.toUtf8()) ).toUtf8() );

	p->lastfmReply = ::lastfm::ws::post(params);

	connect(p->lastfmReply, SIGNAL(finished()), this, SLOT(lastfmQueryFinished()));
}

void ororok::lastfm::Auth::reset()
{
	::lastfm::ws::SessionKey.clear();
	::lastfm::ws::Username.clear();
}


void ororok::lastfm::Auth::lastfmQueryFinished()
{
	ororok::lastfm::Response lfr(ororok::lastfm::parseReply(p->lastfmReply));
	if (lfr.error()) {
		// cleanup username and session data
		reset();
		emit failed(lfr.error(), lfr.errorText());
		return;
	}

	const ::lastfm::XmlQuery lfm(lfr.data);
	// replace username, just in case
	::lastfm::ws::Username = lfm["session"]["name"].text();
	// remember session key, it's important to re-use it in the
	// future, we don't want to login every time when application
	// starts
	::lastfm::ws::SessionKey = lfm["session"]["key"].text();

	emit success(::lastfm::ws::Username, ::lastfm::ws::SessionKey);
}
