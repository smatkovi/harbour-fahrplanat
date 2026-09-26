// Eine Haltestellensuche ohne Oberflaeche -- der Beweis, dass auf dem Geraet
// wirklich ankommt, was am Schreibtisch gebaut wurde.
//
// Geprueft wird damit die ganze Kette: TLS 1.2 aus Qt 4.7 heraus (Harmattans
// Werks-OpenSSL kann es nicht; Qt nimmt die neuere libssl aus
// /usr/local/lib, wenn eine da ist), die HAFAS-Anfrage, und der JSON-Ersatz
// samt Parser.
//
//   meego/build.sh probe   und dann auf dem Geraet:
//   /home/user/hafas_probe "Wien Hauptbahnhof"
#include <QCoreApplication>
#include <QStringList>
#include <QTimer>
#include <QtDebug>

#include <cstdio>

#include "appsettings.h"
#include "hafasclient.h"
#include "locationmodel.h"

class Probe : public QObject
{
    Q_OBJECT

public:
    Probe(const QString &text)
        : m_settings()
        , m_client(&m_settings)
        , m_model(&m_client)
        , m_text(text)
    {
        connect(&m_model, SIGNAL(busyChanged()), this, SLOT(vielleichtFertig()));
        connect(&m_model, SIGNAL(countChanged()), this, SLOT(vielleichtFertig()));
    }

public slots:
    // Ein Slot, kein gewoehnliches Verfahren: QTimer::singleShot ruft ihn
    // ueber den Namen.
    void los()
    {
        printf("Suche \"%s\" ueber %s\n", qPrintable(m_text),
               qPrintable(m_settings.endpoint()));
        fflush(stdout);
        m_model.search(m_text);
    }

private slots:
    void vielleichtFertig()
    {
        if (m_model.property("busy").toBool()) {
            return;
        }
        const QString fehler = m_model.property("error").toString();
        if (!fehler.isEmpty()) {
            printf("Fehler: %s\n", qPrintable(fehler));
            qApp->exit(1);
            return;
        }
        const int anzahl = m_model.property("count").toInt();
        printf("%d Treffer\n", anzahl);
        for (int i = 0; i < anzahl && i < 5; ++i) {
            const QVariantMap ort = m_model.get(i);
            printf("  %s (%s)\n", qPrintable(ort.value("name").toString()),
                   qPrintable(ort.value("type").toString()));
        }
        qApp->exit(anzahl > 0 ? 0 : 2);
    }

private:
    AppSettings m_settings;
    HafasClient m_client;
    LocationModel m_model;
    QString m_text;
};

#include "hafas_probe.moc"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setOrganizationName("harbour-fahrplanat");
    app.setApplicationName("harbour-fahrplanat");

    const QString text = argc > 1 ? QString::fromLocal8Bit(argv[1])
                                  : QString::fromUtf8("Wien Hauptbahnhof");
    Probe probe(text);
    QTimer::singleShot(0, &probe, SLOT(los()));
    // Nicht ewig warten: auf 2G darf es dauern, haengen darf es nicht.
    QTimer::singleShot(60000, &app, SLOT(quit()));
    return app.exec();
}
