#include "hafastypes.h"

#include <QJsonValue>
#include <QLocale>
#include <QRegExp>
#include <QStringList>

// ---------------------------------------------------------------- products

namespace HafasProducts
{
    // Product groups exactly as the ÖBB backend uses them (product class bits).
    const HafasProductGroup groups[] = {
        { "Fernreisezüge (RJ, RJX, ICE, IC, EC)", 0x1005 },
        { "Nachtreisezüge und Schnellzüge (NJ, EN, D)", 0x0008 },
        { "Regionale Reisezüge (R, REX, CJX)", 0x0010 },
        { "S-Bahnen", 0x0020 },
        { "U-Bahnen", 0x0100 },
        { "Straßenbahnen", 0x0200 },
        { "Busse", 0x0040 },
        { "Sonstige Verkehre (Rufbus, AST, Seilbahn)", 0x0800 },
        { "Schifffahrtsverkehre", 0x0080 },
        { "Schienenersatzverkehre", 0x0002 },
    };
    const int groupCount = sizeof(groups) / sizeof(groups[0]);
    const int all = 0x1BFF; // 7167

    QString colorForClass(int cls)
    {
        if (cls & 0x1005) return QStringLiteral("#ab0020"); // long distance red
        if (cls & 0x0008) return QStringLiteral("#3b1f5e"); // night trains
        if (cls & 0x0010) return QStringLiteral("#3d85d0"); // regional blue
        if (cls & 0x0020) return QStringLiteral("#2a8a3a"); // S-Bahn green
        if (cls & 0x0100) return QStringLiteral("#e2001a"); // U-Bahn
        if (cls & 0x0200) return QStringLiteral("#c8102e"); // tram
        if (cls & 0x0040) return QStringLiteral("#a3238e"); // bus
        if (cls & 0x0080) return QStringLiteral("#00689d"); // ship
        if (cls & 0x0800) return QStringLiteral("#7a7a7a"); // on-call
        if (cls & 0x0002) return QStringLiteral("#8b5a2b"); // rail replacement
        return QStringLiteral("#555555");
    }

    QString iconForClass(int cls)
    {
        if (cls & 0x0020) return QStringLiteral("sbahn");
        if (cls & 0x0100) return QStringLiteral("ubahn");
        if (cls & 0x0200) return QStringLiteral("tram");
        if (cls & (0x0040 | 0x0002)) return QStringLiteral("bus");
        if (cls & 0x0080) return QStringLiteral("ship");
        if (cls & 0x0800) return QStringLiteral("taxi");
        return QStringLiteral("train");
    }
}

// ---------------------------------------------------------------- location

QVariantMap HafasLocation::toVariant() const
{
    QVariantMap m;
    m.insert(QStringLiteral("type"), type);
    m.insert(QStringLiteral("lid"), lid);
    m.insert(QStringLiteral("name"), name);
    m.insert(QStringLiteral("extId"), extId);
    m.insert(QStringLiteral("lat"), lat);
    m.insert(QStringLiteral("lon"), lon);
    m.insert(QStringLiteral("hasCoord"), hasCoord);
    m.insert(QStringLiteral("products"), products);
    m.insert(QStringLiteral("isStation"), type == QLatin1String("S"));
    QString kind;
    if (type == QLatin1String("S")) {
        kind = products ? HafasFormat::productNames(products) : QStringLiteral("Haltestelle");
    } else if (type == QLatin1String("A")) {
        kind = QStringLiteral("Adresse");
    } else if (type == QLatin1String("P")) {
        kind = QStringLiteral("Ort / Sehenswürdigkeit");
    }
    m.insert(QStringLiteral("description"), kind);
    return m;
}

