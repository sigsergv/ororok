#ifndef NETWORKACCESSMANAGER_H
#define NETWORKACCESSMANAGER_H

#include <QNetworkAccessManager>

namespace Ororok
{
namespace lastfm
{

class NetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT
public:
    explicit NetworkAccessManager(QObject *parent = 0);
    ~NetworkAccessManager();

protected:
    QNetworkReply * createRequest(Operation op, const QNetworkRequest & req,
                                  QIODevice * outgoingData = 0);
};

}
}

#endif // NETWORKACCESSMANAGER_H
