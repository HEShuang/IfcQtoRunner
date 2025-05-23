#include "MainWindow.h"
#include "./ui_MainWindow.h"

#include <QTreeWidgetItem>
#include <QFileDialog>
#include <QDebug>

#include "IfcPreview.h"
#include "IfcPreviewWidget.h"
#include "OpenGLWidget.h"

MainWindow::MainWindow(qreal dpiScale, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_dpiScale(dpiScale)
    , m_p3DView(nullptr)
{
    ui->setupUi(this);

    QVBoxLayout *layoutTree = new QVBoxLayout(ui->frameTree);
    m_pPreviewTree = new IfcPreviewWidget;
    layoutTree->addWidget(m_pPreviewTree);

    QVBoxLayout *layout3D = new QVBoxLayout(ui->frame3D);
    m_p3DView = new OpenGLWidget(m_dpiScale);
    layout3D->addWidget(m_p3DView);

    connect(ui->btLoad, &QPushButton::clicked, this, &MainWindow::loadPreviewTree);
    connect(ui->bt3D, &QPushButton::clicked, this, &MainWindow::load3DModel);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadPreviewTree()
{
    QString filters = "IFC files (*.ifc)";
    QFileDialog fileDialog(this, "Import Ifc", QString(), filters);
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    if(!fileDialog.exec())
        return;

    const auto& selectedFiles = fileDialog.selectedFiles();
    m_sCurrentFile = selectedFiles.first();

    IfcPreview ifcPreview(m_sCurrentFile.toStdString());
    if(!ifcPreview.execute())
    {
        qDebug() << "preivew execution failed";
        return;
    }
    m_pPreviewTree->loadTree(ifcPreview.getTreeRoot());

}

void MainWindow::load3DModel()
{
    m_p3DView->loadFile(m_sCurrentFile.toStdString());
}
