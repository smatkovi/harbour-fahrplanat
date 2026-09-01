#include "hafasparser.h"

#include <QJsonValue>
#include <QSet>
#include <QStringList>

// ---------------------------------------------------------------- common

HafasCommon::HafasCommon(const QJsonObject &res)
    : m_common(res.value(QStringLiteral("common")).toObject())
{
    m_locL = m_common.value(QStringLiteral("locL")).toArray();
    m_prodL = m_common.value(QStringLiteral("prodL")).toArray();
    m_opL = m_common.value(QStringLiteral("opL")).toArray();
    m_remL = m_common.value(QStringLiteral("remL")).toArray();
    m_himL = m_common.value(QStringLiteral("himL")).toArray();
    m_icoL = m_common.value(QStringLiteral("icoL")).toArray();
    m_polyL = m_common.value(QStringLiteral("polyL")).toArray();
}

QList<HafasPoint> HafasCommon::polyline(int index) const
{
    if (index < 0 || index >= m_polyL.size()) {
        return QList<HafasPoint>();
    }
    return HafasParser::decodePolyline(m_polyL.at(index).toObject().value(QStringLiteral("crdEncYX")).toString());
}

HafasLocation HafasCommon::location(int index) const
{
    if (index < 0 || index >= m_locL.size()) {
        return HafasLocation();
    }
    return HafasParser::parseLocation(m_locL.at(index).toObject());
}

QString HafasCommon::operatorName(int index) const
{
    if (index < 0 || index >= m_opL.size()) {
        return QString();
    }
    return m_opL.at(index).toObject().value(QStringLiteral("name")).toString();
}

QString HafasCommon::iconColor(int index, bool foreground) const
{
    if (index < 0 || index >= m_icoL.size()) {
        return QString();
    }
    const QJsonObject ico = m_icoL.at(index).toObject();
    const QJsonObject c = ico.value(foreground ? QStringLiteral("fg") : QStringLiteral("bg")).toObject();
    if (c.isEmpty()) {
        return QString();
    }
    return QStringLiteral("#%1%2%3")
        .arg(c.value(QStringLiteral("r")).toInt(), 2, 16, QLatin1Char('0'))
        .arg(c.value(QStringLiteral("g")).toInt(), 2, 16, QLatin1Char('0'))
        .arg(c.value(QStringLiteral("b")).toInt(), 2, 16, QLatin1Char('0'));
}

HafasProduct HafasCommon::product(int index) const
{
    HafasProduct p;
    if (index < 0 || index >= m_prodL.size()) {
        return p;
    }
    const QJsonObject o = m_prodL.at(index).toObject();
    p.name = o.value(QStringLiteral("name")).toString();
    p.number = o.value(QStringLiteral("number")).toString();
    p.cls = o.value(QStringLiteral("cls")).toInt();
    const QJsonObject ctx = o.value(QStringLiteral("prodCtx")).toObject();
    p.line = ctx.value(QStringLiteral("line")).toString();
    if (p.line.isEmpty()) {
        p.line = o.value(QStringLiteral("line")).toString();
    }
    p.catOut = ctx.value(QStringLiteral("catOut")).toString();
    if (p.catOut.trimmed().isEmpty()) {
        p.catOut = ctx.value(QStringLiteral("catOutS")).toString();
    }
    p.catOutL = ctx.value(QStringLiteral("catOutL")).toString();
    p.admin = ctx.value(QStringLiteral("admin")).toString();
    if (p.number.isEmpty()) {
        p.number = ctx.value(QStringLiteral("num")).toString();
    }
    if (o.contains(QStringLiteral("oprX"))) {
        p.operatorName = operatorName(o.value(QStringLiteral("oprX")).toInt(-1));
    }
    if (o.contains(QStringLiteral("icoX"))) {
        const int icoX = o.value(QStringLiteral("icoX")).toInt(-1);
        p.color = iconColor(icoX, false);
        p.fgColor = iconColor(icoX, true);
    }
    return p;
}

HafasRemark HafasCommon::remark(int index) const
{
    HafasRemark r;
    if (index < 0 || index >= m_remL.size()) {
        return r;
    }
    const QJsonObject o = m_remL.at(index).toObject();
    r.type = o.value(QStringLiteral("type")).toString();
    r.code = o.value(QStringLiteral("code")).toString();
    r.prio = o.value(QStringLiteral("prio")).toInt();
    QString text = o.value(QStringLiteral("txtN")).toString().trimmed();
    if (text.isEmpty()) {
        text = o.value(QStringLiteral("txtS")).toString().trimmed();
    }
    // Technical remarks carrying identifiers rather than text
    if (r.type == QLatin1String("I")
        && (r.code == QLatin1String("BB") || r.code == QLatin1String("IF") || r.code == QLatin1String("OZ")
            || r.code == QLatin1String("TW") || r.code == QLatin1String("TD") || r.code == QLatin1String("TE"))) {
        text.clear();
    }
    r.text = text;
    return r;
}

