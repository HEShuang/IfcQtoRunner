#ifndef IFCPREVIEWWIDGET_H
#define IFCPREVIEWWIDGET_H

#include <QTreeWidget>

#include "DataNode.h"

class IfcPreviewWidget : public QTreeWidget
{
public:
    IfcPreviewWidget(QWidget *parent = nullptr);
    void loadTree(const std::unique_ptr<DataNode::Base>& upTreeRoot);
};

#endif // IFCPREVIEWWIDGET_H
