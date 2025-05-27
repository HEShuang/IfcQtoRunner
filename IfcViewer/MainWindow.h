#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class IfcPreviewWidget;
class OpenGLWidgetDummy;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(qreal dpiScale, QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    qreal m_dpiScale;
    QString m_sCurrentFile;
    IfcPreviewWidget* m_pPreviewTree;
    OpenGLWidgetDummy* m_pGLWidget;

    void loadIfcFile();

};
#endif // MAINWINDOW_H
