#ifndef IFCPREVIEW_H
#define IFCPREVIEW_H

#include <string>
#include <ifcparse/IfcFile.h>

#include "DataNode.h"


class IfcPreview
{
    std::string m_sFile;
    std::unique_ptr<DataNode::Base> m_upTreeRoot = std::make_unique<DataNode::Base>();

public:
    IfcPreview(const std::string& file): m_sFile(file) {}

    bool execute();
    const auto& getTreeRoot() {return m_upTreeRoot;}
};

#endif // IFCPREVIEW_H
