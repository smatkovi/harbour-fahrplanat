#include "networkfactory.h"

#include <QDir>
#include <QNetworkDiskCache>
#include <QNetworkRequest>
#include <QStandardPaths>

TileNetworkAccessManager::TileNetworkAccessManager(QObject *parent)
    : QNetworkAccessManager(parent)
{
}

QNetworkReply *TileNetworkAccessManager::createRequest(Operation op, const QNetworkRequest &request,
                                                       QIODevice *outgoingData)
{
    QNetworkRequest r(request);
    r.setRawHeader("User-Agent", "harbour-fahrplanat/" APP_VERSION
                                 " (Sailfish OS; https://github.com/smatkovi/harbour-fahrplanat)");
    r.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
    r.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    return QNetworkAccessManager::createRequest(op, r, outgoingData);
}

QNetworkAccessManager *NetworkFactory::create(QObject *parent)
{
    TileNetworkAccessManager *nam = new TileNetworkAccessManager(parent);
    QNetworkDiskCache *cache = new QNetworkDiskCache(nam);
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QStringLiteral("/tiles");
    QDir().mkpath(dir);
    cache->setCacheDirectory(dir);
    cache->setMaximumCacheSize(64 * 1024 * 1024);
    nam->setCache(cache);
    return nam;
}