HafasLocation HafasLocation::fromVariant(const QVariantMap &m)
{
    HafasLocation l;
    l.type = m.value(QStringLiteral("type")).toString();
    l.lid = m.value(QStringLiteral("lid")).toString();
    l.name = m.value(QStringLiteral("name")).toString();
    l.extId = m.value(QStringLiteral("extId")).toString();
    l.lat = m.value(QStringLiteral("lat")).toDouble();
    l.lon = m.value(QStringLiteral("lon")).toDouble();
    l.hasCoord = m.value(QStringLiteral("hasCoord")).toBool();
    l.products = m.value(QStringLiteral("products")).toInt();
    if (l.type.isEmpty()) {
        l.type = QStringLiteral("S");
    }
    return l;
}

QJsonObject HafasLocation::toRequestJson() const
{
    QJsonObject o;
    o.insert(QStringLiteral("type"), type.isEmpty() ? QStringLiteral("S") : type);
    if (!lid.isEmpty()) {
        o.insert(QStringLiteral("lid"), lid);
    } else if (!extId.isEmpty()) {
        o.insert(QStringLiteral("extId"), extId);
    }
    if (type != QLatin1String("S") && !name.isEmpty()) {
        o.insert(QStringLiteral("name"), name);
    }
    return o;
}

// ---------------------------------------------------------------- product

QString HafasProduct::displayName() const
{
    if (!line.isEmpty()) {
        return line.trimmed();
    }
    if (!name.isEmpty()) {
        return name.simplified();
    }
    return catOut.trimmed();
}

QVariantMap HafasProduct::toVariant() const
{
    QVariantMap m;
    m.insert(QStringLiteral("name"), displayName());
    m.insert(QStringLiteral("fullName"), name.simplified());
    m.insert(QStringLiteral("number"), number);
    m.insert(QStringLiteral("category"), catOut.trimmed());
    m.insert(QStringLiteral("categoryLong"), catOutL.trimmed());
    m.insert(QStringLiteral("operator"), operatorName);
    m.insert(QStringLiteral("cls"), cls);
    m.insert(QStringLiteral("color"), color.isEmpty() ? HafasProducts::colorForClass(cls) : color);
    m.insert(QStringLiteral("fgColor"), fgColor.isEmpty() ? QStringLiteral("#ffffff") : fgColor);
    m.insert(QStringLiteral("icon"), HafasProducts::iconForClass(cls));
    return m;
}

// ---------------------------------------------------------------- remark

QVariantMap HafasRemark::toVariant() const
{
    QVariantMap m;
    m.insert(QStringLiteral("type"), type);
    m.insert(QStringLiteral("code"), code);
    m.insert(QStringLiteral("text"), HafasFormat::htmlToStyled(text));
    m.insert(QStringLiteral("prio"), prio);
    return m;
}

// ---------------------------------------------------------------- warning

QVariantMap HafasWarning::toVariant() const
{
    QVariantMap m;
    m.insert(QStringLiteral("id"), hid);
    m.insert(QStringLiteral("head"), HafasFormat::stripHtml(head));
    m.insert(QStringLiteral("lead"), HafasFormat::stripHtml(lead));
    m.insert(QStringLiteral("text"), HafasFormat::htmlToStyled(text));
    m.insert(QStringLiteral("plainText"), HafasFormat::stripHtml(text));
    m.insert(QStringLiteral("company"), company);
    m.insert(QStringLiteral("prio"), prio);
    m.insert(QStringLiteral("category"), cat);
    m.insert(QStringLiteral("products"), HafasFormat::productNames(prod));
    m.insert(QStringLiteral("validFrom"), HafasFormat::dateTime(validFrom));
    m.insert(QStringLiteral("validUntil"), HafasFormat::dateTime(validUntil));
    m.insert(QStringLiteral("modified"), HafasFormat::dateTime(modified));
    m.insert(QStringLiteral("affectedLines"), affectedLines.join(QStringLiteral(", ")));
    m.insert(QStringLiteral("fromName"), fromName);
    m.insert(QStringLiteral("toName"), toName);
    return m;
}

// ---------------------------------------------------------------- time