HafasWarning HafasCommon::warning(int index) const
{
    if (index < 0 || index >= m_himL.size()) {
        return HafasWarning();
    }
    return HafasParser::parseWarning(m_himL.at(index).toObject(), *this);
}

// ---------------------------------------------------------------- parser

namespace HafasParser
{

QDateTime parseDateTime(const QString &date, const QString &time)
{
    if (date.length() < 8 || time.length() < 6) {
        return QDateTime();
    }
    const QString d = date.right(8);
    const QDate day(d.mid(0, 4).toInt(), d.mid(4, 2).toInt(), d.mid(6, 2).toInt());
    const QString t = time.right(6);
    const QTime tod(t.mid(0, 2).toInt(), t.mid(2, 2).toInt(), t.mid(4, 2).toInt());
    if (!day.isValid() || !tod.isValid()) {
        return QDateTime();
    }
    int dayOffset = 0;
    if (time.length() > 6) {
        dayOffset = time.left(time.length() - 6).toInt();
    }
    // Qt::UTC as a plain container: no timezone conversion is applied anywhere.
    return QDateTime(day.addDays(dayOffset), tod, Qt::UTC);
}

int parseDuration(const QString &dur)
{
    if (dur.length() < 6) {
        return -1;
    }
    const QString t = dur.right(6);
    int minutes = t.mid(0, 2).toInt() * 60 + t.mid(2, 2).toInt();
    if (dur.length() > 6) {
        minutes += dur.left(dur.length() - 6).toInt() * 24 * 60;
    }
    return minutes;
}

HafasLocation parseLocation(const QJsonObject &l)
{
    HafasLocation loc;
    loc.type = l.value(QStringLiteral("type")).toString();
    loc.lid = l.value(QStringLiteral("lid")).toString();
    loc.name = l.value(QStringLiteral("name")).toString();
    loc.extId = l.value(QStringLiteral("extId")).toString();
    loc.products = l.value(QStringLiteral("pCls")).toInt();
    const QJsonObject crd = l.value(QStringLiteral("crd")).toObject();
    if (!crd.isEmpty()) {
        loc.lon = crd.value(QStringLiteral("x")).toDouble() / 1000000.0;
        loc.lat = crd.value(QStringLiteral("y")).toDouble() / 1000000.0;
        loc.hasCoord = true;
    }
    if (loc.name.isEmpty() || loc.extId.isEmpty() || !loc.hasCoord) {
        // Fall back to the fields encoded in the location identifier
        const QStringList parts = loc.lid.split(QLatin1Char('@'), QString::SkipEmptyParts);
        for (const QString &part : parts) {
            const int eq = part.indexOf(QLatin1Char('='));
            if (eq <= 0) {
                continue;
            }
            const QString key = part.left(eq);
            const QString value = part.mid(eq + 1);
            if (key == QLatin1String("O") && loc.name.isEmpty()) {
                loc.name = value;
            } else if (key == QLatin1String("L") && loc.extId.isEmpty()) {
                loc.extId = value;
            } else if (key == QLatin1String("X") && !loc.hasCoord) {
                loc.lon = value.toDouble() / 1000000.0;
            } else if (key == QLatin1String("Y") && !loc.hasCoord) {
                loc.lat = value.toDouble() / 1000000.0;
                loc.hasCoord = true;
            }
        }
    }
    if (loc.type.isEmpty()) {
        loc.type = QStringLiteral("S");
    }
    return loc;
}

static QString platformOf(const QJsonObject &o, const QString &oldKey, const QString &newKey)
{
    QString p = o.value(oldKey).toString();
    if (p.isEmpty()) {
        const QJsonValue v = o.value(newKey);
        if (v.isObject()) {
            p = v.toObject().value(QStringLiteral("txt")).toString();
        } else if (v.isString()) {
            p = v.toString();
        }
    }
    return p;
}

HafasTime parseTime(const QJsonObject &o, const QString &date, QChar prefix)
{
    HafasTime t;
    const QString pre(prefix);
    const QString timeS = o.value(pre + QStringLiteral("TimeS")).toString();
    const QString timeR = o.value(pre + QStringLiteral("TimeR")).toString();
    if (!timeS.isEmpty()) {
        t.scheduled = parseDateTime(date, timeS);
    }
    if (!timeR.isEmpty()) {
        t.realtime = parseDateTime(date, timeR);
    }
    t.platformS = platformOf(o, pre + QStringLiteral("PlatfS"), pre + QStringLiteral("PltfS"));
    t.platformR = platformOf(o, pre + QStringLiteral("PlatfR"), pre + QStringLiteral("PltfR"));
    t.cancelled = o.value(pre + QStringLiteral("Cncl")).toBool(false);
    return t;
}

void parseMessages(const QJsonArray &msgL, const HafasCommon &common,
                   QList<HafasRemark> &remarks, QList<HafasWarning> &warnings)
{
    QSet<QString> seenRemarks;
    QSet<QString> seenWarnings;
    for (const HafasRemark &r : remarks) {
        seenRemarks.insert(r.text);
    }
    for (const HafasWarning &w : warnings) {
        seenWarnings.insert(w.hid + w.head);
    }
    for (int i = 0; i < msgL.size(); ++i) {
        const QJsonObject m = msgL.at(i).toObject();
        const QString type = m.value(QStringLiteral("type")).toString();
        if (type == QLatin1String("REM")) {
            const HafasRemark r = common.remark(m.value(QStringLiteral("remX")).toInt(-1));
            if (r.text.isEmpty() || seenRemarks.contains(r.text)) {
                continue;
            }
            seenRemarks.insert(r.text);
            remarks.append(r);
        } else if (type == QLatin1String("HIM")) {
            const HafasWarning w = common.warning(m.value(QStringLiteral("himX")).toInt(-1));
            if ((w.head.isEmpty() && w.text.isEmpty()) || seenWarnings.contains(w.hid + w.head)) {
                continue;
            }
            seenWarnings.insert(w.hid + w.head);
            warnings.append(w);
        }
    }
}

HafasStop parseStop(const QJsonObject &st, const QString &date, const HafasCommon &common)
{
    HafasStop s;
    s.loc = common.location(st.value(QStringLiteral("locX")).toInt(-1));
    s.arr = parseTime(st, date, QLatin1Char('a'));
    s.dep = parseTime(st, date, QLatin1Char('d'));
    const QJsonValue dInS = st.value(QStringLiteral("dInS"));
    const QJsonValue aOutS = st.value(QStringLiteral("aOutS"));
    s.passBy = dInS.isBool() && !dInS.toBool() && aOutS.isBool() && !aOutS.toBool();
    s.additional = st.value(QStringLiteral("isAdd")).toBool(false);
    QList<HafasWarning> ignored;
    parseMessages(st.value(QStringLiteral("msgL")).toArray(), common, s.remarks, ignored);
    return s;
}

HafasLeg parseLeg(const QJsonObject &sec, const QString &date, const HafasCommon &common)
{
    HafasLeg leg;
    leg.type = sec.value(QStringLiteral("type")).toString();
    const QJsonObject dep = sec.value(QStringLiteral("dep")).toObject();
    const QJsonObject arr = sec.value(QStringLiteral("arr")).toObject();
    leg.origin = common.location(dep.value(QStringLiteral("locX")).toInt(-1));
    leg.destination = common.location(arr.value(QStringLiteral("locX")).toInt(-1));
    leg.dep = parseTime(dep, date, QLatin1Char('d'));
    leg.arr = parseTime(arr, date, QLatin1Char('a'));
    leg.cancelled = leg.dep.cancelled || leg.arr.cancelled;

    if (leg.type == QLatin1String("JNY")) {
        const QJsonObject jny = sec.value(QStringLiteral("jny")).toObject();
        leg.jid = jny.value(QStringLiteral("jid")).toString();
        leg.direction = jny.value(QStringLiteral("dirTxt")).toString();
        if (jny.contains(QStringLiteral("prodX"))) {
            leg.product = common.product(jny.value(QStringLiteral("prodX")).toInt(-1));
            leg.hasProduct = !leg.product.displayName().isEmpty();
        }
        if (jny.contains(QStringLiteral("isRchbl"))) {
            leg.reachable = jny.value(QStringLiteral("isRchbl")).toBool(true);
        }
        if (jny.value(QStringLiteral("isCncl")).toBool(false)) {
            leg.cancelled = true;
        }
        const QJsonArray stopL = jny.value(QStringLiteral("stopL")).toArray();
        for (int i = 0; i < stopL.size(); ++i) {
            leg.stops.append(parseStop(stopL.at(i).toObject(), date, common));
        }
        parseMessages(jny.value(QStringLiteral("msgL")).toArray(), common, leg.remarks, leg.warnings);
        // Departure and arrival platforms may only be present in the stop list
        if (leg.dep.platform().isEmpty() && !leg.stops.isEmpty()) {
            leg.dep.platformS = leg.stops.first().dep.platformS;
            leg.dep.platformR = leg.stops.first().dep.platformR;
        }
        if (leg.arr.platform().isEmpty() && !leg.stops.isEmpty()) {
            leg.arr.platformS = leg.stops.last().arr.platformS;
            leg.arr.platformR = leg.stops.last().arr.platformR;
        }
    } else {
        const QJsonObject gis = sec.value(QStringLiteral("gis")).toObject();
        leg.distance = gis.value(QStringLiteral("dist")).toInt(0);
        const QString durS = gis.value(QStringLiteral("durS")).toString();
        if (!durS.isEmpty()) {
            leg.durationMinutes = parseDuration(durS);
        }
        parseMessages(gis.value(QStringLiteral("msgL")).toArray(), common, leg.remarks, leg.warnings);
        const QJsonArray polyXL = gis.value(QStringLiteral("polyG")).toObject().value(QStringLiteral("polyXL")).toArray();
        for (int i = 0; i < polyXL.size() && leg.polyline.isEmpty(); ++i) {
            leg.polyline = common.polyline(polyXL.at(i).toInt(-1));
        }
    }
    if (leg.isPublicTransport()) {
        const QJsonObject jny = sec.value(QStringLiteral("jny")).toObject();
        const QJsonArray polyXL = jny.value(QStringLiteral("polyG")).toObject().value(QStringLiteral("polyXL")).toArray();
        for (int i = 0; i < polyXL.size() && leg.polyline.isEmpty(); ++i) {
            leg.polyline = common.polyline(polyXL.at(i).toInt(-1));
        }
    }

    if (leg.durationMinutes <= 0 && leg.dep.hasScheduled() && leg.arr.hasScheduled()) {
        leg.durationMinutes = static_cast<int>(leg.dep.scheduled.secsTo(leg.arr.scheduled) / 60);
    }
    parseMessages(sec.value(QStringLiteral("msgL")).toArray(), common, leg.remarks, leg.warnings);
    return leg;
}

HafasJourney parseJourney(const QJsonObject &con, const HafasCommon &common)
{
    HafasJourney j;
    j.cid = con.value(QStringLiteral("cid")).toString();
    j.date = con.value(QStringLiteral("date")).toString();
    j.durationMinutes = parseDuration(con.value(QStringLiteral("dur")).toString());
    j.changes = con.value(QStringLiteral("chg")).toInt(0);
    j.ctxRecon = con.value(QStringLiteral("ctxRecon")).toString();
    if (j.ctxRecon.isEmpty()) {
        j.ctxRecon = con.value(QStringLiteral("recon")).toObject().value(QStringLiteral("ctx")).toString();
    }
    const QJsonObject dep = con.value(QStringLiteral("dep")).toObject();
    const QJsonObject arr = con.value(QStringLiteral("arr")).toObject();
    j.origin = common.location(dep.value(QStringLiteral("locX")).toInt(-1));
    j.destination = common.location(arr.value(QStringLiteral("locX")).toInt(-1));
    j.dep = parseTime(dep, j.date, QLatin1Char('d'));
    j.arr = parseTime(arr, j.date, QLatin1Char('a'));

    const QJsonArray secL = con.value(QStringLiteral("secL")).toArray();
    for (int i = 0; i < secL.size(); ++i) {
        HafasLeg leg = parseLeg(secL.at(i).toObject(), j.date, common);
        if (leg.cancelled && leg.isPublicTransport()) {
            j.cancelled = true;
        }
        j.legs.append(leg);
    }
    if (j.dep.cancelled || j.arr.cancelled) {
        j.cancelled = true;
    }
    // Use platforms from the first / last public transport leg
    if (j.dep.platform().isEmpty()) {
        for (const HafasLeg &l : j.legs) {
            if (l.isPublicTransport()) {
                j.dep.platformS = l.dep.platformS;
                j.dep.platformR = l.dep.platformR;
                break;
            }
        }
    }
    if (j.arr.platform().isEmpty()) {
        for (int i = j.legs.size() - 1; i >= 0; --i) {
            if (j.legs.at(i).isPublicTransport()) {
                j.arr.platformS = j.legs.at(i).arr.platformS;
                j.arr.platformR = j.legs.at(i).arr.platformR;
                break;
            }
        }
    }
    if (j.durationMinutes < 0 && j.dep.hasScheduled() && j.arr.hasScheduled()) {
        j.durationMinutes = static_cast<int>(j.dep.effective().secsTo(j.arr.effective()) / 60);
    }
    parseMessages(con.value(QStringLiteral("msgL")).toArray(), common, j.remarks, j.warnings);
    return j;
}

HafasWarning parseWarning(const QJsonObject &w, const HafasCommon &common)
{
    HafasWarning h;
    h.hid = w.value(QStringLiteral("hid")).toString();
    h.head = w.value(QStringLiteral("head")).toString();
    h.lead = w.value(QStringLiteral("lead")).toString();
    h.text = w.value(QStringLiteral("text")).toString();
    h.company = w.value(QStringLiteral("comp")).toString();
    h.prio = w.value(QStringLiteral("prio")).toInt(0);
    h.cat = w.value(QStringLiteral("cat")).toInt(0);
    h.prod = w.value(QStringLiteral("prod")).toInt(0);
    h.validFrom = parseDateTime(w.value(QStringLiteral("sDate")).toString(),
                                w.value(QStringLiteral("sTime")).toString());
    h.validUntil = parseDateTime(w.value(QStringLiteral("eDate")).toString(),
                                 w.value(QStringLiteral("eTime")).toString());
    h.modified = parseDateTime(w.value(QStringLiteral("lModDate")).toString(),
                               w.value(QStringLiteral("lModTime")).toString());
    const QJsonArray aff = w.value(QStringLiteral("affProdRefL")).toArray();
    for (int i = 0; i < aff.size(); ++i) {
        const QString n = common.product(aff.at(i).toInt(-1)).displayName();
        if (!n.isEmpty() && !h.affectedLines.contains(n)) {
            h.affectedLines.append(n);
        }
    }
    if (w.contains(QStringLiteral("fLocX"))) {
        h.fromName = common.location(w.value(QStringLiteral("fLocX")).toInt(-1)).name;
    }
    if (w.contains(QStringLiteral("tLocX"))) {
        h.toName = common.location(w.value(QStringLiteral("tLocX")).toInt(-1)).name;
    }
    return h;
}

QList<HafasWarning> parseHimSearch(const QJsonObject &res)
{
    HafasCommon common(res);
    QList<HafasWarning> list;
    QSet<QString> seen;

    const QJsonArray msgL = res.value(QStringLiteral("msgL")).toArray();
    for (int i = 0; i < msgL.size(); ++i) {
        const HafasWarning w = parseWarning(msgL.at(i).toObject(), common);
        if (!seen.contains(w.hid)) {
            seen.insert(w.hid);
            list.append(w);
        }
    }
    const QJsonArray msgRefL = res.value(QStringLiteral("msgRefL")).toArray();
    for (int i = 0; i < msgRefL.size(); ++i) {
        const HafasWarning w = common.warning(msgRefL.at(i).toInt(-1));
        if (!w.hid.isEmpty() && !seen.contains(w.hid)) {
            seen.insert(w.hid);
            list.append(w);
        }
    }
    if (list.isEmpty()) {
        for (int i = 0; i < common.warningCount(); ++i) {
            const HafasWarning w = common.warning(i);
            if (!w.hid.isEmpty() && !seen.contains(w.hid)) {
                seen.insert(w.hid);
                list.append(w);
            }
        }
    }
    return list;
}

QList<HafasPoint> decodePolyline(const QString &encoded)
{
    QList<HafasPoint> points;
    const QByteArray data = encoded.toLatin1();
    int index = 0;
    long lat = 0;
    long lon = 0;
    while (index < data.size()) {
        long result = 0;
        int shift = 0;
        int b = 0;
        do {
            if (index >= data.size()) {
                return points;
            }
            b = static_cast<int>(data.at(index++)) - 63;
            result |= static_cast<long>(b & 0x1f) << shift;
            shift += 5;
        } while (b >= 0x20);
        lat += (result & 1) ? ~(result >> 1) : (result >> 1);

        result = 0;
        shift = 0;
        do {
            if (index >= data.size()) {
                return points;
            }
            b = static_cast<int>(data.at(index++)) - 63;
            result |= static_cast<long>(b & 0x1f) << shift;
            shift += 5;
        } while (b >= 0x20);
        lon += (result & 1) ? ~(result >> 1) : (result >> 1);

        HafasPoint p;
        p.lat = lat / 1e5;
        p.lon = lon / 1e5;
        points.append(p);
    }
    return points;
}

} // namespace HafasParser
