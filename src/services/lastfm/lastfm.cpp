/*
 * lastfm.cpp
 *
 *  Created on: Mar 29, 2010
 *      Author: Sergei Stolyarov
 */

#include <QtDebug>
#include <QCoreApplication>

#include <lastfm/misc.h>
#include <lastfm/ws.h>

#include "lastfm.h"
#include "networkaccessmanager.h"
#include <QSettings>
#include "settings.h"

#define LASTFM_API_KEY "64a0df9f82da53a7dca15b22d1ad3044"
#define LASTFM_API_SECRET "153bce72f00a2bec1d124e597d69fb5b"

const char * lastfm::ws::ApiKey = LASTFM_API_KEY;
const char * lastfm::ws::SharedSecret = LASTFM_API_SECRET;

QString lastfm::ws::Username;

static bool lastfm_submit_enabled;
static bool lastfm_lookup_enabled;

bool Ororok::lastfm::isSubmitEnabled()
{
	return ::lastfm_submit_enabled;
}

bool Ororok::lastfm::isLookupEnabled()
{
	return ::lastfm_lookup_enabled;
}

Ororok::lastfm::Response::Response(const QByteArray & reply)
	: data(reply), errCode(0)
{}

int Ororok::lastfm::Response::error()
{
	return errCode;
}

QString Ororok::lastfm::Response::errorText()
{
	return errText;
}

void Ororok::lastfm::Response::setError(const QString & text, int code)
{
	errCode = code;
	errText = text;
}

Ororok::lastfm::Response Ororok::lastfm::parseReply(QNetworkReply * reply)
{
	Ororok::lastfm::Response lfr(reply->readAll());
	if (!lfr.data.size()) {
		lfr.setError("Malformed response");
		return lfr;
	}

	//qDebug() << "XML Response:" << lfr.data;

	QDomDocument xml;
	xml.setContent(lfr.data);
	QDomElement lfm = xml.documentElement();

	if (lfm.isNull()) {
		lfr.setError("Malformed XML response");
		return lfr;
	}

	QString const status = lfm.attribute("status");
	QDomElement error = lfm.firstChildElement("error");
	//uint const n = lfm.childNodes().count();

	// no elements beyond the lfm is perfectably acceptable <-- wtf?
	// if (n == 0) // nothing useful in the response
	if (status == "failed"/* || ( n == 1 && !error.isNull())*/ ) {
		lfr.setError(error.text(), error.attribute("code").toInt());
		return lfr;
	}

	switch (reply->error())	{
	case QNetworkReply::RemoteHostClosedError:
	case QNetworkReply::ConnectionRefusedError:
	case QNetworkReply::TimeoutError:
	case QNetworkReply::ContentAccessDenied:
	case QNetworkReply::ContentOperationNotPermittedError:
	case QNetworkReply::UnknownContentError:
	case QNetworkReply::ProtocolInvalidOperationError:
	case QNetworkReply::ProtocolFailure:
		lfr.setError(QString("Network error: %1").arg(reply->error()), 11111);
		return lfr;
		break;

	case QNetworkReply::NoError:
	default:
		break;
	}


	return lfr;
}

void Ororok::initLastfm()
{
	QStringList args = QCoreApplication::arguments();

	::lastfm_submit_enabled = -1 == args.indexOf(QString("--disable-lastfm-submit"));
	::lastfm_lookup_enabled = -1 == args.indexOf(QString("--disable-lastfm-lookup"));

    Ororok::lastfm::NetworkAccessManager * m = new Ororok::lastfm::NetworkAccessManager();
    ::lastfm::setNetworkAccessManager(m);

	QSettings * settings = Ororok::settings();
	settings->beginGroup("LastFm");
	QString sessionKey = settings->value("sessionKey").toString();
	QString username = settings->value("username").toString();
	settings->endGroup();

	if (!sessionKey.isEmpty() && !username.isEmpty()) {
		::lastfm::ws::Username = username;
		::lastfm::ws::SessionKey = sessionKey;
		//qDebug() << "Last.fm initialized for user" << ::lastfm::ws::Username;
	} else {
		::lastfm::ws::SessionKey.clear();
		::lastfm::ws::Username.clear();
	}
}
