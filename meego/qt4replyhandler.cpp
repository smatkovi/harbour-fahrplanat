#include "qt4replyhandler.h"

#include <QNetworkReply>
#include <QTimer>

Qt4ReplyHandler::Qt4ReplyHandler(HafasClient *client, QNetworkReply *reply,
                                 const HafasClient::Callback &callback,
                                 const QString &method, bool logResponses,
                                 QTimer *timeout)
    : QObject(reply)          // stirbt mit der Antwort
    , m_client(client)
    , m_reply(reply)
    , m_callback(callback)
    , m_method(method)
    , m_logResponses(logResponses)
{
    connect(reply, SIGNAL(finished()), this, SLOT(onFinished()));
    if (timeout) {
        connect(timeout, SIGNAL(timeout()), this, SLOT(abbrechen()));
    }
}

void Qt4ReplyHandler::abbrechen()
{
    if (m_reply) {
        m_reply->abort();
    }
}

void Qt4ReplyHandler::onFinished()
{
    // Nur einmal: ein abgebrochener Auftrag kann finished() zweimal melden.
    disconnect(m_reply, SIGNAL(finished()), this, SLOT(onFinished()));
    HafasClient::replyFinished(m_client, m_reply, m_callback, m_method, m_logResponses);
}
