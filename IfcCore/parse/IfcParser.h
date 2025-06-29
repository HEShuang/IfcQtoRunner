#ifndef IFCPARSER_H
#define IFCPARSER_H

#include <string>
#include <ifcparse/IfcFile.h>

#include "DataNode.h"
#include "SceneData.h"

class IfcParser
{
    std::string m_sFile;
    IfcParse::IfcFile m_ifcFile;

public:
    IfcParser(const std::string& file);

    std::unique_ptr<DataNode::Base> createPreviewTree();

    /**
     * Parses the geometry from the IFC file.
     * Create a list of scene objects
     * Each scene object contains its transformation and a list of meshes.
     * Mesh vertices are in local coordinates and should be transformed to get world coordinates.
     * @return pointer to the list of created scene objects
     */
    std::shared_ptr<std::vector<SceneData::Object>> parseGeometry();

    // Define callback types
    using Callback_ObjectReady = std::function<void(std::shared_ptr<SceneData::Object> objectData)>;
    using Callback_ParseFinished = std::function<void(bool success, const std::string& message)>;

    /**
     * @brief parseGeometryFlow
     * Parse geometry from the IFC file supporting callbacks when one object is ready
     * @param onObjectReady: callback function when the geometry of one object is parsed and ready to render
     * @param onParseFinished: callback function when all geometry are parsed
     */
    void parseGeometryFlow(Callback_ObjectReady onObjectReady, Callback_ParseFinished onParseFinished);

};

#endif // IFCPARSER_H
