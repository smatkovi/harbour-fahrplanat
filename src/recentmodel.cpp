#include "recentmodel.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace
{
    const int kMaxEntries = 40;

    struct Seed
    {
        const char *name;
        const char *lid;
        const char *extId;
        double lat;
        double lon;
        int products;
    };

    // Major stations, used as initial suggestions until the user has a history
    const Seed seeds[] = {
        { "Wien", "A=1@O=Wien@X=16372134@Y=48208547@U=181@L=001190100@B=1@", "001190100", 48.208547, 16.372134, 7805 },
        { "Graz", "A=1@O=Graz@X=15417093@Y=47071816@U=81@L=001160100@B=1@", "001160100", 47.071816, 15.417093, 1663 },
        { "Linz/Donau Hbf", "A=1@O=Linz/Donau Hbf@X=14291815@Y=48290151@U=181@L=008100013@B=1@", "008100013", 48.290160, 14.291941, 4157 },
        { "Salzburg Hbf", "A=1@O=Salzburg Hbf@X=13045559@Y=47812824@U=81@L=008100002@B=1@", "008100002", 47.812851, 13.045604, 4159 },
        { "Innsbruck Hbf", "A=1@O=Innsbruck Hbf@X=11401019@Y=47263043@U=181@L=008100108@B=1@", "008100108", 47.263412, 11.401091, 1085 },
        { "Klagenfurt Hbf", "A=1@O=Klagenfurt Hbf@X=14313578@Y=46615604@U=181@L=008100085@B=1@", "008100085", 46.615622, 14.313686, 61 },
        { "Sankt Pölten", "A=1@O=Sankt Pölten@X=15623099@Y=48208178@U=181@L=001130201@B=1@", "001130201", 48.208178, 15.623099, 4221 },
        { "Villach Hbf", "A=1@O=Villach Hbf@X=13848710@Y=46618625@U=181@L=008100147@B=1@", "008100147", 46.618697, 13.848817, 1085 },
        { "Bregenz", "A=1@O=Bregenz@X=9739579@Y=47502885@U=181@L=001180207@B=1@", "001180207", 47.502885, 9.739579, 127 },
        { "Eisenstadt", "A=1@O=Eisenstadt@X=16529500@Y=47839423@U=181@L=001110101@B=1@", "001110101", 47.839423, 16.529500, 80 },
    };
}

RecentModel::RecentModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_settings(QStringLiteral("harbour-fahrplanat"), QStringLiteral("harbour-fahrplanat"))
{
    // Qt 4.7 fragt ein Modell nicht nach roleNames(): dort ist das ein
    // gewoehnlicher Zugriff, kein virtueller. Die Namen muessen einmal
    // gesetzt werden, sonst kennt QML keine einzige Rolle -- die Liste
    // bleibt dann leer, ohne eine Meldung.
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    setRoleNames(roleNames());
#endif
    load();
}

int RecentModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_entries.size();
}

QVariant RecentModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size()) {
        return QVariant();
    }
    const Entry &e = m_entries.at(index.row());
    const HafasLocation &l = e.loc;
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
    case FavoriteRole: return e.favorite;
    }
    return QVariant();
}

QHash<int, QByteArray> RecentModel::roleNames() const
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
    roles[FavoriteRole] = "favorite";
    return roles;
}

int RecentModel::indexOf(const HafasLocation &loc) const
{
    for (int i = 0; i < m_entries.size(); ++i) {
        const HafasLocation &e = m_entries.at(i).loc;
        if (!loc.extId.isEmpty() && e.extId == loc.extId && e.type == loc.type) {
            return i;
        }
        if (!loc.lid.isEmpty() && e.lid == loc.lid) {
            return i;
        }
        if (loc.extId.isEmpty() && loc.lid.isEmpty() && e.name == loc.name && e.type == loc.type) {
            return i;
        }
    }
    return -1;
}

void RecentModel::sortEntries()
{
    // Stable partition: favourites first, keep relative order otherwise
    QList<Entry> favs;
    QList<Entry> rest;
    for (const Entry &e : m_entries) {
        (e.favorite ? favs : rest).append(e);
    }
    m_entries = favs + rest;
}

