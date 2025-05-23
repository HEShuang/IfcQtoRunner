#ifndef IFCPREVIEW_H
#define IFCPREVIEW_H

#include <string>
#include <ifcparse/IfcFile.h>

#include "DataNode.h"
#include "SceneData.h"

class IfcPreview
{
    std::string m_sFile;
    std::unique_ptr<DataNode::Base> m_upTreeRoot = std::make_unique<DataNode::Base>();


public:
    IfcPreview(const std::string& file): m_sFile(file) {}

    bool execute();
    const auto& getTreeRoot() {return m_upTreeRoot;}


    /**
     * Parses the geometry from the IFC file.
     * @param sceneObjects Output vector to be filled with parsed scene objects.
     * Each SceneObject contains its transformation and a list of meshes.
     * Mesh vertices are in local coordinates and should be transformed by
     * SceneObject::transform to get world coordinates.
     * @return True if parsing was successful, false otherwise.
     */
    bool parseGeometry(std::vector<SceneData::Object>& sceneObjects);

    //bool parseGeometryOSG(std::vector<osg::ref_ptr<osg::MatrixTransform>>& osgMatrixTransforms);
};

#endif // IFCPREVIEW_H
