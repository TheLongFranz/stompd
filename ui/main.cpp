// STL Stuff
#include<print>

// Third-Party Stuff
#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    std::println("Hello, World!");
    
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, [] { QCoreApplication::exit(1); }, Qt::QueuedConnection);

    // Qt.quit() in QML is wired to QCoreApplication::quit() by the engine.
    engine.loadFromModule("HelloQt", "Main");

    return app.exec();
}
