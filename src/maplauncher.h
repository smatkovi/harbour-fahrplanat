#ifndef MAPLAUNCHER_H
#define MAPLAUNCHER_H

#include <QObject>
#include <QString>
#include <QVariantList>

// Hands locations and walking routes over to a map application.
//
// Pure Maps is addressed via its D-Bus interface (io.github.rinigus.PureMaps,
// auto-started by D-Bus activation). A complete route is passed with the
// ShowRoute method that the patch in contrib/ adds to Pure Maps; without that
// patch the route is written as GPX (~/Downloads) for Pure Maps' GPX router and only the
// destination is shown. Any other geo: handler is used as last resort.
class MapLauncher : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool pureMapsInstalled READ pureMapsInstalled CONSTANT)
    Q_PROPERTY(QString lastMessage READ lastMessage NOTIFY lastMessageChanged)
    Q_PROPERTY(QString gpxPath READ gpxPath CONSTANT)

public:
    explicit MapLauncher(QObject *parent = 0);

    bool pureMapsInstalled() const;
    QString lastMessage() const { return m_lastMessage; }
    QString gpxPath() const;

    Q_INVOKABLE void showLocation(const QString &title, double latitude, double longitude);

    // points: list of {lat, lon} maps in walking order (may be empty -> straight line)
    Q_INVOKABLE void showRoute(const QString &fromName, double fromLat, double fromLon,
                               const QString &toName, double toLat, double toLon,
                               const QVariantList &points);

signals:
    void lastMessageChanged();

private:
    void setMessage(const QString &message);
    void openGeoUri(double latitude, double longitude);
    void callShowPoi(const QString &title, double latitude, double longitude);
    bool writeGpx(const QString &fromName, const QString &toName, const QVariantList &points);

    QString m_lastMessage;
};

#endif // MAPLAUNCHER_H
