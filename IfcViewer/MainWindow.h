#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class IfcPreviewWidget;
class OpenGLWidget;

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
    OpenGLWidget* m_p3DView;

    void loadPreviewTree();
    void load3DModel();

};
#endif // MAINWINDOW_H
