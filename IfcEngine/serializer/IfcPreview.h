#ifndef IFCPREVIEW_H
#define IFCPREVIEW_H

#include <string>
#include <ifcparse/IfcFile.h>

#include "DataNode.h"
#include "SceneData.h"

class IfcPreview
{
    std::string m_sFile;
    IfcParse::IfcFile m_ifcFile;

public:
    IfcPreview(const std::string& file);

    std::unique_ptr<DataNode::Base> createPreviewTree();

    /**
     * Parses the geometry from the IFC file.
     * Create a list of scene objects
     * Each scene object contains its transformation and a list of meshes.
     * Mesh vertices are in local coordinates and should be transformed to get world coordinates.
     * @return pointer to the list of created scene objects
     */
    std::unique_ptr<std::vector<SceneData::Object>> parseGeometry();

};

#endif // IFCPREVIEW_H
