#include "hafasclient.h"
#include "appsettings.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPointer>
#include <QTimer>
#include <QUrl>

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#include "qt4replyhandler.h"
#endif

namespace
{
    const int kTimeoutMs = 30000;
}

HafasClient::HafasClient(AppSettings *settings, QObject *parent)
    : QObject(parent)
    , m_settings(settings)
{
}

QJsonObject HafasClient::buildEnvelope(const QString &method, const QJsonObject &req) const
{
    QJsonObject svc;
    svc.insert(QStringLiteral("meth"), method);
    QJsonObject cfg;
    cfg.insert(QStringLiteral("polyEnc"), QStringLiteral("GPA"));
    svc.insert(QStringLiteral("cfg"), cfg);
    svc.insert(QStringLiteral("req"), req);

    QJsonObject env;
    env.insert(QStringLiteral("ver"), m_settings->version());
    if (!m_settings->ext().isEmpty()) {
        env.insert(QStringLiteral("ext"), m_settings->ext());
    }
    env.insert(QStringLiteral("lang"), m_settings->language());

    QJsonObject auth;
    auth.insert(QStringLiteral("type"), QStringLiteral("AID"));
    auth.insert(QStringLiteral("aid"), m_settings->aid());
    env.insert(QStringLiteral("auth"), auth);

    QJsonObject client;
    client.insert(QStringLiteral("id"), m_settings->clientId());
    client.insert(QStringLiteral("type"), m_settings->clientType());
    if (!m_settings->clientVersion().isEmpty()) {
        client.insert(QStringLiteral("v"), m_settings->clientVersion());
    }
    if (!m_settings->clientName().isEmpty()) {
        client.insert(QStringLiteral("name"), m_settings->clientName());
    }
    env.insert(QStringLiteral("client"), client);

    env.insert(QStringLiteral("formatted"), false);
    env.insert(QStringLiteral("svcReqL"), QJsonArray() << svc);
    return env;
}

QNetworkReply *HafasClient::request(const QString &method, const QJsonObject &req, const Callback &callback)
{
    const QByteArray body = QJsonDocument(buildEnvelope(method, req)).toJson(QJsonDocument::Compact);

    QNetworkRequest netReq(QUrl(m_settings->endpoint()));
    netReq.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json; charset=utf-8"));
    netReq.setRawHeader("Accept", "application/json");
    netReq.setRawHeader("Accept-Language", "de");
    if (!m_settings->userAgent().isEmpty()) {
        netReq.setRawHeader("User-Agent", m_settings->userAgent().toUtf8());
    }
    netReq.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    if (m_settings->logRequests()) {
        qDebug().noquote() << "HAFAS >>" << method << QString::fromUtf8(body);
        emit requestLogged(QStringLiteral(">> %1 %2").arg(method, QString::fromUtf8(body)));
    }

    QNetworkReply *reply = m_nam.post(netReq, body);
    ++m_pending;
    emit pendingRequestsChanged();

    QTimer *timeout = new QTimer(reply);
    timeout->setSingleShot(true);
    timeout->setInterval(kTimeoutMs);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    // Die Zeichenkettenform, nicht die Zeigerform: Qt 4.7 kennt nur jene.
    connect(timeout, SIGNAL(timeout()), reply, SLOT(abort()));
#endif
    timeout->start();

    QPointer<HafasClient> self(this);
    const bool logResponses = m_settings->logRequests();
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    connect(reply, &QNetworkReply::finished, this, [self, reply, callback, method, logResponses]() {
        replyFinished(self, reply, callback, method, logResponses);
    });
#else
    // Qt 4.7 kann an keine Lambda binden; das erledigt das Hilfsobjekt. Es
    // bricht auch bei Zeitueberschreitung ab: QNetworkReply::abort() ist hier
    // noch kein Slot, ein connect darauf geht wortlos daneben.
    new Qt4ReplyHandler(this, reply, callback, method, logResponses, timeout);
#endif

    return reply;
}

