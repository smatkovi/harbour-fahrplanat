#ifndef HIMMODEL_H
#define HIMMODEL_H

#include "hafastypes.h"

#include <QAbstractListModel>
#include <QList>
#include <QPointer>

class HafasClient;
class QNetworkReply;

class HimModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(bool loaded READ loaded NOTIFY countChanged)
    Q_PROPERTY(QString lastUpdate READ lastUpdate NOTIFY countChanged)
    Q_PROPERTY(int sortMode READ sortMode WRITE setSortMode NOTIFY sortModeChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        HeadRole,
        LeadRole,
        TextRole,
        CompanyRole,
        PrioRole,
        ProductsRole,
        ValidFromRole,
        ValidUntilRole,
        ModifiedRole,
        AffectedLinesRole,
        FromNameRole,
        ToNameRole,
        WarningRole
    };
    enum SortMode { ByPriority = 0, ByModified = 1, ByTitle = 2 };

    explicit HimModel(HafasClient *client, QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

    bool busy() const { return m_busy; }
    QString error() const { return m_error; }
    bool loaded() const { return m_loaded; }
    QString lastUpdate() const { return m_lastUpdate; }
    int sortMode() const { return m_sortMode; }
    void setSortMode(int mode);

    Q_INVOKABLE void load();
    Q_INVOKABLE QVariantMap get(int row) const;

signals:
    void busyChanged();
    void errorChanged();
    void countChanged();
    void sortModeChanged();

private:
    void setBusy(bool busy);
    void setError(const QString &error);
    void applySort();

    HafasClient *m_client;
    QList<HafasWarning> m_warnings;
    QPointer<QNetworkReply> m_reply;
    bool m_busy = false;
    bool m_loaded = false;
    QString m_error;
    QString m_lastUpdate;
    int m_sortMode = ByPriority;
};

#endif // HIMMODEL_H
