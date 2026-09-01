#include <QGuiApplication>
#include <QQmlContext>
#include <QQuickView>
#include <QScopedPointer>
#include <QtQml>

#include <sailfishapp.h>

#include "appsettings.h"
#include "hafasclient.h"
#include "himmodel.h"
#include "journeymodel.h"
#include "locationmodel.h"
#include "maplauncher.h"
#include "networkfactory.h"
#include "recentmodel.h"

int main(int argc, char *argv[])
{
    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));
    app->setOrganizationName(QStringLiteral("harbour-fahrplanat"));
    app->setApplicationName(QStringLiteral("harbour-fahrplanat"));
    app->setApplicationVersion(QStringLiteral(APP_VERSION));

    AppSettings settings;
    HafasClient client(&settings);
    LocationModel locationModel(&client);
    JourneyModel journeyModel(&client, &settings);
    HimModel himModel(&client);
    RecentModel recentModel;
    MapLauncher mapLauncher;
    NetworkFactory networkFactory;

    QScopedPointer<QQuickView> view(SailfishApp::createView());
    view->engine()->setNetworkAccessManagerFactory(&networkFactory);
    QQmlContext *ctx = view->rootContext();
    ctx->setContextProperty(QStringLiteral("settings"), &settings);
    ctx->setContextProperty(QStringLiteral("hafas"), &client);
    ctx->setContextProperty(QStringLiteral("locationModel"), &locationModel);
    ctx->setContextProperty(QStringLiteral("journeyModel"), &journeyModel);
    ctx->setContextProperty(QStringLiteral("himModel"), &himModel);
    ctx->setContextProperty(QStringLiteral("recentModel"), &recentModel);
    ctx->setContextProperty(QStringLiteral("maps"), &mapLauncher);
    ctx->setContextProperty(QStringLiteral("appVersion"), QStringLiteral(APP_VERSION));

    view->setSource(SailfishApp::pathTo(QStringLiteral("qml/harbour-fahrplanat.qml")));
    view->show();
    return app->exec();
}
