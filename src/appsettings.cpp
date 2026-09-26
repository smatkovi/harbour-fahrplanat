#include "appsettings.h"
#include "hafastypes.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace
{
    struct Preset
    {
        const char *name;
        const char *endpoint;
        const char *version;
        const char *ext;
        const char *language;
        const char *clientType;
        const char *clientId;
        const char *clientVersion;
        const char *clientName;
        const char *aid;
        const char *userAgent;
    };

    // 0: parameters of the current Android app (HaCon HCI configuration)
    // 1: parameters used by Öffi / public-transport-enabler
    // 2: legacy mgate.exe endpoint used by hafas-client
    const Preset presets[] = {
        { "ÖBB App (gate, HCI 1.77, OEBB.14)",
          "https://fahrplan.oebb.at/gate", "1.77", "OEBB.14", "deu",
          "AND", "OEBB", "72", "", "OWDL4fE4ixNiPBBm",
          "Dalvik/2.1.0 (Linux; U; Android 14)" },
        { "Öffi / pte (gate, HCI 1.88, OEBB.14)",
          "https://fahrplan.oebb.at/gate", "1.88", "OEBB.14", "deu",
          "AND", "OEBB", "", "", "OWDL4fE4ixNiPBBm",
          "Dalvik/2.1.0 (Linux; U; Android 14)" },
        { "hafas-client (mgate.exe, HCI 1.45)",
          "https://fahrplan.oebb.at/bin/mgate.exe", "1.45", "", "de",
          "IPH", "OEBB", "6030600", "oebbPROD-ADHOC", "OWDL4fE4ixNiPBBm",
          "Mozilla/5.0 (iPhone; CPU iPhone OS 17_0 like Mac OS X)" },
    };
    const int kPresetCount = sizeof(presets) / sizeof(presets[0]);
}

AppSettings::AppSettings(QObject *parent)
    : QObject(parent)
    , m_settings(QStringLiteral("harbour-fahrplanat"), QStringLiteral("harbour-fahrplanat"))
{
    if (!m_settings.contains(QStringLiteral("profile/endpoint"))) {
        applyPreset(0);
    }
}

// ---------------------------------------------------------------- helpers

QString AppSettings::str(const QString &key, const QString &def) const
{
    return m_settings.value(key, def).toString();
}

void AppSettings::setStr(const QString &key, const QString &value)
{
    if (m_settings.value(key).toString() == value) {
        return;
    }
    m_settings.setValue(key, value);
    if (key.startsWith(QLatin1String("profile/"))) {
        emit profileChanged();
    }
}

QVariantMap AppSettings::map(const QString &key) const
{
    const QByteArray raw = m_settings.value(key).toByteArray();
    if (raw.isEmpty()) {
        return QVariantMap();
    }
    return QJsonDocument::fromJson(raw).object().toVariantMap();
}

void AppSettings::setMap(const QString &key, const QVariantMap &m)
{
    m_settings.setValue(key, QJsonDocument(QJsonObject::fromVariantMap(m)).toJson(QJsonDocument::Compact));
}

// ---------------------------------------------------------------- profile

int AppSettings::preset() const { return m_settings.value(QStringLiteral("profile/preset"), 0).toInt(); }
QString AppSettings::endpoint() const { return str(QStringLiteral("profile/endpoint")); }
QString AppSettings::version() const { return str(QStringLiteral("profile/version")); }
QString AppSettings::ext() const { return str(QStringLiteral("profile/ext")); }
QString AppSettings::language() const { return str(QStringLiteral("profile/language"), QStringLiteral("deu")); }
QString AppSettings::clientType() const { return str(QStringLiteral("profile/clientType")); }
QString AppSettings::clientId() const { return str(QStringLiteral("profile/clientId")); }
QString AppSettings::clientVersion() const { return str(QStringLiteral("profile/clientVersion")); }
QString AppSettings::clientName() const { return str(QStringLiteral("profile/clientName")); }
QString AppSettings::aid() const { return str(QStringLiteral("profile/aid")); }
QString AppSettings::userAgent() const { return str(QStringLiteral("profile/userAgent")); }
bool AppSettings::logRequests() const { return m_settings.value(QStringLiteral("profile/logRequests"), false).toBool(); }

