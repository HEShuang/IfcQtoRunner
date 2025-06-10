#ifndef IFCPROCESSOR_OCC_H
#define IFCPROCESSOR_OCC_H

#include "IfcElemProcessorBase.h"

class IfcElemProcessorOCC : public IfcElemProcessorBase
{
public:
    bool process(const IfcGeom::Element* pElement) override;
};

#endif // IFCPROCESSOR_OCC_H
