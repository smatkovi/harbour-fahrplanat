#include "locationmodel.h"
#include "hafasclient.h"
#include "hafasparser.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkReply>

LocationModel::LocationModel(HafasClient *client, QObject *parent)
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
    m_debounce.setSingleShot(true);
    m_debounce.setInterval(300);
    connect(&m_debounce, SIGNAL(timeout()), this, SLOT(sendRequest()));
}

int LocationModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_locations.size();
}

QVariant LocationModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_locations.size()) {
        return QVariant();
    }
    const HafasLocation &l = m_locations.at(index.row());
    switch (role) {
    case NameRole: return l.name;
    case TypeRole: return l.type;
    case LidRole: return l.lid;
    case ExtIdRole: return l.extId;
    case LatRole: return l.lat;
    case LonRole: return l.lon;
    case ProductsRole: return l.products;
    case DescriptionRole: return l.toVariant().value(QStringLiteral("description"));
    case IsStationRole: return l.type == QLatin1String("S");
    case LocationRole: return l.toVariant();
    }
    return QVariant();
}

QHash<int, QByteArray> LocationModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[TypeRole] = "type";
    roles[LidRole] = "lid";
    roles[ExtIdRole] = "extId";
    roles[LatRole] = "lat";
    roles[LonRole] = "lon";
    roles[ProductsRole] = "products";
    roles[DescriptionRole] = "description";
    roles[IsStationRole] = "isStation";
    roles[LocationRole] = "location";
    return roles;
}

void LocationModel::setBusy(bool busy)
{
    if (m_busy != busy) {
        m_busy = busy;
        emit busyChanged();
    }
}

void LocationModel::setError(const QString &error)
{
    if (m_error != error) {
        m_error = error;
        emit errorChanged();
    }
}

void LocationModel::search(const QString &query)
{
    const QString q = query.trimmed();
    if (q == m_pendingQuery) {
        return;
    }
    m_pendingQuery = q;
    if (m_reply) {
        HafasClient::abort(m_reply.data());
        m_reply = 0;   // QPointer::clear gibt es erst ab Qt 5
    }
    if (q.length() < 2) {
        m_debounce.stop();
        setBusy(false);
        clear();
        return;
    }
    setError(QString());
    m_debounce.start();
}

void LocationModel::clear()
{
    if (!m_locations.isEmpty()) {
        beginResetModel();
        m_locations.clear();
        endResetModel();
        emit countChanged();
    }
    if (!m_query.isEmpty()) {
        m_query.clear();
        emit queryChanged();
    }
}

QVariantMap LocationModel::get(int row) const
{
    if (row < 0 || row >= m_locations.size()) {
        return QVariantMap();
    }
    return m_locations.at(row).toVariant();
}

void LocationModel::sendRequest()
{
    const QString q = m_pendingQuery;
    if (q.length() < 2) {
        return;
    }
    QJsonObject loc;
    loc.insert(QStringLiteral("type"), QStringLiteral("ALL"));
    loc.insert(QStringLiteral("name"), q + QLatin1Char('?'));
    QJsonObject input;
    input.insert(QStringLiteral("loc"), loc);
    input.insert(QStringLiteral("maxLoc"), 15);
    input.insert(QStringLiteral("field"), QStringLiteral("S"));
    QJsonObject req;
    req.insert(QStringLiteral("input"), input);

    setBusy(true);
    QPointer<LocationModel> self(this);
    m_reply = m_client->request(QStringLiteral("LocMatch"), req,
        [self, q](const QJsonObject &res, const QString &errCode, const QString &errText) {
            if (!self || q != self->m_pendingQuery) {
                return;
            }
            self->m_reply = 0;   // QPointer::clear gibt es erst ab Qt 5
            self->setBusy(false);
            if (!errCode.isEmpty()) {
                if (errCode != QLatin1String("NO_MATCH") && errCode != QLatin1String("LOCATION")) {
                    self->setError(errText);
                }
                self->beginResetModel();
                self->m_locations.clear();
                self->endResetModel();
                emit self->countChanged();
                return;
            }
            const QJsonArray locL = res.value(QStringLiteral("match")).toObject()
                                        .value(QStringLiteral("locL")).toArray();
            QList<HafasLocation> found;
            for (int i = 0; i < locL.size(); ++i) {
                HafasLocation l = HafasParser::parseLocation(locL.at(i).toObject());
                if (l.isValid() && !l.name.isEmpty()) {
                    found.append(l);
                }
            }
            self->beginResetModel();
            self->m_locations = found;
            self->endResetModel();
            self->m_query = q;
            emit self->queryChanged();
            emit self->countChanged();
        });
}
