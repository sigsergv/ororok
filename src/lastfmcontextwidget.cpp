/*
 * lastfmcontextwidget.cpp
 *
 *  Created on: May 8, 2010
 *      Author: Sergei Stolyarov
 */

#include <QtGui>
#include <QtDebug>
#include <QtWebKit>
#include <lastfm.h>

#include "lastfmcontextwidget.h"
#include "mimetrackinfo.h"
#include "services/lastfm/lastfm.h"

struct LastfmContextWidget::Private
{
	QWebView * webview;
	QNetworkReply * trackSearchReply;

	QString currentArtist;
	QString currentTrack;
	QString currentAlbum;
};

LastfmContextWidget::LastfmContextWidget(QWidget * parent, Qt::WindowFlags f)
	: QWidget(parent, f)
{
	p = new Private;
	p->webview = new QWebView(this);
	p->trackSearchReply = 0;

	// create layout
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(p->webview);
	//view->show();
	this->setLayout(layout);
	p->webview->setHtml("");
}

LastfmContextWidget::~LastfmContextWidget()
{
	delete p;
}

void LastfmContextWidget::playerTrackStarted(const QStringList & trackInfo)
{
	// fetch info from lastfm about playing track: artist, tags etc
	QMap<QString, QString> map;
	map["method"] = "track.getInfo";
	map["limit"] = "1";
	map["track"] = trackInfo[Ororok::TrackFieldTitle];
	map["artist"] = trackInfo[Ororok::TrackFieldArtist];
	if (!::lastfm::ws::Username.isEmpty()) {
		map["username"] = ::lastfm::ws::Username;
	}
	p->webview->setHtml(tr("Searching…"));
	p->trackSearchReply = lastfm::ws::get(map);
	connect(p->trackSearchReply, SIGNAL(finished()), this, SLOT(trackGetInfoRequestFinished()));
}

void LastfmContextWidget::trackGetInfoRequestFinished()
{
	qDebug() << "track search finished";
	if (p->trackSearchReply) {
		Ororok::lastfm::Response lfr = Ororok::lastfm::parseReply(p->trackSearchReply);
		if (lfr.error()) {
			return;
		}

		const ::lastfm::XmlQuery lfm(lfr.data);

		p->currentArtist = lfm["track"]["artist"]["name"].text();
		p->currentTrack = lfm["track"]["name"].text();
		QString trackUrl = lfm["track"]["url"].text();
		QString artistUrl = lfm["track"]["artist"]["url"].text();

		// get tags
		//QDomElement te = lfm["track"]["artist"]["toptags"];
		QStringList tag_urls;
		foreach (::lastfm::XmlQuery x, lfm["track"]["toptags"].children("tag")) {
			tag_urls << QString("<a href=\"%1\">%2</a>")
					.arg(x["url"].text())
					.arg(x["name"].text());
		}

		QString tags_html;
		if (tag_urls.length() > 0) {
			tags_html = QString("<!--top tags-->Top tags: %1").arg(tag_urls.join(", "));
		}

		QStringList duration_parts;
		int duration = lfm["track"]["duration"].text().toInt() / 1000;
		duration_parts << QString("%1").arg(duration%60, 2, 10, QLatin1Char('0'));
		duration /= 60;
		duration_parts.insert(0, QString("%1").arg(duration, 2, 10, QLatin1Char('0')));

		// obtain playing count data
		QString playcount_info = QString(tr("Played <strong>%1</strong> times "))
				.arg(lfm["track"]["playcount"].text().toInt());
		int user_playcount = lfm["track"]["userplaycount"].text().toInt();
		if (user_playcount > 0) {
			playcount_info += QString("(<strong>%1</strong> times by you)")
					.arg(user_playcount);
		}

		int listeners = lfm["track"]["listeners"].text().toInt();

		QString loved_info;
		if (lfm["track"]["userloved"].text() == "1") {
			loved_info = tr("You love this track.");
		}

		p->webview->setHtml(tr("<div><!--artist--><strong>%1</strong> — <!--track--><strong>%2</strong> (<!--duration-->%3)</div>"
				"<div><!--tags list-->%4</div>"
				"<div><!--played count-->%5</div>"
				"<div><!--listeners count--><strong>%6</strong> listeners</div>"
				"<div><!--loved info-->%7</div>")
				.arg(QString("<a href=\"http://www.last.fm/music/%1\">%2</a>")
						.arg(artistUrl)
						.arg(p->currentArtist))
				.arg(QString("<a href=\"%1\">%2</a>")
						.arg(trackUrl)
						.arg(p->currentTrack))
				.arg(duration_parts.join(":"))
				.arg(tags_html)
				.arg(playcount_info)
				.arg(listeners)
				.arg(loved_info));
	}
	p->trackSearchReply = 0;
}
