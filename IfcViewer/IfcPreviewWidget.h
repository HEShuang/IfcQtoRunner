#ifndef IFCPREVIEWWIDGET_H
#define IFCPREVIEWWIDGET_H

#include <QTreeWidget>

#include "DataNode.h"

class IfcPreviewWidget : public QTreeWidget
{
    Q_OBJECT

public:
    IfcPreviewWidget(QWidget *parent = nullptr);
    void clearAll();
    void loadTree(const std::unique_ptr<DataNode::Base>& upTreeRoot);
    void handleLoadGeometryFinished();

signals:
    void objectVisibilityChanged(const QString& guid, bool visible);
    void objectSelectionChanged(const QSet<QString>& guids);

private slots:
    void handleTreeItemChanged(QTreeWidgetItem * item, int column);
    void handleItemSelectionChanged();

private:
    std::vector<QTreeWidgetItem*> m_pItemsToHideByDefault;

};

#endif // IFCPREVIEWWIDGET_H
