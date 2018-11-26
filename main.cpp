#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "scene_gui.h"
#include "navagator_mouse_controller.h"
#include "utils.h"

char **cmd_args = nullptr;
int cmd_cnt = 0;
int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    cmd_args = argv + 1;
    cmd_cnt = argc;
    qmlRegisterType<SceneGUI>("MyScene", 1, 0, "SceneGUI");
    qmlRegisterType<NavagatorMouseController>("MyScene", 1, 0, "NavMouseController");
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
