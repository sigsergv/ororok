/*
 * lastfm.h
 *
 *  Created on: Mar 29, 2010
 *      Author: Sergei Stolyarov
 */

#ifndef LASTFM_H_
#define LASTFM_H_

#include <QByteArray>

#include <lastfm/ws.h>
#include <lastfm/misc.h>
#include <lastfm/XmlQuery>

namespace ororok
{

	struct LastfmResponse
	{
		QByteArray data;

		LastfmResponse(const QByteArray & reply);
		int error();
		QString errorText();
		void setError(const QString & text, int code=0);

	protected:
		int errCode;
		QString errText;
	};

	LastfmResponse parseLastfmReply(QNetworkReply * reply);

	/**
	 * load session details from the settings and initialize lastfm-related data
	 */
	void initLastfm();
}

#endif /* LASTFM_H_ */
