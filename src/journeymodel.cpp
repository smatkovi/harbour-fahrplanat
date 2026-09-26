#include "journeymodel.h"
#include "appsettings.h"
#include "hafasclient.h"
#include "hafasparser.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkReply>
#include <QSet>

JourneyModel::JourneyModel(HafasClient *client, AppSettings *settings, QObject *parent)
    : QAbstractListModel(parent)
    , m_client(client)
    , m_settings(settings)
{
    // Qt 4.7 fragt ein Modell nicht nach roleNames(): dort ist das ein
    // gewoehnlicher Zugriff, kein virtueller. Die Namen muessen einmal
    // gesetzt werden, sonst kennt QML keine einzige Rolle -- die Liste
    // bleibt dann leer, ohne eine Meldung.
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    setRoleNames(roleNames());
#endif
}

int JourneyModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_journeys.size();
}

QVariant JourneyModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_journeys.size()) {
        return QVariant();
    }
    const HafasJourney &j = m_journeys.at(index.row());
    switch (role) {
    case DepTimeRole: return HafasFormat::time(j.dep.scheduled);
    case DepRealTimeRole: return HafasFormat::time(j.dep.effective());
    case DepDelayRole: return j.dep.delayMinutes();
    case DepDelayTextRole: return j.dep.hasRealtime() ? HafasFormat::delay(j.dep.delayMinutes()) : QString();
    case DepHasRealtimeRole: return j.dep.hasRealtime();
    case ArrTimeRole: return HafasFormat::time(j.arr.scheduled);
    case ArrRealTimeRole: return HafasFormat::time(j.arr.effective());
    case ArrDelayRole: return j.arr.delayMinutes();
    case ArrDelayTextRole: return j.arr.hasRealtime() ? HafasFormat::delay(j.arr.delayMinutes()) : QString();
    case ArrHasRealtimeRole: return j.arr.hasRealtime();
    case DurationRole: return HafasFormat::duration(j.durationMinutes);
    case ChangesRole: return j.changes;
    case LinesRole: return j.lineNames().join(QStringLiteral(" · "));
    case DepPlatformRole: return j.dep.platform();
    case DepPlatformChangedRole: return j.dep.platformChanged();
    case ArrPlatformRole: return j.arr.platform();
    case CancelledRole: return j.cancelled;
    case WarningCountRole: return j.warningCount();
    case DateTextRole: return HafasFormat::date(j.dep.effective());
    case OriginRole: return j.origin.name;
    case DestinationRole: return j.destination.name;
    case DayChangeRole: {
        if (index.row() == 0) {
            return true;
        }
        const HafasJourney &prev = m_journeys.at(index.row() - 1);
        return prev.dep.effective().date() != j.dep.effective().date();
    }
    case LegsSummaryRole: {
        // Badge sequence for the overview: products of all public transport legs,
        // walking legs only when they take a while (as the reference app does).
        QVariantList list;
        for (const HafasLeg &l : j.legs) {
            QVariantMap b;
            if (l.isPublicTransport()) {
                if (!l.hasProduct) {
                    continue;
                }
                b.insert(QStringLiteral("text"), l.product.displayName());
                b.insert(QStringLiteral("color"), l.product.color.isEmpty()
                                                     ? HafasProducts::colorForClass(l.product.cls)
                                                     : l.product.color);
                b.insert(QStringLiteral("fgColor"), l.product.fgColor.isEmpty() ? QStringLiteral("#ffffff")
                                                                                 : l.product.fgColor);
                b.insert(QStringLiteral("icon"), HafasProducts::iconForClass(l.product.cls));
                b.insert(QStringLiteral("isWalk"), false);
                b.insert(QStringLiteral("cancelled"), l.cancelled);
            } else {
                if (l.durationMinutes < 15) {
                    continue;
                }
                b.insert(QStringLiteral("text"), QStringLiteral("%1 min").arg(l.durationMinutes));
                b.insert(QStringLiteral("color"), QStringLiteral("#6f6f6f"));
                b.insert(QStringLiteral("fgColor"), QStringLiteral("#ffffff"));
                b.insert(QStringLiteral("icon"), QStringLiteral("walk"));
                b.insert(QStringLiteral("isWalk"), true);
                b.insert(QStringLiteral("cancelled"), false);
            }
            list.append(b);
        }
        return list;
    }
    }
    return QVariant();
}

