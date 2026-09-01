#ifndef LOCATIONMODEL_H
#define LOCATIONMODEL_H

#include "hafastypes.h"

#include <QAbstractListModel>
#include <QList>
#include <QPointer>
#include <QTimer>

class HafasClient;
class QNetworkReply;

class LocationModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(QString query READ query NOTIFY queryChanged)

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
        LocationRole
    };

    explicit LocationModel(HafasClient *client, QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

    bool busy() const { return m_busy; }
    QString error() const { return m_error; }
    QString query() const { return m_query; }

    Q_INVOKABLE void search(const QString &query);
    Q_INVOKABLE void clear();
    Q_INVOKABLE QVariantMap get(int row) const;

signals:
    void busyChanged();
    void errorChanged();
    void countChanged();
    void queryChanged();

private slots:
    void sendRequest();

private:
    void setBusy(bool busy);
    void setError(const QString &error);

    HafasClient *m_client;
    QList<HafasLocation> m_locations;
    QString m_query;
    QString m_pendingQuery;
    QTimer m_debounce;
    QPointer<QNetworkReply> m_reply;
    bool m_busy = false;
    QString m_error;
};

#endif // LOCATIONMODEL_H
