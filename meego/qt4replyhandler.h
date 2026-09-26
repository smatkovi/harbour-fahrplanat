// Was in Qt 5 eine Lambda am finished-Signal ist.
//
// Qt 4.7 kennt nur die Zeichenkettenform von connect und kann an gar keine
// Lambda binden. Dieses kleine Objekt haelt stattdessen, was die Lambda
// aufgefangen haette, haengt als Kind an der Antwort -- stirbt also mit ihr --
// und ruft im Slot dieselbe Funktion, die auch die Sailfish-Fassung ruft.
#ifndef QT4REPLYHANDLER_H
#define QT4REPLYHANDLER_H

#include <QObject>
#include <QPointer>
#include <QString>

#include "hafasclient.h"

class QNetworkReply;
class QTimer;

class Qt4ReplyHandler : public QObject
{
    Q_OBJECT

public:
    Qt4ReplyHandler(HafasClient *client, QNetworkReply *reply,
                    const HafasClient::Callback &callback, const QString &method,
                    bool logResponses, QTimer *timeout = 0);

private slots:
    void onFinished();
    // QNetworkReply::abort() ist in Qt 4.7 kein Slot -- der Umweg hier.
    void abbrechen();

private:
    QPointer<HafasClient> m_client;
    QNetworkReply *m_reply;
    HafasClient::Callback m_callback;
    QString m_method;
    bool m_logResponses;
};

#endif // QT4REPLYHANDLER_H
