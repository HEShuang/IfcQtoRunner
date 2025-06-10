#ifndef IFCSTRUCTUREBUILDER_H
#define IFCSTRUCTUREBUILDER_H

#include <ifcparse/IfcFile.h>
#include "IfcSchemaStrategyBase.h"
#include "DataNode.h"

class IfcStructureBuilder
{
public:
    IfcStructureBuilder();

    // Create tree grouped by storey
    std::unique_ptr<DataNode::Base> buildTreeByStorey(IfcParse::IfcFile& ifcFile, const IfcSchemaStrategyBase& strategy);

    // TODO: create tree grouped by element type
};

#endif // IFCSTRUCTUREBUILDER_H
