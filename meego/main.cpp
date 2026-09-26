// Fahrplan AT fuer MeeGo Harmattan (Nokia N9 / N950).
//
// Upstreams src/main.cpp baut eine Sailfish-App: SailfishApp fuer die Sicht,
// QQuickView fuer das QML. Qt 4.7 hat beides nicht, also startet hier eine
// QDeclarativeView und legt dieselben Objekte unter denselben Namen in den
// Wurzelkontext -- eine Kontext-Eigenschaft und ein importiertes Singleton
// werden an der Stelle, wo sie benutzt werden, gleich geschrieben.
//
// Alles unterhalb des QML ist der Code von Sailfish, unveraendert bis auf
// das, was meego/compat auffaengt.
#include <QApplication>
#include <QDeclarativeComponent>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDeclarativeError>
#include <QDeclarativeView>
#include <QDir>
#include <QFileInfo>
// Fuer rootObject(): ohne den vollstaendigen Typ ist QGraphicsObject* nicht
// nach QObject* umzudeuten.
#include <QGraphicsObject>
#include <QLocale>
#include <QTextCodec>
#include <QUrl>
#include <QVariantMap>
#include <QtDebug>
#include <QtDeclarative>

#include "polylineitem.h"

#include <src/appsettings.h>
#include <src/hafasclient.h>
#include <src/himmodel.h>
#include <src/journeymodel.h>
#include <src/locationmodel.h>
#include <src/maplauncher.h>
#include <src/networkfactory.h>
#include <src/recentmodel.h>

// Wo QML und Symbole nach dem Auspacken liegen; wird das Programm aus einem
// Quellbaum gestartet, wird der genommen.
static QString dataDir()
{
    const QString installed = QLatin1String("/opt/harbour-fahrplanat");
    if (QFileInfo(installed + "/qml/harbour-fahrplanat.qml").exists()) {
        return installed;
    }
    const QString here = QCoreApplication::applicationDirPath();
    for (QDir dir(here); !dir.isRoot(); dir.cdUp()) {
        if (QFileInfo(dir.absolutePath() + "/meego/qml/harbour-fahrplanat.qml").exists()) {
            return dir.absolutePath() + "/meego";
        }
    }
    return installed;
}

// Ein QML-Objekt aus einer Datei -- das Thema. Es haengt an der Maschine und
// nicht am Stapel: ein von einer QDeclarativeComponent erzeugtes Objekt lebt
// in deren Kontext, und stirbt die Komponente, ist danach jede Eigenschaft
// des Objekts "undefined" -- eine ganze App in Standardschrift, ohne eine
// Zeile im Log.
static QObject *instantiate(QDeclarativeEngine *engine, const QString &file)
{
    QDeclarativeComponent *component =
        new QDeclarativeComponent(engine, QUrl::fromLocalFile(file), engine);
    if (component->isError()) {
        // Bereichs-for statt foreach: Qt 4.7s Q_FOREACH laeuft unter GCC 14
        // mit -std=gnu++11 nur ein einziges Mal durch -- es sah aus, als
        // haette ein Verzeichnis nur eine Datei.
        const QList<QDeclarativeError> fehler = component->errors();
        for (const QDeclarativeError &error : fehler) {
            qWarning() << file << error.toString();
        }
        return 0;
    }
    QObject *object = component->create(engine->rootContext());
    if (object) {
        object->setParent(engine);
        QDeclarativeEngine::setObjectOwnership(object, QDeclarativeEngine::CppOwnership);
    }
    return object;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Qt 4 schickt den Quelltext von tr() durch Latin-1, wenn man es ihm
    // nicht abgewoehnt -- und dann ist jeder Umlaut in der App kaputt.
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    app.setOrganizationName(QLatin1String("harbour-fahrplanat"));
    app.setApplicationName(QLatin1String("harbour-fahrplanat"));
    app.setApplicationVersion(QLatin1String(APP_VERSION));

    // Der Streckenverlauf auf der Karte; QtQuick 1.1 hat kein Canvas.
    qmlRegisterType<PolylineItem>("fahrplanat.polyline", 1, 0, "PolylineItem");

    AppSettings settings;
    HafasClient client(&settings);
    LocationModel locationModel(&client);
    JourneyModel journeyModel(&client, &settings);
    HimModel himModel(&client);
    RecentModel recentModel;
    MapLauncher mapLauncher;
    NetworkFactory networkFactory;

    const QString data = dataDir();

    QDeclarativeView view;
    view.engine()->setNetworkAccessManagerFactory(&networkFactory);
    QDeclarativeContext *ctx = view.rootContext();
    ctx->setContextProperty(QLatin1String("settings"), &settings);
    ctx->setContextProperty(QLatin1String("hafas"), &client);
    ctx->setContextProperty(QLatin1String("locationModel"), &locationModel);
    ctx->setContextProperty(QLatin1String("journeyModel"), &journeyModel);
    ctx->setContextProperty(QLatin1String("himModel"), &himModel);
    ctx->setContextProperty(QLatin1String("recentModel"), &recentModel);
    ctx->setContextProperty(QLatin1String("maps"), &mapLauncher);
    ctx->setContextProperty(QLatin1String("appVersion"), QLatin1String(APP_VERSION));
    // Wird unten gefuellt, sobald das Wurzelobjekt steht; schon jetzt
    // angemeldet, damit die Bindungen, die es lesen, null bekommen statt auf
    // dem Weg nach oben zu scheitern.
    QObject *nochKeinFenster = 0;
    ctx->setContextProperty(QLatin1String("appWindow"), nochKeinFenster);

    // Das Thema, aus dem die portierten Seiten ihre Masse und Farben lesen.
    // Es heisst mit Absicht AppTheme und nicht Theme: com.nokia.meego bringt
    // ein eigenes Theme mit, das eine gleichnamige Kontext-Eigenschaft
    // verdeckt -- und dann steht die ganze App ohne Gestaltung da.
    ctx->setContextProperty(QLatin1String("AppTheme"),
                            instantiate(view.engine(), data + "/qml/context/Theme.qml"));

    view.engine()->addImportPath(data + "/qml");
    view.setResizeMode(QDeclarativeView::SizeRootObjectToView);
    view.setSource(QUrl::fromLocalFile(data + "/qml/harbour-fahrplanat.qml"));
    if (view.status() == QDeclarativeView::Error) {
        const QList<QDeclarativeError> fehler = view.errors();
        for (const QDeclarativeError &error : fehler) {
            qWarning() << "QML:" << error.toString();
        }
        return 1;
    }
    // rootObject() ist ein QGraphicsObject*; ohne die ausdrueckliche Umdeutung
    // auf QObject* greift der Compiler nach der QVariant-Ueberladung, und
    // deren Zeiger-Konstruktor ist privat.
    ctx->setContextProperty(QLatin1String("appWindow"),
                            static_cast<QObject *>(view.rootObject()));

    view.showFullScreen();
    return app.exec();
}
