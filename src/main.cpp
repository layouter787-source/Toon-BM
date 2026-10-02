#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include "core/Project.h"
#include "script/ScriptRunner.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    app.setApplicationName("Toon-BM");
    QQuickStyle::setStyle("Material");

    Project project;
    ScriptRunner scripts(&project);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("project", &project);
    engine.rootContext()->setContextProperty("scripts", &scripts);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, [] { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    engine.loadFromModule("ToonBM", "Main");
    return app.exec();
}
