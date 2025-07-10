#include "FileReader.h"
#include "mbrsave.h"
#include "mbrwrite.h"
#include "HexEditor.h"
#include "disassemblywindows.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QObject>
#include <qqmlengine.h>
#include <QQmlContext>
#include <QQuickStyle>

class WindowManager : public QObject {
    Q_OBJECT
public:
    explicit WindowManager(QObject *parent = nullptr) : QObject(parent) {}

public slots:
    void openDisassemblyWin() {
        // 创建新的 QQmlApplicationEngine 实例
        QQmlApplicationEngine *engine = new QQmlApplicationEngine;

        // 使用 load 加载文件路径
        engine->load(QUrl::fromLocalFile("MBRtool/disassembly.qml"));  // 根据实际路径修改
    }
};

int main(int argc, char *argv[])
{
    QQuickStyle::setStyle("Fusion");

    QGuiApplication app(argc, argv);

    // 注册 FileReader 类，暴露给 QML
    qmlRegisterType<FileReader>("FileReader", 1, 0, "FileReader");

    QQmlApplicationEngine engine;

    //对象
    MBRsave mbrsave;
    MBRwrite mbrwrite;
    WindowManager windowManager;
    DisassemblyWindows disassemblywindows;
    HexEditor hexEditor;
    hexEditor.loadFile();

    //注册对象
    engine.rootContext()->setContextProperty("mbrsave", &mbrsave);
    engine.rootContext()->setContextProperty("mbrwrite", &mbrwrite);
    engine.rootContext()->setContextProperty("window", &windowManager);
    engine.rootContext()->setContextProperty("disassembler", &disassemblywindows);
    engine.rootContext()->setContextProperty("hexEditorBackend", &hexEditor);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("MBRtool", "Main");

    return app.exec();
}

#include "main.moc"
