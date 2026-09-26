// Laedt jede portierte QML-Datei und sagt, was die Maschine dazu meint.
//
// Das echte com.nokia.meego gibt es nur auf dem Geraet; hier stehen deshalb
// Attrappen dafuer auf dem Importpfad (meego/tests/stubs, erzeugt von
// meego/tests/make-stubs.sh aus den echten Komponenten). Das beweist nicht,
// dass die Seiten gut aussehen -- es beweist, dass sie sich lesen lassen,
// dass keine Eigenschaft gesetzt wird, die es nicht gibt, und dass die
// C++-Objekte, die Kontext-Eigenschaften und die Silica-Ersatzkomponenten zu
// dem passen, was die Seiten erwarten. Genau die Fehler also, die ein
// mechanischer Port zu Dutzenden macht.
//
//   meego/tests/check-qml.sh
#include <QApplication>
#include <QDeclarativeComponent>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDeclarativeError>
#include <QDir>
#include <QFileInfo>
#include <QObject>
#include <QStringList>
#include <QVariantMap>
#include <QtDebug>
#include <QtDeclarative>

#include <cstdio>

#include "polylineitem.h"

#include <src/appsettings.h>
#include <src/hafasclient.h>
#include <src/himmodel.h>
#include <src/journeymodel.h>
#include <src/locationmodel.h>
#include <src/maplauncher.h>
#include <src/recentmodel.h>

static QObject *instantiate(QDeclarativeEngine *engine, const QString &file)
{
    QDeclarativeComponent *component =
        new QDeclarativeComponent(engine, QUrl::fromLocalFile(file), engine);
    QObject *object = component->create(engine->rootContext());
    if (object) {
        object->setParent(engine);
        QDeclarativeEngine::setObjectOwnership(object, QDeclarativeEngine::CppOwnership);
    }
    return object;
}

int main(int argc, char *argv[])
{
    // Ohne Oberflaeche: der Pruefer zeigt nie ein Fenster, und der
    // Build-Rechner hat keinen Bildschirm.
    QApplication app(argc, argv, false);

    const QString qmlDir = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QString("meego/qml");
    const QString stubs = argc > 2 ? QString::fromLocal8Bit(argv[2]) : QString("meego/tests/stubs");

    qmlRegisterType<PolylineItem>("fahrplanat.polyline", 1, 0, "PolylineItem");

    QDeclarativeEngine engine;
    engine.addImportPath(stubs);
    engine.addImportPath(qmlDir);

    AppSettings settings;
    HafasClient client(&settings);
    LocationModel locationModel(&client);
    JourneyModel journeyModel(&client, &settings);
    HimModel himModel(&client);
    RecentModel recentModel;
    MapLauncher mapLauncher;

    QDeclarativeContext *context = engine.rootContext();
    context->setContextProperty("settings", &settings);
    context->setContextProperty("hafas", &client);
    context->setContextProperty("locationModel", &locationModel);
    context->setContextProperty("journeyModel", &journeyModel);
    context->setContextProperty("himModel", &himModel);
    context->setContextProperty("recentModel", &recentModel);
    context->setContextProperty("maps", &mapLauncher);
    context->setContextProperty("appVersion", QString("check"));

    QVariantMap pageOrientation;
    pageOrientation.insert("Automatic", 0);
    pageOrientation.insert("LockPortrait", 1);
    pageOrientation.insert("LockLandscape", 2);
    context->setContextProperty("PageOrientation", pageOrientation);

    QVariantMap pageStatus;
    pageStatus.insert("Inactive", 0);
    pageStatus.insert("Activating", 1);
    pageStatus.insert("Active", 2);
    pageStatus.insert("Deactivating", 3);
    context->setContextProperty("PageStatus", pageStatus);

    QVariantMap theme;
    theme.insert("inverted", true);
    context->setContextProperty("theme", theme);

    context->setContextProperty("AppTheme", instantiate(&engine, qmlDir + "/context/Theme.qml"));

    // Jede Datei, damit auch eine Seite geprueft wird, die von der Wurzel aus
    // niemand erreicht.
    QStringList files;
    files << qmlDir + "/harbour-fahrplanat.qml";
    const char *subdirs[] = {"silica", "pages", "components", 0};
    for (const char **sub = subdirs; *sub; sub++) {
        QDir dir(qmlDir + "/" + *sub);
        const QStringList namen = dir.entryList(QStringList() << "*.qml", QDir::Files, QDir::Name);
        for (const QString &name : namen) {
            files << dir.absoluteFilePath(name);
        }
    }

    int failed = 0;
    for (const QString &file : files) {
        QDeclarativeComponent component(&engine, QUrl::fromLocalFile(file));
        if (component.isError()) {
            failed++;
            printf("FEHLER %s\n", qPrintable(QFileInfo(file).fileName()));
            const QList<QDeclarativeError> fehler = component.errors();
            for (const QDeclarativeError &error : fehler) {
                printf("       %s\n", qPrintable(error.toString()));
            }
            continue;
        }
        printf("ok     %s\n", qPrintable(QFileInfo(file).fileName()));
    }

    printf("\n%d von %d Dateien liessen sich nicht laden\n", failed, files.size());
    return failed == 0 ? 0 : 1;
}
