#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QObject>
#include <QSettings>
#include <QString>
#include <QVariantMap>

// Persistent settings (~/.config/harbour-fahrplanat/harbour-fahrplanat.conf).
// The HAFAS backend profile is fully configurable so that a change on the
// server side can be handled without a new build.
class AppSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int preset READ preset WRITE setPreset NOTIFY profileChanged)
    Q_PROPERTY(QString endpoint READ endpoint WRITE setEndpoint NOTIFY profileChanged)
    Q_PROPERTY(QString version READ version WRITE setVersion NOTIFY profileChanged)
    Q_PROPERTY(QString ext READ ext WRITE setExt NOTIFY profileChanged)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY profileChanged)
    Q_PROPERTY(QString clientType READ clientType WRITE setClientType NOTIFY profileChanged)
    Q_PROPERTY(QString clientId READ clientId WRITE setClientId NOTIFY profileChanged)
    Q_PROPERTY(QString clientVersion READ clientVersion WRITE setClientVersion NOTIFY profileChanged)
    Q_PROPERTY(QString clientName READ clientName WRITE setClientName NOTIFY profileChanged)
    Q_PROPERTY(QString aid READ aid WRITE setAid NOTIFY profileChanged)
    Q_PROPERTY(QString userAgent READ userAgent WRITE setUserAgent NOTIFY profileChanged)
    Q_PROPERTY(bool logRequests READ logRequests WRITE setLogRequests NOTIFY profileChanged)

    Q_PROPERTY(int products READ products WRITE setProducts NOTIFY searchOptionsChanged)
    Q_PROPERTY(bool directOnly READ directOnly WRITE setDirectOnly NOTIFY searchOptionsChanged)
    Q_PROPERTY(int minChangeTime READ minChangeTime WRITE setMinChangeTime NOTIFY searchOptionsChanged)
    Q_PROPERTY(bool wheelchair READ wheelchair WRITE setWheelchair NOTIFY searchOptionsChanged)
    Q_PROPERTY(bool bicycle READ bicycle WRITE setBicycle NOTIFY searchOptionsChanged)
    Q_PROPERTY(bool einfachRaus READ einfachRaus WRITE setEinfachRaus NOTIFY searchOptionsChanged)

    Q_PROPERTY(int colorTheme READ colorTheme WRITE setColorTheme NOTIFY uiChanged)

    Q_PROPERTY(QVariantMap lastFrom READ lastFrom WRITE setLastFrom NOTIFY lastLocationsChanged)
    Q_PROPERTY(QVariantMap lastTo READ lastTo WRITE setLastTo NOTIFY lastLocationsChanged)

public:
    explicit AppSettings(QObject *parent = 0);

    int preset() const;
    QString endpoint() const;
    QString version() const;
    QString ext() const;
    QString language() const;
    QString clientType() const;
    QString clientId() const;
    QString clientVersion() const;
    QString clientName() const;
    QString aid() const;
    QString userAgent() const;
    bool logRequests() const;

    int products() const;
    bool directOnly() const;
    int minChangeTime() const;
    bool wheelchair() const;
    bool bicycle() const;
    bool einfachRaus() const;

    int colorTheme() const;
    void setColorTheme(int theme);

    QVariantMap lastFrom() const;
    QVariantMap lastTo() const;

    void setPreset(int preset);
    void setEndpoint(const QString &v);
    void setVersion(const QString &v);
    void setExt(const QString &v);
    void setLanguage(const QString &v);
    void setClientType(const QString &v);
    void setClientId(const QString &v);
    void setClientVersion(const QString &v);
    void setClientName(const QString &v);
    void setAid(const QString &v);
    void setUserAgent(const QString &v);
    void setLogRequests(bool v);

    void setProducts(int v);
    void setDirectOnly(bool v);
    void setMinChangeTime(int v);
    void setWheelchair(bool v);
    void setBicycle(bool v);
    void setEinfachRaus(bool v);

    void setLastFrom(const QVariantMap &m);
    void setLastTo(const QVariantMap &m);

    // Re-applies the values of a preset (0 = Scotty Android 9.x, 1 = Öffi/pte, 2 = hafas-client legacy).
    Q_INVOKABLE void applyPreset(int preset);
    Q_INVOKABLE QString presetName(int preset) const;
    Q_INVOKABLE int presetCount() const;

    Q_INVOKABLE QString productGroupName(int index) const;
    Q_INVOKABLE int productGroupBits(int index) const;
    Q_INVOKABLE int productGroupCount() const;
    Q_INVOKABLE int allProducts() const;
    Q_INVOKABLE QString productsSummary(int bitmask) const;

signals:
    void profileChanged();
    void searchOptionsChanged();
    void lastLocationsChanged();
    void uiChanged();

private:
    QString str(const QString &key, const QString &def = QString()) const;
    void setStr(const QString &key, const QString &value);
    QVariantMap map(const QString &key) const;
    void setMap(const QString &key, const QVariantMap &m);

    QSettings m_settings;
};

#endif // APPSETTINGS_H