void HafasClient::replyFinished(const QPointer<HafasClient> &self, QNetworkReply *reply,
                                const Callback &callback, const QString &method,
                                bool logResponses)
{
    {
        if (self) {
            --self->m_pending;
            emit self->pendingRequestsChanged();
        }
        reply->deleteLater();

        if (reply->error() == QNetworkReply::OperationCanceledError) {
            // Either timeout or aborted by the caller. The caller aborts only
            // when it no longer cares, so a timeout is the only case to report.
            if (reply->property("hafas_userAbort").toBool()) {
                return;
            }
            callback(QJsonObject(), QStringLiteral("TIMEOUT"),
                     QStringLiteral("Der Server hat nicht rechtzeitig geantwortet."));
            return;
        }

        const QByteArray data = reply->readAll();
        const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        if (logResponses && self) {
            qDebug().noquote() << "HAFAS <<" << method << httpStatus << QString::fromUtf8(data.left(4000));
            emit self->requestLogged(QStringLiteral("<< %1 %2 %3").arg(method).arg(httpStatus).arg(QString::fromUtf8(data.left(4000))));
        }

        if (reply->error() != QNetworkReply::NoError && data.isEmpty()) {
            callback(QJsonObject(), QStringLiteral("NETWORK"),
                     QStringLiteral("Netzwerkfehler: %1").arg(reply->errorString()));
            return;
        }

        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            callback(QJsonObject(), QStringLiteral("PARSE"),
                     httpStatus >= 400
                         ? QStringLiteral("Server antwortete mit HTTP %1.").arg(httpStatus)
                         : QStringLiteral("Antwort des Servers konnte nicht gelesen werden."));
            return;
        }

        const QJsonObject root = doc.object();
        const QString err = root.value(QStringLiteral("err")).toString();
        if (!err.isEmpty() && err != QLatin1String("OK")) {
            callback(QJsonObject(), err,
                     friendlyError(err, root.value(QStringLiteral("errTxt")).toString(),
                                   root.value(QStringLiteral("errTxtOut")).toString()));
            return;
        }

        const QJsonArray svcResL = root.value(QStringLiteral("svcResL")).toArray();
        QJsonObject svcRes;
        for (int i = 0; i < svcResL.size(); ++i) {
            const QJsonObject o = svcResL.at(i).toObject();
            if (o.value(QStringLiteral("meth")).toString() == method) {
                svcRes = o;
                break;
            }
        }
        if (svcRes.isEmpty() && !svcResL.isEmpty()) {
            svcRes = svcResL.first().toObject();
        }
        if (svcRes.isEmpty()) {
            callback(QJsonObject(), QStringLiteral("EMPTY"),
                     QStringLiteral("Der Server hat keine Daten geliefert."));
            return;
        }

        const QString svcErr = svcRes.value(QStringLiteral("err")).toString();
        if (!svcErr.isEmpty() && svcErr != QLatin1String("OK")) {
            callback(QJsonObject(), svcErr,
                     friendlyError(svcErr, svcRes.value(QStringLiteral("errTxt")).toString(),
                                   svcRes.value(QStringLiteral("errTxtOut")).toString()));
            return;
        }

        callback(svcRes.value(QStringLiteral("res")).toObject(), QString(), QString());
    }
}

void HafasClient::abort(QNetworkReply *reply)
{
    if (!reply) {
        return;
    }
    reply->setProperty("hafas_userAbort", true);
    reply->abort();
}

QString HafasClient::friendlyError(const QString &code, const QString &errTxt, const QString &errTxtOut)
{
    struct Entry
    {
        const char *code;
        const char *text;
    };
    static const Entry entries[] = {
        { "AUTH", "Zugriff verweigert (Authentifizierung). Bitte Profil in den Einstellungen prüfen." },
        { "R5000", "Zugriff verweigert. Bitte Profil in den Einstellungen prüfen." },
        { "METHOD_NA", "Diese Funktion wird vom Server nicht angeboten." },
        { "R0001", "Unbekannte Anfrage (Protokollversion?)." },
        { "R0002", "Ungültige Anfrageparameter." },
        { "R0007", "Interner Kommunikationsfehler des Servers." },
        { "S1", "Keine Verbindung zum Fahrplanserver möglich." },
        { "PROBLEMS", "Bei der Suche ist ein Problem aufgetreten." },
        { "LOCATION", "Haltestelle oder Adresse nicht gefunden." },
        { "NO_MATCH", "Nichts gefunden." },
        { "PARAMETER", "Ungültiger Parameter." },
        { "H390", "Start oder Ziel wurde durch eine andere Haltestelle ersetzt." },
        { "H410", "Unvollständige Antwort wegen Fahrplanwechsels." },
        { "H455", "Zu langer Aufenthalt." },
        { "H460", "Haltestellen werden mehrfach durchfahren." },
        { "H500", "Zu viele Züge, Verbindung unvollständig." },
        { "H890", "Keine Verbindungen gefunden." },
        { "H891", "Keine Route gefunden – bitte einen Via-Halt angeben." },
        { "H892", "Anfrage zu komplex – weniger Zwischenhalte angeben." },
        { "H895", "Start und Ziel liegen zu nahe beieinander." },
        { "H899", "Suche wegen Fahrplanwechsels nicht möglich oder unvollständig." },
        { "H900", "Suche wegen Fahrplanwechsels nicht möglich oder unvollständig." },
        { "H9220", "Keine Haltestellen in der Nähe der Adresse gefunden." },
        { "H9230", "Interner Fehler bei der Suche." },
        { "H9240", "Keine Verbindungen gefunden." },
        { "H9250", "Suche unterbrochen." },
        { "H9260", "Unbekannte Abfahrtshaltestelle." },
        { "H9280", "Unbekannter Zwischenhalt." },
        { "H9300", "Unbekannte Zielhaltestelle." },
        { "H9320", "Eingabe unvollständig oder ungültig." },
        { "H9360", "Datum außerhalb der Fahrplanperiode." },
        { "H9380", "Start, Ziel oder Via sind zu nahe beieinander." },
        { "H_UNKNOWN", "Unbekannter interner Fehler." },
    };
    for (size_t i = 0; i < sizeof(entries) / sizeof(entries[0]); ++i) {
        if (code == QLatin1String(entries[i].code)) {
            QString text = QString::fromUtf8(entries[i].text);
            if (!errTxtOut.isEmpty()) {
                text += QStringLiteral(" (%1)").arg(errTxtOut);
            }
            return text;
        }
    }
    if (!errTxtOut.isEmpty()) {
        return errTxtOut;
    }
    if (!errTxt.isEmpty()) {
        return QStringLiteral("%1: %2").arg(code, errTxt);
    }
    return QStringLiteral("Fehler %1").arg(code);
}
