#ifndef RECENTMODEL_H
#define RECENTMODEL_H

#include "hafastypes.h"

#include <QAbstractListModel>
#include <QList>
#include <QSettings>

// History of chosen locations plus favourites. Favourites are always listed
// first; the rest is ordered by most recent use.
class RecentModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        TypeRole,
        LidRole,
        ExtIdRole,
        LatRole,
        LonRole,
        ProductsRole,
        DescriptionRole,
        IsStationRole,
        LocationRole,
        FavoriteRole
    };

    explicit RecentModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

    Q_INVOKABLE void add(const QVariantMap &location);
    Q_INVOKABLE void remove(int row);
    Q_INVOKABLE void toggleFavorite(int row);
    Q_INVOKABLE void clearHistory();
    Q_INVOKABLE QVariantMap get(int row) const;

signals:
    void countChanged();

private:
    struct Entry
    {
        HafasLocation loc;
        bool favorite = false;
    };
    void load();
    void save();
    void sortEntries();
    int indexOf(const HafasLocation &loc) const;

    QSettings m_settings;
    QList<Entry> m_entries;
};

#endif // RECENTMODEL_H