int HafasTime::delayMinutes() const
{
    if (!scheduled.isValid() || !realtime.isValid()) {
        return 0;
    }
    return static_cast<int>(scheduled.secsTo(realtime) / 60);
}

void HafasTime::fillVariant(QVariantMap &m, const QString &prefix) const
{
    m.insert(prefix + QStringLiteral("Time"), HafasFormat::time(scheduled));
    m.insert(prefix + QStringLiteral("RealTime"), HafasFormat::time(effective()));
    m.insert(prefix + QStringLiteral("HasRealtime"), hasRealtime());
    m.insert(prefix + QStringLiteral("Delay"), delayMinutes());
    m.insert(prefix + QStringLiteral("DelayText"), hasRealtime() ? HafasFormat::delay(delayMinutes()) : QString());
    m.insert(prefix + QStringLiteral("Platform"), platform());
    m.insert(prefix + QStringLiteral("PlannedPlatform"), platformS);
    m.insert(prefix + QStringLiteral("PlatformChanged"), platformChanged());
    m.insert(prefix + QStringLiteral("Cancelled"), cancelled);
    m.insert(prefix + QStringLiteral("Date"), HafasFormat::date(effective()));
}

// ---------------------------------------------------------------- stop

QVariantMap HafasStop::toVariant() const
{
    QVariantMap m;
    m.insert(QStringLiteral("name"), loc.name);
    m.insert(QStringLiteral("location"), loc.toVariant());
    arr.fillVariant(m, QStringLiteral("arr"));
    dep.fillVariant(m, QStringLiteral("dep"));
    m.insert(QStringLiteral("passBy"), passBy);
    m.insert(QStringLiteral("additional"), additional);
    m.insert(QStringLiteral("cancelled"), arr.cancelled || dep.cancelled);
    QVariantList rems;
    for (const HafasRemark &r : remarks) {
        rems.append(r.toVariant());
    }
    m.insert(QStringLiteral("remarks"), rems);
    return m;
}

// ---------------------------------------------------------------- leg

QVariantMap HafasLeg::toVariant() const
{
    QVariantMap m;
    m.insert(QStringLiteral("type"), type);
    m.insert(QStringLiteral("isWalk"), isWalk());
    m.insert(QStringLiteral("isTransfer"), type == QLatin1String("TRSF") || type == QLatin1String("DEVI"));
    m.insert(QStringLiteral("origin"), origin.name);
    m.insert(QStringLiteral("destination"), destination.name);
    m.insert(QStringLiteral("originLocation"), origin.toVariant());
    m.insert(QStringLiteral("destinationLocation"), destination.toVariant());
    dep.fillVariant(m, QStringLiteral("dep"));
    arr.fillVariant(m, QStringLiteral("arr"));
    m.insert(QStringLiteral("hasProduct"), hasProduct);
    m.insert(QStringLiteral("product"), product.toVariant());
    m.insert(QStringLiteral("lineName"), hasProduct ? product.displayName() : QString());
    m.insert(QStringLiteral("icon"), isWalk() ? QStringLiteral("walk") : HafasProducts::iconForClass(product.cls));
    m.insert(QStringLiteral("direction"), direction);
    m.insert(QStringLiteral("jid"), jid);
    m.insert(QStringLiteral("reachable"), reachable);
    m.insert(QStringLiteral("cancelled"), cancelled);
    m.insert(QStringLiteral("distance"), distance);
    m.insert(QStringLiteral("distanceText"), HafasFormat::distance(distance));
    m.insert(QStringLiteral("duration"), durationMinutes);
    m.insert(QStringLiteral("durationText"), HafasFormat::duration(durationMinutes));
    QVariantList st;
    for (const HafasStop &s : stops) {
        st.append(s.toVariant());
    }
    m.insert(QStringLiteral("stops"), st);
    // Intermediate stops only (without first and last)
    QVariantList inter;
    for (int i = 1; i + 1 < stops.size(); ++i) {
        if (!stops.at(i).passBy) {
            inter.append(stops.at(i).toVariant());
        }
    }
    m.insert(QStringLiteral("intermediateStops"), inter);
    QVariantList rems;
    for (const HafasRemark &r : remarks) {
        rems.append(r.toVariant());
    }
    m.insert(QStringLiteral("remarks"), rems);
    QVariantList warns;
    for (const HafasWarning &w : warnings) {
        warns.append(w.toVariant());
    }
    m.insert(QStringLiteral("warnings"), warns);
    QVariantList poly;
    for (const HafasPoint &pt : polyline) {
        QVariantMap pm;
        pm.insert(QStringLiteral("lat"), pt.lat);
        pm.insert(QStringLiteral("lon"), pt.lon);
        poly.append(pm);
    }
    m.insert(QStringLiteral("polyline"), poly);
    return m;
}

