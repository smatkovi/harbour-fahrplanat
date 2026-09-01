#ifndef JOURNEYMODEL_H
#define JOURNEYMODEL_H

#include "hafastypes.h"

#include <QAbstractListModel>
#include <QDateTime>
#include <QList>
#include <QPointer>
#include <QVariantMap>

class AppSettings;
class HafasClient;
class QNetworkReply;

class JourneyModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(bool canScrollEarlier READ canScrollEarlier NOTIFY contextChanged)
    Q_PROPERTY(bool canScrollLater READ canScrollLater NOTIFY contextChanged)
    Q_PROPERTY(QString fromName READ fromName NOTIFY searchChanged)
    Q_PROPERTY(QString toName READ toName NOTIFY searchChanged)
    Q_PROPERTY(QString viaName READ viaName NOTIFY searchChanged)
    Q_PROPERTY(QString whenText READ whenText NOTIFY searchChanged)
    Q_PROPERTY(bool hasSearch READ hasSearch NOTIFY searchChanged)
    Q_PROPERTY(QString realtimeUpdated READ realtimeUpdated NOTIFY contextChanged)
    Q_PROPERTY(QString summary READ summary NOTIFY countChanged)

public:
    enum Roles {
        DepTimeRole = Qt::UserRole + 1,
        DepRealTimeRole,
        DepDelayRole,
        DepDelayTextRole,
        DepHasRealtimeRole,
        ArrTimeRole,
        ArrRealTimeRole,
        ArrDelayRole,
        ArrDelayTextRole,
        ArrHasRealtimeRole,
        DurationRole,
        ChangesRole,
        LinesRole,
        DepPlatformRole,
        DepPlatformChangedRole,
        ArrPlatformRole,
        CancelledRole,
        WarningCountRole,
        DateTextRole,
        OriginRole,
        DestinationRole,
        DayChangeRole,
        LegsSummaryRole
    };

    JourneyModel(HafasClient *client, AppSettings *settings, QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

    bool busy() const { return m_busy; }
    QString error() const { return m_error; }
    bool canScrollEarlier() const { return !m_ctxEarlier.isEmpty(); }
    bool canScrollLater() const { return !m_ctxLater.isEmpty(); }
    QString fromName() const { return m_from.name; }
    QString toName() const { return m_to.name; }
    QString viaName() const { return m_via.name; }
    QString whenText() const;
    bool hasSearch() const { return m_from.isValid() && m_to.isValid(); }
    QString realtimeUpdated() const { return m_realtimeUpdated; }
    QString summary() const;

    // `when` is interpreted as local wall-clock time of the network.
    Q_INVOKABLE void search(const QVariantMap &from, const QVariantMap &to, const QVariantMap &via,
                            const QDateTime &when, bool isDeparture);
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void searchEarlier();
    Q_INVOKABLE void searchLater();
    Q_INVOKABLE void cancel();
    Q_INVOKABLE QVariantMap journey(int row) const;

signals:
    void busyChanged();
    void errorChanged();
    void countChanged();
    void contextChanged();
    void searchChanged();

private:
    enum Mode { Initial, Earlier, Later };
    void sendRequest(Mode mode);
    QJsonObject buildRequest(Mode mode) const;
    void setBusy(bool busy);
    void setError(const QString &error);

    HafasClient *m_client;
    AppSettings *m_settings;
    QList<HafasJourney> m_journeys;
    HafasLocation m_from;
    HafasLocation m_to;
    HafasLocation m_via;
    QDateTime m_when;
    bool m_isDeparture = true;
    QString m_ctxEarlier;
    QString m_ctxLater;
    QString m_realtimeUpdated;
    QPointer<QNetworkReply> m_reply;
    bool m_busy = false;
    QString m_error;
};

#endif // JOURNEYMODEL_H