QHash<int, QByteArray> JourneyModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[DepTimeRole] = "depTime";
    roles[DepRealTimeRole] = "depRealTime";
    roles[DepDelayRole] = "depDelay";
    roles[DepDelayTextRole] = "depDelayText";
    roles[DepHasRealtimeRole] = "depHasRealtime";
    roles[ArrTimeRole] = "arrTime";
    roles[ArrRealTimeRole] = "arrRealTime";
    roles[ArrDelayRole] = "arrDelay";
    roles[ArrDelayTextRole] = "arrDelayText";
    roles[ArrHasRealtimeRole] = "arrHasRealtime";
    roles[DurationRole] = "duration";
    roles[ChangesRole] = "changes";
    roles[LinesRole] = "lines";
    roles[DepPlatformRole] = "depPlatform";
    roles[DepPlatformChangedRole] = "depPlatformChanged";
    roles[ArrPlatformRole] = "arrPlatform";
    roles[CancelledRole] = "cancelled";
    roles[WarningCountRole] = "warningCount";
    roles[DateTextRole] = "dateText";
    roles[OriginRole] = "origin";
    roles[DestinationRole] = "destination";
    roles[DayChangeRole] = "dayChange";
    roles[LegsSummaryRole] = "legsSummary";
    return roles;
}

QString JourneyModel::whenText() const
{
    if (!m_when.isValid()) {
        return QString();
    }
    return QStringLiteral("%1 %2 %3")
        .arg(m_isDeparture ? QStringLiteral("Abfahrt") : QStringLiteral("Ankunft"))
        .arg(HafasFormat::date(QDateTime(m_when.date(), m_when.time(), Qt::UTC)))
        .arg(m_when.toString(QStringLiteral("HH:mm")));
}

QString JourneyModel::summary() const
{
    for (const HafasJourney &j : m_journeys) {
        if (j.cancelled) {
            continue;
        }
        QString s = HafasFormat::time(j.dep.effective());
        if (j.dep.hasRealtime() && j.dep.delayMinutes() != 0) {
            s += QStringLiteral(" (%1)").arg(HafasFormat::delay(j.dep.delayMinutes()));
        }
        if (!j.dep.platform().isEmpty()) {
            s += QStringLiteral(" Gl. %1").arg(j.dep.platform());
        }
        const QStringList lines = j.lineNames();
        if (!lines.isEmpty()) {
            s += QStringLiteral("\n%1").arg(lines.first());
        }
        return s;
    }
    return QString();
}

void JourneyModel::setBusy(bool busy)
{
    if (m_busy != busy) {
        m_busy = busy;
        emit busyChanged();
    }
}

void JourneyModel::setError(const QString &error)
{
    if (m_error != error) {
        m_error = error;
        emit errorChanged();
    }
}

void JourneyModel::search(const QVariantMap &from, const QVariantMap &to, const QVariantMap &via,
                          const QDateTime &when, bool isDeparture)
{
    m_from = HafasLocation::fromVariant(from);
    m_to = HafasLocation::fromVariant(to);
    m_via = via.isEmpty() ? HafasLocation() : HafasLocation::fromVariant(via);
    m_when = when.isValid() ? when.toLocalTime() : QDateTime::currentDateTime();
    m_isDeparture = isDeparture;
    emit searchChanged();

    if (!m_from.isValid() || !m_to.isValid()) {
        setError(QStringLiteral("Bitte Start und Ziel auswählen."));
        return;
    }
    beginResetModel();
    m_journeys.clear();
    endResetModel();
    emit countChanged();
    m_ctxEarlier.clear();
    m_ctxLater.clear();
    emit contextChanged();
    sendRequest(Initial);
}

void JourneyModel::refresh()
{
    if (!hasSearch()) {
        return;
    }
    beginResetModel();
    m_journeys.clear();
    endResetModel();
    emit countChanged();
    m_ctxEarlier.clear();
    m_ctxLater.clear();
    emit contextChanged();
    sendRequest(Initial);
}