// ---------------------------------------------------------------- journey

QStringList HafasJourney::lineNames() const
{
    QStringList names;
    for (const HafasLeg &l : legs) {
        if (l.isPublicTransport() && l.hasProduct) {
            const QString n = l.product.displayName();
            if (!n.isEmpty()) {
                names.append(n);
            }
        }
    }
    return names;
}

int HafasJourney::warningCount() const
{
    int n = warnings.size();
    for (const HafasLeg &l : legs) {
        n += l.warnings.size();
    }
    return n;
}

QVariantMap HafasJourney::toVariant() const
{
    QVariantMap m;
    m.insert(QStringLiteral("cid"), cid);
    m.insert(QStringLiteral("date"), date);
    m.insert(QStringLiteral("dateText"), HafasFormat::date(dep.effective()));
    m.insert(QStringLiteral("ctxRecon"), ctxRecon);
    m.insert(QStringLiteral("duration"), durationMinutes);
    m.insert(QStringLiteral("durationText"), HafasFormat::duration(durationMinutes));
    m.insert(QStringLiteral("changes"), changes);
    m.insert(QStringLiteral("origin"), origin.name);
    m.insert(QStringLiteral("destination"), destination.name);
    dep.fillVariant(m, QStringLiteral("dep"));
    arr.fillVariant(m, QStringLiteral("arr"));
    m.insert(QStringLiteral("lines"), lineNames().join(QStringLiteral(" · ")));
    m.insert(QStringLiteral("cancelled"), cancelled);
    m.insert(QStringLiteral("warningCount"), warningCount());
    QVariantList lg;
    for (const HafasLeg &l : legs) {
        lg.append(l.toVariant());
    }
    m.insert(QStringLiteral("legs"), lg);
    QVariantList rems;
    for (const HafasRemark &r : remarks) {
        rems.append(r.toVariant());
    }
    m.insert(QStringLiteral("remarks"), rems);
    QVariantList warns;
    for (const HafasWarning &w : warnings) {
        warns.append(w.toVariant());
    }
    m.insert(QStringLiteral("warnings"), warns);
    return m;
}

// ---------------------------------------------------------------- formatting

namespace HafasFormat
{
    QString time(const QDateTime &dt)
    {
        return dt.isValid() ? dt.toString(QStringLiteral("HH:mm")) : QString();
    }

    QString date(const QDateTime &dt)
    {
        if (!dt.isValid()) {
            return QString();
        }
        static const char *days[] = { "Mo", "Di", "Mi", "Do", "Fr", "Sa", "So" };
        const int dow = dt.date().dayOfWeek(); // 1..7
        return QString::fromLatin1(days[(dow - 1) % 7]) + QStringLiteral(", ")
               + dt.toString(QStringLiteral("dd.MM.yyyy"));
    }

    QString dateTime(const QDateTime &dt)
    {
        return dt.isValid() ? dt.toString(QStringLiteral("dd.MM.yyyy HH:mm")) : QString();
    }

