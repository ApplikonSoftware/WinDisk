#include <QGuiApplication>
#include <QQuickStyle>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "guimanager.h"
#include "winnativeeventfilter.h"

int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QLocale::setDefault(QLocale::English);
    QGuiApplication::setApplicationName("WinDisk");
    QGuiApplication::setOrganizationName("Applikon Biotechnology");

    QGuiApplication app(argc, argv);

    QQuickStyle::setStyle("Universal");

    GuiManager* guiManager = new GuiManager(&app);
    app.installNativeEventFilter(new WinNativeEventFilter(guiManager));

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("guiManager", guiManager);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
