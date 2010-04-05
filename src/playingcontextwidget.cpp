/*
 * playingcontextwidget.cpp
 *
 *  Created on: Apr 5, 2010
 *      Author: Sergei Stolyarov
 */
#include <QtGui>
#include <QtDebug>
 #include <QtWebKit>

#include "playingcontextwidget.h"
#include "mimetrackinfo.h"

struct PlayingContextWidget::Private
{
	QWebView * webview;
};

PlayingContextWidget::PlayingContextWidget(QWidget * parent, Qt::WindowFlags f)
	: QWidget(parent, f)
{
	p = new Private;

	p->webview = new QWebView(this);

	// create layout
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(p->webview);
	//view->show();
	this->setLayout(layout);
	p->webview->setHtml(tr("<i>Playing track info</i>")); // init with empty text
}

PlayingContextWidget::~PlayingContextWidget()
{
	delete p;
}

void PlayingContextWidget::playerTrackStarted(const QStringList & trackInfo)
{
	QString html;

	//html += tr("<h1>Now Playing</h1>");
	html += tr("<!--song title--><strong>%1</strong> by <!--artist name--><strong>%2</strong>")
			.arg(trackInfo[Ororok::TrackFieldTitle]).
			arg(trackInfo[Ororok::TrackFieldArtist]);
	if (!trackInfo[Ororok::TrackFieldAlbum].isEmpty()) {
		html += tr("<!--album--> from album “<strong>%1</strong>”")
				.arg(trackInfo[Ororok::TrackFieldAlbum]);
		bool bOk;
		int trackNum = trackInfo[Ororok::TrackFieldNo].toInt(&bOk);
		if (bOk && trackNum > 0) {
			html += tr("<!--track #--> (track #<strong>%1</strong>)")
					.arg(trackNum);
		}
	}
	p->webview->setHtml(html);
}
