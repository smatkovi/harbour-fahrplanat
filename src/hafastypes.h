#ifndef HAFASTYPES_H
#define HAFASTYPES_H

#include <QDateTime>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

// All timestamps are stored as "wall clock" values of the transport network
// (Europe/Vienna). They are kept in QDateTime objects with Qt::UTC spec purely
// as a container, so that no timezone conversion is ever applied. They are only
// used for display and for computing differences (delays, durations).

struct HafasLocation
{
    QString type;    // "S" station/stop, "A" address, "P" point of interest
    QString lid;     // HAFAS location identifier string, used for requests
    QString name;
    QString extId;   // station number (e.g. "8100353")
    double lat = 0.0;
    double lon = 0.0;
    bool hasCoord = false;
    int products = 0; // pCls bitmask

    bool isValid() const { return !lid.isEmpty() || !extId.isEmpty(); }

    QVariantMap toVariant() const;
    static HafasLocation fromVariant(const QVariantMap &m);
    QJsonObject toRequestJson() const;
};

struct HafasProduct
{
    QString name;       // "RJ 742", "S 2", "Bus 200"
    QString number;     // "742"
    QString line;       // line designation for buses/trams, may be empty
    QString catOut;     // short category, e.g. "RJ"
    QString catOutL;    // long category, e.g. "railjet"
    QString admin;
    QString operatorName;
    int cls = 0;        // product class bitmask
    QString color;      // "#rrggbb" from the HAFAS icon table, may be empty
    QString fgColor;

    QString displayName() const;
    QVariantMap toVariant() const;
};

struct HafasRemark
{
    QString type;   // A, I, M, D, U, R, N, Y, Q, P ...
    QString code;
    QString text;
    int prio = 0;

    QVariantMap toVariant() const;
};

struct HafasWarning
{
    QString hid;
    QString head;
    QString lead;
    QString text;
    QString company;
    int prio = 0;
    int cat = 0;
    int prod = 0;
    QDateTime validFrom;
    QDateTime validUntil;
    QDateTime modified;
    QStringList affectedLines;
    QString fromName;
    QString toName;

    QVariantMap toVariant() const;
};

struct HafasTime
{
    QDateTime scheduled;
    QDateTime realtime;
    QString platformS;
    QString platformR;
    bool cancelled = false;

    bool hasScheduled() const { return scheduled.isValid(); }
    bool hasRealtime() const { return realtime.isValid(); }
    QDateTime effective() const { return realtime.isValid() ? realtime : scheduled; }
    int delayMinutes() const;
    QString platform() const { return platformR.isEmpty() ? platformS : platformR; }
    bool platformChanged() const
    {
        return !platformR.isEmpty() && !platformS.isEmpty() && platformR != platformS;
    }

    // Adds keys with the given prefix, e.g. "depTime", "depDelay", "depPlatform"...
    void fillVariant(QVariantMap &m, const QString &prefix) const;
};

struct HafasPoint
{
    double lat = 0.0;
    double lon = 0.0;
};

struct HafasStop
{
    HafasLocation loc;
    HafasTime arr;
    HafasTime dep;
    bool passBy = false;
    bool additional = false;
    QList<HafasRemark> remarks;

    QVariantMap toVariant() const;
};

struct HafasLeg
{
    QString type;   // JNY, WALK, TRSF, DEVI, CHKI
    HafasLocation origin;
    HafasLocation destination;
    HafasTime dep;
    HafasTime arr;
    bool hasProduct = false;
    HafasProduct product;
    QString direction;
    QString jid;
    bool reachable = true;
    bool cancelled = false;
    int distance = 0;      // metres, for walking legs
    int durationMinutes = 0;
    QList<HafasStop> stops;
    QList<HafasRemark> remarks;
    QList<HafasWarning> warnings;
    QList<HafasPoint> polyline;   // geometry of the leg, if the server delivered one

    bool isPublicTransport() const { return type == QLatin1String("JNY"); }
    bool isWalk() const { return !isPublicTransport(); }

    QVariantMap toVariant() const;
};

struct HafasJourney
{
    QString cid;
    QString date;        // yyyyMMdd
    QString ctxRecon;
    int durationMinutes = 0;
    int changes = 0;
    HafasTime dep;
    HafasTime arr;
    HafasLocation origin;
    HafasLocation destination;
    QList<HafasLeg> legs;
    QList<HafasRemark> remarks;
    QList<HafasWarning> warnings;
    bool cancelled = false;

    QStringList lineNames() const;
    int warningCount() const;
    QVariantMap toVariant() const;
};

namespace HafasFormat
{
    QString time(const QDateTime &dt);                 // "12:34" or ""
    QString date(const QDateTime &dt);                 // "Di, 01.09.2026"
    QString dateTime(const QDateTime &dt);             // "01.09.2026 12:34"
    QString duration(int minutes);                     // "1 h 05 min" / "45 min"
    QString delay(int minutes);                        // "+5" / "-2" / ""
    QString distance(int metres);                      // "350 m" / "1,2 km"
    QString productNames(int bitmask);                 // "Railjet/ICE, S-Bahn"
    QString stripHtml(const QString &html);            // for list previews
    QString htmlToStyled(const QString &html);         // normalise for Text.StyledText
}

// Product groups used for filtering (bitmasks of HAFAS product classes)
struct HafasProductGroup
{
    const char *name;
    int bitmask;
};

namespace HafasProducts
{
    extern const HafasProductGroup groups[];
    extern const int groupCount;
    extern const int all;
    QString colorForClass(int cls);
    QString iconForClass(int cls);   // pictogram file name without extension
}

#endif // HAFASTYPES_H