void JourneyModel::searchEarlier()
{
    if (m_busy || m_ctxEarlier.isEmpty()) {
        return;
    }
    sendRequest(Earlier);
}

void JourneyModel::searchLater()
{
    if (m_busy || m_ctxLater.isEmpty()) {
        return;
    }
    sendRequest(Later);
}

void JourneyModel::cancel()
{
    if (m_reply) {
        HafasClient::abort(m_reply.data());
        m_reply = 0;   // QPointer::clear gibt es erst ab Qt 5
    }
    setBusy(false);
}

QVariantMap JourneyModel::journey(int row) const
{
    if (row < 0 || row >= m_journeys.size()) {
        return QVariantMap();
    }
    return m_journeys.at(row).toVariant();
}

QJsonObject JourneyModel::buildRequest(Mode mode) const
{
    QJsonObject req;
    if (mode == Earlier) {
        req.insert(QStringLiteral("ctxScr"), m_ctxEarlier);
    } else if (mode == Later) {
        req.insert(QStringLiteral("ctxScr"), m_ctxLater);
    } else {
        req.insert(QStringLiteral("outDate"), m_when.toString(QStringLiteral("yyyyMMdd")));
        req.insert(QStringLiteral("outTime"), m_when.toString(QStringLiteral("HHmmss")));
    }
    req.insert(QStringLiteral("outFrwd"), m_isDeparture);
    req.insert(QStringLiteral("depLocL"), QJsonArray() << m_from.toRequestJson());
    req.insert(QStringLiteral("arrLocL"), QJsonArray() << m_to.toRequestJson());
    if (m_via.isValid()) {
        QJsonObject via;
        via.insert(QStringLiteral("loc"), m_via.toRequestJson());
        req.insert(QStringLiteral("viaLocL"), QJsonArray() << via);
    }

    QJsonArray jnyFltrL;
    QJsonObject prod;
    prod.insert(QStringLiteral("type"), QStringLiteral("PROD"));
    prod.insert(QStringLiteral("mode"), QStringLiteral("INC"));
    prod.insert(QStringLiteral("value"), QString::number(m_settings->products() & HafasProducts::all));
    jnyFltrL.append(prod);
    if (m_settings->bicycle()) {
        QJsonObject bc;
        bc.insert(QStringLiteral("type"), QStringLiteral("BC"));
        bc.insert(QStringLiteral("mode"), QStringLiteral("INC"));
        jnyFltrL.append(bc);
    }
    if (m_settings->wheelchair()) {
        QJsonObject hs;
        hs.insert(QStringLiteral("type"), QStringLiteral("ATTRJ"));
        hs.insert(QStringLiteral("mode"), QStringLiteral("INC"));
        hs.insert(QStringLiteral("value"), QStringLiteral("HS"));
        jnyFltrL.append(hs);
    }
    if (m_settings->einfachRaus()) {
        QJsonObject er;
        er.insert(QStringLiteral("type"), QStringLiteral("META"));
        er.insert(QStringLiteral("mode"), QStringLiteral("INC"));
        er.insert(QStringLiteral("meta"), QStringLiteral("EINFACH_RAUS"));
        jnyFltrL.append(er);
    }
    req.insert(QStringLiteral("jnyFltrL"), jnyFltrL);

    QJsonObject gis;
    gis.insert(QStringLiteral("type"), QStringLiteral("M"));
    gis.insert(QStringLiteral("mode"), QStringLiteral("FB"));
    gis.insert(QStringLiteral("meta"), QStringLiteral("foot_speed_normal"));
    req.insert(QStringLiteral("gisFltrL"), QJsonArray() << gis);

    req.insert(QStringLiteral("maxChg"), m_settings->directOnly() ? 0 : -1);
    if (m_settings->minChangeTime() > 0) {
        req.insert(QStringLiteral("minChgTime"), m_settings->minChangeTime());
    }
    req.insert(QStringLiteral("getPasslist"), true);
    req.insert(QStringLiteral("getPolyline"), true);
    req.insert(QStringLiteral("getTariff"), false);
    req.insert(QStringLiteral("getPT"), true);
    req.insert(QStringLiteral("getIV"), false);
    req.insert(QStringLiteral("getEco"), false);
    req.insert(QStringLiteral("getIST"), false);
    req.insert(QStringLiteral("ushrp"), true);
    req.insert(QStringLiteral("extChgTime"), -1);
    return req;
}

