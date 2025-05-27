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

    connect(ui->btLoad, &QPushButton::clicked, this, &MainWindow::loadIfcFile);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadIfcFile()
{
    m_sCurrentFile.clear();
    m_sCurrentFile = QFileDialog::getOpenFileName(this, tr("Open IFC File"), ".", tr("IFC Files (*.ifc)"));
    if(m_sCurrentFile.isEmpty())
        return;

    IfcPreview ifcPreview(m_sCurrentFile.toStdString());

    m_pPreviewTree->loadTree(ifcPreview.createPreviewTree());

    m_p3DView->setSceneObjects(ifcPreview.parseGeometry());
}

