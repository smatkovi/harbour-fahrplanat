#include "himmodel.h"
#include "hafasclient.h"
#include "hafasparser.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkReply>

#include <algorithm>

HimModel::HimModel(HafasClient *client, QObject *parent)
    : QAbstractListModel(parent)
    , m_client(client)
{
    // Qt 4.7 fragt ein Modell nicht nach roleNames(): dort ist das ein
    // gewoehnlicher Zugriff, kein virtueller. Die Namen muessen einmal
    // gesetzt werden, sonst kennt QML keine einzige Rolle -- die Liste
    // bleibt dann leer, ohne eine Meldung.
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    setRoleNames(roleNames());
#endif
}

int HimModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_warnings.size();
}

QVariant HimModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_warnings.size()) {
        return QVariant();
    }
    const HafasWarning &w = m_warnings.at(index.row());
    switch (role) {
    case IdRole: return w.hid;
    case HeadRole: return HafasFormat::stripHtml(w.head);
    case LeadRole: {
        QString lead = HafasFormat::stripHtml(w.lead);
        if (lead.isEmpty()) {
            lead = HafasFormat::stripHtml(w.text);
        }
        return lead;
    }
    case TextRole: return HafasFormat::htmlToStyled(w.text);
    case CompanyRole: return w.company;
    case PrioRole: return w.prio;
    case ProductsRole: return HafasFormat::productNames(w.prod);
    case ValidFromRole: return HafasFormat::dateTime(w.validFrom);
    case ValidUntilRole: return HafasFormat::dateTime(w.validUntil);
    case ModifiedRole: return HafasFormat::dateTime(w.modified);
    case AffectedLinesRole: return w.affectedLines.join(QStringLiteral(", "));
    case FromNameRole: return w.fromName;
    case ToNameRole: return w.toName;
    case WarningRole: return w.toVariant();
    }
    return QVariant();
}

QHash<int, QByteArray> HimModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[HeadRole] = "head";
    roles[LeadRole] = "lead";
    roles[TextRole] = "text";
    roles[CompanyRole] = "company";
    roles[PrioRole] = "prio";
    roles[ProductsRole] = "products";
    roles[ValidFromRole] = "validFrom";
    roles[ValidUntilRole] = "validUntil";
    roles[ModifiedRole] = "modified";
    roles[AffectedLinesRole] = "affectedLines";
    roles[FromNameRole] = "fromName";
    roles[ToNameRole] = "toName";
    roles[WarningRole] = "warning";
    return roles;
}

void HimModel::setBusy(bool busy)
{
    if (m_busy != busy) {
        m_busy = busy;
        emit busyChanged();
    }
}

void HimModel::setError(const QString &error)
{
    if (m_error != error) {
        m_error = error;
        emit errorChanged();
    }
}

void HimModel::setSortMode(int mode)
{
    if (m_sortMode == mode) {
        return;
    }
    m_sortMode = mode;
    emit sortModeChanged();
    beginResetModel();
    applySort();
    endResetModel();
}

namespace
{
    bool byPriority(const HafasWarning &a, const HafasWarning &b)
    {
        if (a.prio != b.prio) {
            return a.prio > b.prio;
        }
        return a.modified > b.modified;
    }
    bool byModified(const HafasWarning &a, const HafasWarning &b)
    {
        return a.modified > b.modified;
    }
    bool byTitle(const HafasWarning &a, const HafasWarning &b)
    {
        return QString::localeAwareCompare(a.head, b.head) < 0;
    }
}

void HimModel::applySort()
{
    switch (m_sortMode) {
    case ByModified: std::stable_sort(m_warnings.begin(), m_warnings.end(), byModified); break;
    case ByTitle: std::stable_sort(m_warnings.begin(), m_warnings.end(), byTitle); break;
    default: std::stable_sort(m_warnings.begin(), m_warnings.end(), byPriority); break;
    }
}

QVariantMap HimModel::get(int row) const
{
    if (row < 0 || row >= m_warnings.size()) {
        return QVariantMap();
    }
    return m_warnings.at(row).toVariant();
}

void HimModel::load()
{
    if (m_reply) {
        HafasClient::abort(m_reply.data());
        m_reply = 0;   // QPointer::clear gibt es erst ab Qt 5
    }
    const QDateTime now = QDateTime::currentDateTime();
    QJsonObject req;
    req.insert(QStringLiteral("himFltrL"), QJsonArray());
    req.insert(QStringLiteral("getPolyline"), false);
    req.insert(QStringLiteral("maxNum"), 500);
    req.insert(QStringLiteral("dateB"), now.toString(QStringLiteral("yyyyMMdd")));
    req.insert(QStringLiteral("timeB"), now.toString(QStringLiteral("HHmmss")));

    setError(QString());
    setBusy(true);
    QPointer<HimModel> self(this);
    m_reply = m_client->request(QStringLiteral("HimSearch"), req,
        [self](const QJsonObject &res, const QString &errCode, const QString &errText) {
            if (!self) {
                return;
            }
            self->m_reply = 0;   // QPointer::clear gibt es erst ab Qt 5
            self->setBusy(false);
            if (!errCode.isEmpty()) {
                if (errCode == QLatin1String("NO_MATCH")) {
                    self->beginResetModel();
                    self->m_warnings.clear();
                    self->endResetModel();
                    self->m_loaded = true;
                    emit self->countChanged();
                } else {
                    self->setError(errText);
                }
                return;
            }
            self->beginResetModel();
            self->m_warnings = HafasParser::parseHimSearch(res);
            self->applySort();
            self->endResetModel();
            self->m_loaded = true;
            self->m_lastUpdate = QDateTime::currentDateTime().toString(QStringLiteral("HH:mm"));
            emit self->countChanged();
        });
}
