#include <QtDebug>
#include <QNetworkRequest>

#include "networkaccessmanager.h"


Ororok::lastfm::NetworkAccessManager::NetworkAccessManager(QObject *parent) :
    QNetworkAccessManager(parent)
{
}

Ororok::lastfm::NetworkAccessManager::~NetworkAccessManager()
{

}

QNetworkReply * Ororok::lastfm::NetworkAccessManager::createRequest(Operation op,
                              const QNetworkRequest & req,
                              QIODevice * outgoingData)
{
    QNetworkRequest newRequest(req);

    QUrl url = req.url();

    // force url scheme to https, another workaround
    if (url.host() == "ws.audioscrobbler.com") {
        url.setScheme("https");
        url.setPort(-1);
    }


    newRequest.setUrl(url);

    return QNetworkAccessManager::createRequest(op, newRequest, outgoingData);
}
