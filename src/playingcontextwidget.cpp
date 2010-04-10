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
#include "formats.h"

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
	QString filename = trackInfo[Ororok::TrackFieldPath];
	Ororok::MusicTrackMetadata * md = Ororok::getMusicFileMetadata(filename);
	if (md == 0) {
		p->webview->setHtml(tr("Unable to detect file metadata"));
		return;
	}

	QString songInfoHtml;

	//html += tr("<h1>Now Playing</h1>");
	songInfoHtml += tr("<div><!--song title--><strong>%1</strong></div>"
			"<div>by <!--artist name--><strong>%2</strong></div>")
			.arg(md->title).
			arg(md->artist);
	if (!md->album.isEmpty()) {
		bool bOk;
		int trackNum = trackInfo[Ororok::TrackFieldNo].toInt(&bOk);
		QString t;
		if (bOk && trackNum > 0) {
			t = tr("<!--track #--> / #<strong>%1</strong>")
					.arg(trackNum);
		}

		songInfoHtml += tr("<!--album--><div>on <strong>%1</strong>%2</div>")
				.arg(trackInfo[Ororok::TrackFieldAlbum])
				.arg(t);
	}

	// try to find cover image
	QString coverHtml;
	QDir albumDir = QFileInfo(filename).absoluteDir();
	QStringList coverFilter;
	coverFilter << QString("cover.jpg") << QString("cover.png") << QString("cover.gif") << QString("cover.bmp");
	QStringList covers = albumDir.entryList(coverFilter);
	if (covers.length() > 0) {
		QString cover = covers.at(0);
		coverHtml = QString("<img border=\"1\" width=\"150\" src=\"file://%1\">")
				.arg(albumDir.absoluteFilePath(cover));
	}

	QString html = tr("<table border=\"0\"><tr>"
			"<td><!--cover-->%1</div></td>"
			"<td></div><!--song info-->%2</div></td>"
			"</tr></table>")
			.arg(coverHtml)
			.arg(songInfoHtml);

	p->webview->setHtml(html);
}
