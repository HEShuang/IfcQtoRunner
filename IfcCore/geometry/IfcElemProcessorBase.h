#ifndef IFCELEMPROCESSOR_BASE_H
#define IFCELEMPROCESSOR_BASE_H

#include <ifcgeom/IfcGeomElement.h>

class IfcElemProcessorBase {
public:
    virtual ~IfcElemProcessorBase() = default;
    virtual bool process(const IfcGeom::Element* pElement) = 0;
    virtual void onStart() {}
    virtual void onFinish(bool success, const std::string& message) {}
};

#endif // IFCELEMPROCESSOR_BASE_H
