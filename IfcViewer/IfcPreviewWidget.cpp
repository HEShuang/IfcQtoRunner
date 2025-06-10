#include "IfcPreviewWidget.h"

IfcPreviewWidget::IfcPreviewWidget(QWidget *parent) : QTreeWidget(parent)
{
    connect(this, &QTreeWidget::itemChanged, this, &IfcPreviewWidget::handleTreeItemChanged);
    connect(this, &QTreeWidget::itemSelectionChanged, this, &IfcPreviewWidget::handleItemSelectionChanged);
}

void IfcPreviewWidget::clearAll()
{
    QTreeWidget::clear();
    m_pItemsToHideByDefault.clear();
}

void IfcPreviewWidget::loadTree(const std::unique_ptr<DataNode::Base>& upTreeRoot)
{
    clearAll();

    if(!upTreeRoot)
        return;

    auto fillObjectItem = [this](QTreeWidgetItem* pItem, DataNode::IfcObject* pObjectNode) {

        auto ifcClass = QString::fromStdString(pObjectNode->m_ifcClass);
        if (ifcClass.compare("IfcOpeningElement", Qt::CaseInsensitive) == 0 ||
            ifcClass.compare("IfcSpace", Qt::CaseInsensitive) == 0) {
            //todo complete default hidden types
            m_pItemsToHideByDefault.push_back(pItem);
        }

        auto name = QString::fromStdString(pObjectNode->m_name);
        if(name.isEmpty())
            name = tr("unnamed ") + ifcClass;

        pItem->setText(0, name);
        pItem->setData(0, Qt::UserRole, QString::fromStdString(pObjectNode->m_guid));
        pItem->setCheckState(0, Qt::CheckState::Checked);
    };

    std::function< void (QTreeWidgetItem*, const std::unique_ptr<DataNode::Base>&) > createWidgetItem
        = [&createWidgetItem, &fillObjectItem](QTreeWidgetItem* pParentItem, const std::unique_ptr<DataNode::Base>& upNode){

              auto pItem = new QTreeWidgetItem();
              pParentItem->addChild(pItem);

              if(auto pObjectNode = upNode->as<DataNode::IfcObject>())
                  fillObjectItem(pItem, pObjectNode);

              else if(auto pClassNode = upNode->as<DataNode::IfcClass>())
              {
                  pItem->setText(0, QString("%1 (%2)").arg(pClassNode->m_ifcClass).arg(pClassNode->m_objectsCount));
              }

              for(const auto& upChild : upNode->getChildren())
                  createWidgetItem(pItem, upChild);
          };

    //create top level widget items, then create children items recursively
    for(const auto& upChild: upTreeRoot->getChildren())
    {
        //the first level node is always an object eg IfcProject, IfcBuilding etc.
        if(const auto& pChild = upChild->as<DataNode::IfcObject>())
        {
            auto topItem = new QTreeWidgetItem(this);
            fillObjectItem(topItem, pChild);

            for(const auto& upChildChild : pChild->getChildren())
                createWidgetItem(topItem, upChildChild);
        }
    }

    this->expandAll();
}

void IfcPreviewWidget::handleTreeItemChanged(QTreeWidgetItem * item, int column)
{
    emit objectVisibilityChanged(item->data(0, Qt::UserRole).toString() /*guid*/ , item->checkState(column));

    //todo update children and parent state
    //item->treeWidget()->blockSignals(true);
    //set children and parents state
    //item->treeWidget()->blockSignals(false);
}

void IfcPreviewWidget::handleItemSelectionChanged()
{
    const auto& selection = selectedItems();
    if(selection.isEmpty())
        emit objectSelectionChanged(QSet<QString>());
    else {
        QSet<QString> selectedGuids;
        for (auto pItem : selection) {
            selectedGuids << pItem->data(0, Qt::UserRole).toString();
        }
        emit objectSelectionChanged(std::move(selectedGuids));
    }
}

void IfcPreviewWidget::handleLoadGeometryFinished()
{
    for(const auto& pItem : m_pItemsToHideByDefault) {
        pItem->setCheckState(0, Qt::CheckState::Unchecked);
    }
}
