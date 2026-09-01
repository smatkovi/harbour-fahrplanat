#ifndef NETWORKFACTORY_H
#define NETWORKFACTORY_H

#include <QNetworkAccessManager>
#include <QQmlNetworkAccessManagerFactory>

// Network access manager for QML (map tiles): identifies the app towards the
// tile server as its usage policy requires, and keeps a disk cache so tiles
// are fetched once.
class TileNetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT
public:
    explicit TileNetworkAccessManager(QObject *parent = 0);

protected:
    QNetworkReply *createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData = 0);
};

class NetworkFactory : public QQmlNetworkAccessManagerFactory
{
public:
    QNetworkAccessManager *create(QObject *parent);
};

#endif // NETWORKFACTORY_H
