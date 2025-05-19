#include "MainWindow.h"
#include "./ui_MainWindow.h"

#include <QTreeWidgetItem>
#include <QFileDialog>
#include <QDebug>

#include "IfcPreview.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->btLoad, &QPushButton::clicked, this, &MainWindow::loadPreviewTree);
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

    auto filePath = fileDialog.selectedFiles().first();

    qDebug() << filePath;


    IfcPreview ifcPreview(filePath.toStdString());
    if(!ifcPreview.execute())
    {
        qDebug() << "preivew execution failed";
        return;
    }
    std::function< void (QTreeWidgetItem*, const std::unique_ptr<DataNode::Base>&) > createWidgetItem
        = [&createWidgetItem](QTreeWidgetItem* pParentItem, const std::unique_ptr<DataNode::Base>& upNode){

        auto pItem = new QTreeWidgetItem();
        pParentItem->addChild(pItem);

        if(auto pObjectNode = upNode->as<DataNode::IfcObject>())
        {
            pItem->setText(0, QString::fromStdString(pObjectNode->m_name));
            //pItem->setData(0, Qt::UserRole, QString::fromStdString(pObjectNode->m_guid));
            //pItem->setData(0, Qt::UserRole + 1, QString::fromStdString(pObjectNode->m_ifcClass));

        }
        else if(auto pClassNode = upNode->as<DataNode::IfcClass>())
        {
            pItem->setText(0, QString("%1 (%2)").arg(pClassNode->m_ifcClass).arg(pClassNode->m_objectsCount));
            //pItem->setData(0, Qt::UserRole, pClassNode->m_objectsGuids)
            //pItem->setData(0, Qt::UserRole + 1, QString::fromStdString(pClassNode->m_ifcClass));
        }

        for(const auto& upChild : upNode->getChildren())
            createWidgetItem(pItem, upChild);
    };

    //create top level widget items, then create children items recursively
    for(const auto& upChild: ifcPreview.getTreeRoot()->getChildren())
    {
        //the first level node is always an object eg IfcProject, IfcBuilding etc.
        if(const auto& pChild = upChild->as<DataNode::IfcObject>())
        {
            auto topItem = new QTreeWidgetItem(ui->treeWidget);
            topItem->setText(0, QString::fromStdString(pChild->m_name));
            topItem->setData(0, Qt::UserRole, QString::fromStdString(pChild->m_guid));
            topItem->setData(0, Qt::UserRole + 1, QString::fromStdString(pChild->m_ifcClass));

            for(const auto& upChildChild : pChild->getChildren())
                createWidgetItem(topItem, upChildChild);
        }
    }
    ui->treeWidget->expandAll();
}
