#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "CachedImageProvider.h"

int main(int argc, char *argv[])
{
    const QGuiApplication app(argc, argv);
    QGuiApplication::setOrganizationName("PortableControlPanel");
    QGuiApplication::setOrganizationDomain("https://github.com/SuperTuxii/PortableControlPanel");
    QGuiApplication::setApplicationName("ControlPanelSoftware");

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        [] { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.addImportPath(":/");
    engine.addImageProvider("cached", new CachedImageProvider());
    engine.loadFromModule("ControlPanelSoftware", "Main");

    return QGuiApplication::exec();
}
