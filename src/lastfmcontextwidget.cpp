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
	QString webviewHtml;
	QNetworkReply * trackGetInfoReply;
	QNetworkReply * artistGetInfoReply;

	QString currentArtist;
	QString currentTrack;
	QString currentAlbum;
};

LastfmContextWidget::LastfmContextWidget(QWidget * parent, Qt::WindowFlags f)
	: QWidget(parent, f)
{
	p = new Private;
	p->webview = new QWebView(this);
	p->webview->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
	connect( p->webview->page(), SIGNAL(linkClicked(const QUrl &)),
			this, SLOT(linkClicked(const QUrl &)));
	p->trackGetInfoReply = 0;

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
	p->trackGetInfoReply = lastfm::ws::get(map);
	connect(p->trackGetInfoReply, SIGNAL(finished()), this, SLOT(trackGetInfoRequestFinished()));
}

void LastfmContextWidget::trackGetInfoRequestFinished()
{
	qDebug() << "track search finished";
	if (!p->trackGetInfoReply) {
		return;
	}

	Ororok::lastfm::Response lfr = Ororok::lastfm::parseReply(p->trackGetInfoReply);
	if (lfr.error()) {
		return;
	}

	const ::lastfm::XmlQuery lfm(lfr.data);

	p->currentArtist = lfm["track"]["artist"]["name"].text();
	p->currentTrack = lfm["track"]["name"].text();
	QString trackUrl = lfm["track"]["url"].text();
	QString artistUrl = lfm["track"]["artist"]["url"].text();

	// get tags
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
	QString playcount_info = QString(tr("Played <strong>%1</strong> times"))
			.arg(lfm["track"]["playcount"].text().toInt());
	int user_playcount = lfm["track"]["userplaycount"].text().toInt();
	if (user_playcount > 0) {
		playcount_info += QString(" (<strong>%1</strong> times by you)")
				.arg(user_playcount);
	}

	int listeners = lfm["track"]["listeners"].text().toInt();

	QString loved_info;
	if (lfm["track"]["userloved"].text() == "1") {
		loved_info = tr("You love this track.");
	}

	p->webviewHtml = tr("<div><!--artist--><strong>%1</strong> — <!--track--><strong>%2</strong> (<!--duration-->%3)</div>"
			"<div><!--tags list-->%4</div>"
			"<div>"
			"<!--played count-->%5"
			"<!--listeners count-->, <strong>%6</strong> listeners"
			"</div>"
			"<div><!--loved info-->%7</div>")
			.arg(QString("<a href=\"%1\">%2</a>")
					.arg(artistUrl)
					.arg(p->currentArtist))
			.arg(QString("<a href=\"%1\">%2</a>")
					.arg(trackUrl)
					.arg(p->currentTrack))
			.arg(duration_parts.join(":"))
			.arg(tags_html)
			.arg(playcount_info)
			.arg(listeners)
			.arg(loved_info);

	p->webview->setHtml(p->webviewHtml + tr("<div>Loading artist info…</div>"));

	p->trackGetInfoReply = 0;
	disconnect(this, SLOT(trackGetInfoRequestFinished()));

	// send artist info request
	QMap<QString, QString> map;
	map["method"] = "artist.getTopTags";
	map["artist"] = p->currentArtist;
	p->artistGetInfoReply = lastfm::ws::get(map);
	connect(p->artistGetInfoReply, SIGNAL(finished()), this, SLOT(artistGetInfoRequestFinished()));
}

void LastfmContextWidget::artistGetInfoRequestFinished()
{
	if (!p->artistGetInfoReply) {
		return;
	}

	Ororok::lastfm::Response lfr = Ororok::lastfm::parseReply(p->artistGetInfoReply);
	if (lfr.error()) {
		return;
	}

	const ::lastfm::XmlQuery lfm(lfr.data);

	QStringList tag_urls;
	foreach (::lastfm::XmlQuery x, lfm["toptags"].children("tag")) {
		tag_urls << QString("<a href=\"%1\">%2</a>")
				.arg(x["url"].text())
				.arg(x["name"].text());
	}

	QString tags_html;
	if (tag_urls.length() > 0) {
		tags_html = QString("<!--top artist tags-->Artist top tags: %1").arg(tag_urls.join(", "));
	}

	p->webviewHtml += tr("<!--artist info--><hr width=\"80%\">"
			"<div>%1</div>")
			.arg(tags_html);

	p->webview->setHtml(p->webviewHtml);
	p->artistGetInfoReply = 0;
	disconnect(this, SLOT(artistGetInfoRequestFinished()));
}

void LastfmContextWidget::linkClicked(const QUrl & url)
{
	// open url in the external browser
	QDesktopServices::openUrl(url);
}
