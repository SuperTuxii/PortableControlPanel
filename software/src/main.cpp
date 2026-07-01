#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setOrganizationName("PortableControlPanel");
    QGuiApplication::setOrganizationDomain("https://github.com/SuperTuxii/PortableControlPanel");
    QGuiApplication::setApplicationName("ControlPanelSoftware");

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.addImportPath(":/");
    engine.loadFromModule("ControlPanelSoftware", "Main");

    return app.exec();
}
