// MapLauncher fuer Harmattan.
//
// Auf Sailfish reicht diese Klasse Orte und ganze Fusswege ueber D-Bus an
// Pure Maps weiter. Auf diesem Geraet gibt es Pure Maps nicht -- wohl aber
// einen Umgang mit geo:-Adressen (/usr/share/contentaction/geo.xml), also
// bekommt die Karten-App den Punkt und sonst nichts. Ein Fussweg als ganze
// Linie laesst sich nicht uebergeben; die App sagt das, statt so zu tun, als
// haette sie es getan.
#include "maplauncher.h"

#include <QDesktopServices>
#include <QString>
#include <QUrl>

MapLauncher::MapLauncher(QObject *parent)
    : QObject(parent)
{
}

bool MapLauncher::pureMapsInstalled() const
{
    return false;
}

QString MapLauncher::gpxPath() const
{
    // Kein GPX-Router auf diesem Geraet, also auch keine Datei dafuer.
    return QString();
}

void MapLauncher::setMessage(const QString &message)
{
    if (m_lastMessage != message) {
        m_lastMessage = message;
        emit lastMessageChanged();
    }
}

void MapLauncher::openGeoUri(double latitude, double longitude)
{
    const QString uri = QString::fromLatin1("geo:%1,%2")
                            .arg(latitude, 0, 'f', 6)
                            .arg(longitude, 0, 'f', 6);
    if (!QDesktopServices::openUrl(QUrl(uri))) {
        setMessage(QString::fromUtf8("Keine Karten-App für geo:-Adressen gefunden."));
    }
}

void MapLauncher::callShowPoi(const QString &, double latitude, double longitude)
{
    openGeoUri(latitude, longitude);
}

bool MapLauncher::writeGpx(const QString &, const QString &, const QVariantList &)
{
    return false;
}

void MapLauncher::showLocation(const QString &title, double latitude, double longitude)
{
    Q_UNUSED(title)
    openGeoUri(latitude, longitude);
}

void MapLauncher::showRoute(const QString &, double, double,
                            const QString &toName, double toLat, double toLon,
                            const QVariantList &)
{
    Q_UNUSED(toName)
    setMessage(QString::fromUtf8("Auf diesem Gerät wird nur das Ziel übergeben, "
                                 "nicht der ganze Fußweg."));
    openGeoUri(toLat, toLon);
}
