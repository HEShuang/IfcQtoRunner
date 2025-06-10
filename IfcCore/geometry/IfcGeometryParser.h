#ifndef IFCGEOMETRYPARSER_H
#define IFCGEOMETRYPARSER_H

#include <ifcparse/IfcFile.h>
#include "IfcElemProcessorBase.h"

class IfcGeometryParser
{
public:
    void parse(IfcParse::IfcFile& ifcFile, IfcElemProcessorBase& elemProcessor);
};

#endif
