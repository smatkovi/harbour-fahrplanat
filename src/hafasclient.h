#ifndef HAFASCLIENT_H
#define HAFASCLIENT_H

#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QObject>
#include <QString>

#include <functional>

class AppSettings;
class QNetworkReply;

// Thin client for the HAFAS "HCI" JSON interface (mgate). One service request
// per HTTP call. The callback receives the `res` object of the service result
// on success, or empty `res` plus an error code and a human readable message.
class HafasClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int pendingRequests READ pendingRequests NOTIFY pendingRequestsChanged)

public:
    typedef std::function<void(const QJsonObject &res, const QString &errorCode, const QString &errorText)> Callback;

    explicit HafasClient(AppSettings *settings, QObject *parent = 0);

    // Returns the reply so that callers can abort it. The callback is always
    // invoked exactly once unless the reply is aborted by the caller.
    QNetworkReply *request(const QString &method, const QJsonObject &req, const Callback &callback);

    int pendingRequests() const { return m_pending; }

    // Aborts a request without invoking its callback.
    static void abort(QNetworkReply *reply);

    // Translates HAFAS error codes to German messages.
    static QString friendlyError(const QString &code, const QString &errTxt, const QString &errTxtOut);

signals:
    void pendingRequestsChanged();
    void requestLogged(const QString &line);

private:
    QJsonObject buildEnvelope(const QString &method, const QJsonObject &req) const;

    AppSettings *m_settings;
    QNetworkAccessManager m_nam;
    int m_pending = 0;
};

#endif // HAFASCLIENT_H