void RecentModel::add(const QVariantMap &location)
{
    const HafasLocation loc = HafasLocation::fromVariant(location);
    if (!loc.isValid() || loc.name.isEmpty()) {
        return;
    }
    beginResetModel();
    const int existing = indexOf(loc);
    Entry e;
    if (existing >= 0) {
        e = m_entries.takeAt(existing);
        e.loc = loc;
    } else {
        e.loc = loc;
    }
    // Insert as most recent (after the favourites block)
    int pos = 0;
    while (pos < m_entries.size() && m_entries.at(pos).favorite) {
        ++pos;
    }
    if (e.favorite) {
        pos = 0;
    }
    m_entries.insert(pos, e);
    while (m_entries.size() > kMaxEntries) {
        // drop the oldest non-favourite
        int drop = -1;
        for (int i = m_entries.size() - 1; i >= 0; --i) {
            if (!m_entries.at(i).favorite) {
                drop = i;
                break;
            }
        }
        if (drop < 0) {
            break;
        }
        m_entries.removeAt(drop);
    }
    endResetModel();
    save();
    emit countChanged();
}

void RecentModel::remove(int row)
{
    if (row < 0 || row >= m_entries.size()) {
        return;
    }
    beginRemoveRows(QModelIndex(), row, row);
    m_entries.removeAt(row);
    endRemoveRows();
    save();
    emit countChanged();
}

void RecentModel::toggleFavorite(int row)
{
    if (row < 0 || row >= m_entries.size()) {
        return;
    }
    beginResetModel();
    m_entries[row].favorite = !m_entries.at(row).favorite;
    sortEntries();
    endResetModel();
    save();
}

void RecentModel::clearHistory()
{
    beginResetModel();
    QList<Entry> favs;
    for (const Entry &e : m_entries) {
        if (e.favorite) {
            favs.append(e);
        }
    }
    m_entries = favs;
    endResetModel();
    save();
    emit countChanged();
}

QVariantMap RecentModel::get(int row) const
{
    if (row < 0 || row >= m_entries.size()) {
        return QVariantMap();
    }
    QVariantMap m = m_entries.at(row).loc.toVariant();
    m.insert(QStringLiteral("favorite"), m_entries.at(row).favorite);
    return m;
}

void RecentModel::load()
{
    m_entries.clear();
    const QByteArray raw = m_settings.value(QStringLiteral("history/locations")).toByteArray();
    if (!raw.isEmpty()) {
        const QJsonArray arr = QJsonDocument::fromJson(raw).array();
        for (int i = 0; i < arr.size(); ++i) {
            const QJsonObject o = arr.at(i).toObject();
            Entry e;
            e.loc = HafasLocation::fromVariant(o.value(QStringLiteral("location")).toObject().toVariantMap());
            e.favorite = o.value(QStringLiteral("favorite")).toBool(false);
            if (e.loc.isValid()) {
                m_entries.append(e);
            }
        }
    }
    if (m_entries.isEmpty() && !m_settings.value(QStringLiteral("history/seeded"), false).toBool()) {
        for (size_t i = 0; i < sizeof(seeds) / sizeof(seeds[0]); ++i) {
            Entry e;
            e.loc.type = QStringLiteral("S");
            e.loc.name = QString::fromUtf8(seeds[i].name);
            e.loc.lid = QString::fromUtf8(seeds[i].lid);
            e.loc.extId = QString::fromUtf8(seeds[i].extId);
            e.loc.lat = seeds[i].lat;
            e.loc.lon = seeds[i].lon;
            e.loc.hasCoord = true;
            e.loc.products = seeds[i].products;
            m_entries.append(e);
        }
        m_settings.setValue(QStringLiteral("history/seeded"), true);
    }
    sortEntries();
}

void RecentModel::save()
{
    QJsonArray arr;
    for (const Entry &e : m_entries) {
        QJsonObject o;
        o.insert(QStringLiteral("location"), QJsonObject::fromVariantMap(e.loc.toVariant()));
        o.insert(QStringLiteral("favorite"), e.favorite);
        arr.append(o);
    }
    m_settings.setValue(QStringLiteral("history/locations"), QJsonDocument(arr).toJson(QJsonDocument::Compact));
    m_settings.sync();
}