void AppSettings::setPreset(int preset)
{
    if (this->preset() == preset) {
        return;
    }
    m_settings.setValue(QStringLiteral("profile/preset"), preset);
    emit profileChanged();
}
void AppSettings::setEndpoint(const QString &v) { setStr(QStringLiteral("profile/endpoint"), v); }
void AppSettings::setVersion(const QString &v) { setStr(QStringLiteral("profile/version"), v); }
void AppSettings::setExt(const QString &v) { setStr(QStringLiteral("profile/ext"), v); }
void AppSettings::setLanguage(const QString &v) { setStr(QStringLiteral("profile/language"), v); }
void AppSettings::setClientType(const QString &v) { setStr(QStringLiteral("profile/clientType"), v); }
void AppSettings::setClientId(const QString &v) { setStr(QStringLiteral("profile/clientId"), v); }
void AppSettings::setClientVersion(const QString &v) { setStr(QStringLiteral("profile/clientVersion"), v); }
void AppSettings::setClientName(const QString &v) { setStr(QStringLiteral("profile/clientName"), v); }
void AppSettings::setAid(const QString &v) { setStr(QStringLiteral("profile/aid"), v); }
void AppSettings::setUserAgent(const QString &v) { setStr(QStringLiteral("profile/userAgent"), v); }
void AppSettings::setLogRequests(bool v)
{
    if (logRequests() == v) {
        return;
    }
    m_settings.setValue(QStringLiteral("profile/logRequests"), v);
    emit profileChanged();
}

void AppSettings::applyPreset(int preset)
{
    if (preset < 0 || preset >= kPresetCount) {
        preset = 0;
    }
    const Preset &p = presets[preset];
    m_settings.setValue(QStringLiteral("profile/preset"), preset);
    m_settings.setValue(QStringLiteral("profile/endpoint"), QString::fromUtf8(p.endpoint));
    m_settings.setValue(QStringLiteral("profile/version"), QString::fromUtf8(p.version));
    m_settings.setValue(QStringLiteral("profile/ext"), QString::fromUtf8(p.ext));
    m_settings.setValue(QStringLiteral("profile/language"), QString::fromUtf8(p.language));
    m_settings.setValue(QStringLiteral("profile/clientType"), QString::fromUtf8(p.clientType));
    m_settings.setValue(QStringLiteral("profile/clientId"), QString::fromUtf8(p.clientId));
    m_settings.setValue(QStringLiteral("profile/clientVersion"), QString::fromUtf8(p.clientVersion));
    m_settings.setValue(QStringLiteral("profile/clientName"), QString::fromUtf8(p.clientName));
    m_settings.setValue(QStringLiteral("profile/aid"), QString::fromUtf8(p.aid));
    m_settings.setValue(QStringLiteral("profile/userAgent"), QString::fromUtf8(p.userAgent));
    m_settings.sync();
    emit profileChanged();
}

int AppSettings::presetCount() const
{
    return kPresetCount;
}

QString AppSettings::presetName(int preset) const
{
    if (preset < 0 || preset >= kPresetCount) {
        return QString();
    }
    return QString::fromUtf8(presets[preset].name);
}

// ---------------------------------------------------------------- search options

int AppSettings::products() const
{
    return m_settings.value(QStringLiteral("search/products"), HafasProducts::all).toInt();
}
bool AppSettings::directOnly() const { return m_settings.value(QStringLiteral("search/directOnly"), false).toBool(); }
int AppSettings::minChangeTime() const { return m_settings.value(QStringLiteral("search/minChangeTime"), 0).toInt(); }
bool AppSettings::wheelchair() const { return m_settings.value(QStringLiteral("search/wheelchair"), false).toBool(); }
bool AppSettings::bicycle() const { return m_settings.value(QStringLiteral("search/bicycle"), false).toBool(); }
bool AppSettings::einfachRaus() const { return m_settings.value(QStringLiteral("search/einfachRaus"), false).toBool(); }

