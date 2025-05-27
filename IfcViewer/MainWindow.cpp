#include "MainWindow.h"
#include "./ui_MainWindow.h"

#include <QTreeWidgetItem>
#include <QFileDialog>
#include <QElapsedTimer>

#include "IfcPreview.h"
#include "IfcPreviewWidget.h"
#include "OpenGLWidgetDummy.h"

MainWindow::MainWindow(qreal dpiScale, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_dpiScale(dpiScale)
    , m_pPreviewTree(new IfcPreviewWidget)
    , m_pGLWidget(new OpenGLWidgetDummy(m_dpiScale))
{
    ui->setupUi(this);

    m_pPreviewTree->setHeaderHidden(true);

    QVBoxLayout *layoutTree = new QVBoxLayout(ui->frameTree);
    layoutTree->setContentsMargins(0,0,0,0);
    layoutTree->addWidget(m_pPreviewTree);

    QVBoxLayout *layout3D = new QVBoxLayout(ui->frame3D);
    layout3D->setContentsMargins(0,0,0,0);
    layout3D->addWidget(m_pGLWidget);

    ui->splitter->setSizes(QList<int>{250, 750});

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

    ui->labelStatus->setText(m_sCurrentFile);

    IfcPreview ifcPreview(m_sCurrentFile.toStdString());
    m_pPreviewTree->loadTree(ifcPreview.createPreviewTree());

    //parse geometry once then load all
    QElapsedTimer timer;
    timer.start();
    m_pGLWidget->setSceneObjects(ifcPreview.parseGeometry());
    ui->statusbar->showMessage(QString("Geometry parsing: %1 ms").arg(timer.elapsed()));
}

