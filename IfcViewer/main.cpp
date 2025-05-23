#include "MainWindow.h"

#include <QApplication>
#include <QSurfaceFormat>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // --- Configure OpenGL Context ---
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);

    // Request a Core Profile OpenGL 3.2 context (or higher, e.g., 4,1 for macOS)
    // 3.2 is a good common baseline
    format.setVersion(3, 2);
    format.setProfile(QSurfaceFormat::CoreProfile);
    //format.setOption(QSurfaceFormat::DebugContext); // Request debug context for logging errors

    QSurfaceFormat::setDefaultFormat(format);
    // --- End OpenGL Context Configuration ---


    qreal dpiScale = app.devicePixelRatio();

    MainWindow w(dpiScale);
    w.show();
    return app.exec();
}
