/*
 * playingcontextwidget.cpp
 *
 *  Created on: Apr 5, 2010
 *      Author: Sergey Stolyarov
 */
#include <QtGui>
#include <QtWidgets>
#include <QtWebKit>
#include <QtWebKitWidgets>
#include <QtDebug>

#include "playingcontextwidget.h"
#include "mimetrackinfo.h"
#include "formats.h"

struct PlayingContextWidget::Private
{
	QWebView * webview;
	QString pageTemplate;
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

	setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

	QFile templateFile(":/page-template.html");
	templateFile.open(QIODevice::ReadOnly);
	p->pageTemplate = QString(templateFile.readAll());

	p->webview->setHtml(p->pageTemplate.arg(tr("<i>Playing track info</i>")));
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
		p->webview->setHtml(p->pageTemplate.arg(tr("Unable to detect file metadata")));
		return;
	}

	QStringList htmlRows;

	// order of data fields as follows
	// {title} by {author}
	// on {album-name} / #{track-num}
	// Album date: {year}
	// Genre: {genre}
	// Bitrate: {bitrate}
	// Lyrics by: {lyrics}
	// Composed by: {composer}

	// title
	htmlRows << tr("<div><!--song title--><strong>%1</strong> "
			"by <!--artist name--><strong>%2</strong></div>")
			.arg(md->title)
			.arg(md->artist);

	// album
	QString albumName = tr("<em>Unknown album</em>");
	if (!md->album.isEmpty()) {
		albumName = md->album;
	}

	QString trackNum;
	if (md->track > 0) {
		trackNum = tr(" / #<strong>%1</strong>")
			.arg(md->track);
	}

	htmlRows << tr("<div>on <strong>%1</strong>%2</div>")
			.arg(albumName)
			.arg(trackNum);

	// cover
	QDir albumDir = QFileInfo(filename).absoluteDir();
	QStringList coverFilter;
	coverFilter << QString("cover.jpg") << QString("cover.png") << QString("cover.gif") << QString("cover.bmp");
	QStringList covers = albumDir.entryList(coverFilter);
	if (covers.length() > 0) {
		QString cover = covers.at(0);
		htmlRows << QString("<div class=\"cover\"><img border=\"1\" width=\"100%\" src=\"file://%1\"></div>")
				.arg(albumDir.absoluteFilePath(cover));
	}

	// album date
	if (md->year > 0) {
		htmlRows << tr("<div>Album year: <strong>%1</strong></div>")
			.arg(md->year);
	}

	// genre
	if (!md->genre.isEmpty()) {
		htmlRows << tr("<div>Genre: <strong>%1</strong></div>")
			.arg(md->genre);
	}

	// bitrate
	htmlRows << tr("<div>Bitrate: <strong>%1</strong> kbps</div>")
			.arg(md->bitrate);

	if (!md->composer.isEmpty()) {
		htmlRows << tr("<div>Composed by <strong>%1</strong></div>")
				.arg(md->composer);
	}
	if (!md->lyricsAuthor.isEmpty()) {
		htmlRows << tr("<div>Lyrics by <strong>%1</strong></div>")
				.arg(md->lyricsAuthor);
	}

	QString html = p->pageTemplate.arg(htmlRows.join("\n"));
	p->webview->setHtml(html);
}