void AppSettings::setProducts(int v)
{
    if (products() == v) {
        return;
    }
    m_settings.setValue(QStringLiteral("search/products"), v);
    emit searchOptionsChanged();
}
void AppSettings::setDirectOnly(bool v)
{
    if (directOnly() == v) {
        return;
    }
    m_settings.setValue(QStringLiteral("search/directOnly"), v);
    emit searchOptionsChanged();
}
void AppSettings::setMinChangeTime(int v)
{
    if (minChangeTime() == v) {
        return;
    }
    m_settings.setValue(QStringLiteral("search/minChangeTime"), v);
    emit searchOptionsChanged();
}
void AppSettings::setWheelchair(bool v)
{
    if (wheelchair() == v) {
        return;
    }
    m_settings.setValue(QStringLiteral("search/wheelchair"), v);
    emit searchOptionsChanged();
}
void AppSettings::setBicycle(bool v)
{
    if (bicycle() == v) {
        return;
    }
    m_settings.setValue(QStringLiteral("search/bicycle"), v);
    emit searchOptionsChanged();
}
void AppSettings::setEinfachRaus(bool v)
{
    if (einfachRaus() == v) {
        return;
    }
    m_settings.setValue(QStringLiteral("search/einfachRaus"), v);
    emit searchOptionsChanged();
}

// ---------------------------------------------------------------- ui

int AppSettings::colorTheme() const
{
    // 1 = rot auf hellem Grund, 2 = rot auf schwarzem, 0 = Ambiente.
    // Auf Harmattan ist die Oberflaeche durchgehend dunkel (theme.inverted),
    // und der Bildschirm ist ein OLED: dort ist Schwarz die Voreinstellung.
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    const int vorgabe = 2;
#else
    const int vorgabe = 1;
#endif
    return m_settings.value(QStringLiteral("ui/colorTheme"), vorgabe).toInt();
}

void AppSettings::setColorTheme(int theme)
{
    if (colorTheme() == theme) {
        return;
    }
    m_settings.setValue(QStringLiteral("ui/colorTheme"), theme);
    emit uiChanged();
}

// ---------------------------------------------------------------- last locations

QVariantMap AppSettings::lastFrom() const { return map(QStringLiteral("last/from")); }
QVariantMap AppSettings::lastTo() const { return map(QStringLiteral("last/to")); }

void AppSettings::setLastFrom(const QVariantMap &m)
{
    setMap(QStringLiteral("last/from"), m);
    emit lastLocationsChanged();
}

void AppSettings::setLastTo(const QVariantMap &m)
{
    setMap(QStringLiteral("last/to"), m);
    emit lastLocationsChanged();
}

// ---------------------------------------------------------------- product groups

QString AppSettings::productGroupName(int index) const
{
    if (index < 0 || index >= HafasProducts::groupCount) {
        return QString();
    }
    return QString::fromUtf8(HafasProducts::groups[index].name);
}

int AppSettings::productGroupBits(int index) const
{
    if (index < 0 || index >= HafasProducts::groupCount) {
        return 0;
    }
    return HafasProducts::groups[index].bitmask;
}

int AppSettings::productGroupCount() const { return HafasProducts::groupCount; }
int AppSettings::allProducts() const { return HafasProducts::all; }

QString AppSettings::productsSummary(int bitmask) const
{
    if ((bitmask & HafasProducts::all) == HafasProducts::all) {
        return QStringLiteral("Alle Verkehrsmittel");
    }
    if (bitmask == 0) {
        return QStringLiteral("Keine Verkehrsmittel");
    }
    QStringList selected;
    QStringList excluded;
    for (int i = 0; i < HafasProducts::groupCount; ++i) {
        QString n = QString::fromUtf8(HafasProducts::groups[i].name);
        const int paren = n.indexOf(QLatin1Char('('));
        if (paren > 0) {
            n = n.left(paren).trimmed();
        }
        if (bitmask & HafasProducts::groups[i].bitmask) {
            selected.append(n);
        } else {
            excluded.append(n);
        }
    }
    if (excluded.size() <= 2) {
        return QStringLiteral("Alle außer ") + excluded.join(QStringLiteral(", "));
    }
    return QStringLiteral("Nur ") + selected.join(QStringLiteral(", "));
}