    QString duration(int minutes)
    {
        if (minutes < 0) {
            return QString();
        }
        const int h = minutes / 60;
        const int min = minutes % 60;
        if (h == 0) {
            return QStringLiteral("%1 min").arg(min);
        }
        return QStringLiteral("%1 h %2 min").arg(h).arg(min, 2, 10, QLatin1Char('0'));
    }

    QString delay(int minutes)
    {
        if (minutes > 0) {
            return QStringLiteral("+%1").arg(minutes);
        }
        if (minutes < 0) {
            return QStringLiteral("%1").arg(minutes);
        }
        return QStringLiteral("+0");
    }

    QString distance(int metres)
    {
        if (metres <= 0) {
            return QString();
        }
        if (metres < 1000) {
            return QStringLiteral("%1 m").arg(metres);
        }
        return QStringLiteral("%1 km").arg(QLocale(QLocale::German).toString(metres / 1000.0, 'f', 1));
    }

    QString productNames(int bitmask)
    {
        if ((bitmask & HafasProducts::all) == HafasProducts::all) {
            return QStringLiteral("Alle Verkehrsmittel");
        }
        QStringList names;
        for (int i = 0; i < HafasProducts::groupCount; ++i) {
            if (bitmask & HafasProducts::groups[i].bitmask) {
                QString n = QString::fromUtf8(HafasProducts::groups[i].name);
                const int paren = n.indexOf(QLatin1Char('('));
                if (paren > 0) {
                    n = n.left(paren).trimmed();
                }
                names.append(n);
            }
        }
        if (names.size() > 4) {
            const int more = names.size() - 3;
            names = names.mid(0, 3);
            names.append(QStringLiteral("%1 weitere").arg(more));
        }
        return names.join(QStringLiteral(", "));
    }

    QString stripHtml(const QString &html)
    {
        QString s = html;
        s.replace(QRegExp(QStringLiteral("<br\\s*/?>"), Qt::CaseInsensitive), QStringLiteral(" "));
        s.remove(QRegExp(QStringLiteral("<[^>]*>")));
        s.replace(QStringLiteral("&nbsp;"), QStringLiteral(" "));
        s.replace(QStringLiteral("&amp;"), QStringLiteral("&"));
        s.replace(QStringLiteral("&lt;"), QStringLiteral("<"));
        s.replace(QStringLiteral("&gt;"), QStringLiteral(">"));
        s.replace(QStringLiteral("&quot;"), QStringLiteral("\""));
        s.replace(QStringLiteral("&#39;"), QStringLiteral("'"));
        return s.simplified();
    }

    QString htmlToStyled(const QString &html)
    {
        // Text.StyledText understands <b>, <i>, <u>, <br>, <a>, <p>.
        // Normalise line breaks and drop tags it cannot render.
        QString s = html;
        s.replace(QRegExp(QStringLiteral("<br\\s*/?>"), Qt::CaseInsensitive), QStringLiteral("<br>"));
        s.replace(QRegExp(QStringLiteral("</?(div|span|ul|ol|font|strong|em)[^>]*>"), Qt::CaseInsensitive),
                  QString());
        s.replace(QRegExp(QStringLiteral("<li[^>]*>"), Qt::CaseInsensitive), QStringLiteral("<br>• "));
        s.replace(QRegExp(QStringLiteral("</li>"), Qt::CaseInsensitive), QString());
        s.replace(QRegExp(QStringLiteral("</p>\\s*<p[^>]*>"), Qt::CaseInsensitive), QStringLiteral("<br><br>"));
        s.replace(QRegExp(QStringLiteral("</?p[^>]*>"), Qt::CaseInsensitive), QString());
        s.replace(QStringLiteral("\n"), QStringLiteral("<br>"));
        while (s.startsWith(QLatin1String("<br>"))) {
            s = s.mid(4);
        }
        return s.trimmed();
    }
}