void JourneyModel::sendRequest(Mode mode)
{
    if (m_reply) {
        HafasClient::abort(m_reply.data());
        m_reply = 0;   // QPointer::clear gibt es erst ab Qt 5
    }
    setError(QString());
    setBusy(true);

    QPointer<JourneyModel> self(this);
    m_reply = m_client->request(QStringLiteral("TripSearch"), buildRequest(mode),
        [self, mode](const QJsonObject &res, const QString &errCode, const QString &errText) {
            if (!self) {
                return;
            }
            self->m_reply = 0;   // QPointer::clear gibt es erst ab Qt 5
            self->setBusy(false);
            if (!errCode.isEmpty()) {
                self->setError(errText);
                return;
            }
            HafasCommon common(res);
            const QJsonArray outConL = res.value(QStringLiteral("outConL")).toArray();
            QList<HafasJourney> parsed;
            QSet<QString> known;
            for (const HafasJourney &j : self->m_journeys) {
                known.insert(j.ctxRecon.isEmpty() ? j.dep.scheduled.toString() + j.arr.scheduled.toString()
                                                  : j.ctxRecon);
            }
            for (int i = 0; i < outConL.size(); ++i) {
                HafasJourney j = HafasParser::parseJourney(outConL.at(i).toObject(), common);
                const QString key = j.ctxRecon.isEmpty() ? j.dep.scheduled.toString() + j.arr.scheduled.toString()
                                                         : j.ctxRecon;
                if (mode != Initial && known.contains(key)) {
                    continue;
                }
                known.insert(key);
                parsed.append(j);
            }

            if (mode == Initial) {
                self->beginResetModel();
                self->m_journeys = parsed;
                self->endResetModel();
                self->m_ctxEarlier = res.value(QStringLiteral("outCtxScrB")).toString();
                self->m_ctxLater = res.value(QStringLiteral("outCtxScrF")).toString();
                if (parsed.isEmpty()) {
                    self->setError(QStringLiteral("Keine Verbindungen gefunden."));
                }
            } else if (mode == Earlier) {
                if (!parsed.isEmpty()) {
                    self->beginInsertRows(QModelIndex(), 0, parsed.size() - 1);
                    for (int i = parsed.size() - 1; i >= 0; --i) {
                        self->m_journeys.prepend(parsed.at(i));
                    }
                    self->endInsertRows();
                    // The first row after the inserted block may lose its day-change marker
                    if (self->m_journeys.size() > parsed.size()) {
                        const QModelIndex idx = self->index(parsed.size());
                        emit self->dataChanged(idx, idx);
                    }
                }
                const QString ctx = res.value(QStringLiteral("outCtxScrB")).toString();
                if (!ctx.isEmpty()) {
                    self->m_ctxEarlier = ctx;
                }
            } else {
                if (!parsed.isEmpty()) {
                    const int first = self->m_journeys.size();
                    self->beginInsertRows(QModelIndex(), first, first + parsed.size() - 1);
                    self->m_journeys.append(parsed);
                    self->endInsertRows();
                }
                const QString ctx = res.value(QStringLiteral("outCtxScrF")).toString();
                if (!ctx.isEmpty()) {
                    self->m_ctxLater = ctx;
                }
            }
            const QString ts = res.value(QStringLiteral("planrtTS")).toString();
            if (!ts.isEmpty() && ts != QLatin1String("0")) {
                self->m_realtimeUpdated = QDateTime::fromMSecsSinceEpoch(ts.toLongLong() * 1000)
                                              .toLocalTime().toString(QStringLiteral("HH:mm"));
            }
            emit self->countChanged();
            emit self->contextChanged();
        });
}
