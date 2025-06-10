#include "IfcParser.h"

#include <iostream>
#include <string>

#include <ifcparse/Ifc2x3.h>
#include <ifcparse/Ifc4.h>
#include <ifcparse/Ifc4x1.h>
#include <ifcparse/Ifc4x2.h>
#include <ifcparse/Ifc4x3.h>
#include <ifcparse/Ifc4x3_add2.h>
#include <ifcparse/Ifc4.h>
#include <ifcparse/Ifc4x3.h>
#include <ifcparse/IfcLogger.h>

#include <ifcgeom/IfcGeomElement.h>
#include <ifcgeom/Iterator.h>
#include <ifcgeom/ConversionSettings.h>

// For BOOST_PP_SEQ_FOR_EACH and BOOST_PP_STRINGIZE preprocessor macro
#include <boost/preprocessor/seq/for_each.hpp>

#include "IfcSchemaStrategyImpl.h"
#include "IfcStructureBuilder.h"
#include "IfcGeometryParser.h"
#include "IfcElemProcessorMesh.h"
#include "IfcElemProcessorMeshFlow.h"

#define IFC_SCHEMA_SEQ (Ifc4x3_add2)(Ifc4x3)(Ifc4x2)(Ifc4x1)(Ifc4)(Ifc2x3)
#define PROCESS_FOR_SCHEMA(r, data, elem)                               \
if (schema_version == BOOST_PP_STRINGIZE(elem))                         \
{                                                                       \
    adapter = std::make_unique<IfcSchemaStrategyImpl<elem>>(m_ifcFile); \
}                                                                       \
else                                                                    \

IfcParser::IfcParser(const std::string& file): m_sFile(file), m_ifcFile(file)
{
    if(!m_ifcFile.good())
        std::cerr << "Unable to parse .ifc file" << std::endl;
}


std::unique_ptr<DataNode::Base> IfcParser::createPreviewTree()
{
    auto schema_version = m_ifcFile.schema()->name().substr(3);
    std::transform(schema_version.begin(), schema_version.end(), schema_version.begin(), [](const char& c) {
        return std::tolower(c);
    });
    schema_version = "Ifc" + schema_version;

    std::unique_ptr<IfcSchemaStrategyBase> adapter = nullptr;

    BOOST_PP_SEQ_FOR_EACH(PROCESS_FOR_SCHEMA,  , IFC_SCHEMA_SEQ) {
        // The final else to catch unhandled schema version
        throw std::invalid_argument("IFC Schema " + schema_version + " not supported");
        return nullptr;
    }

    IfcStructureBuilder builder;
    return builder.buildTreeByStorey(m_ifcFile, *adapter);
}

std::shared_ptr<std::vector<SceneData::Object>> IfcParser::parseGeometry() {
    IfcElemProcessorMesh elemProcessor;
    IfcGeometryParser geomParser;
    geomParser.parse(m_ifcFile, elemProcessor);
    return elemProcessor.getSceneObjects();
}

void IfcParser::parseGeometryFlow(Callback_ObjectReady onObjectReady, Callback_ParseFinished onParseFinished) {
    IfcElemProcessorMeshFlow elemProcessor(onObjectReady, onParseFinished);
    IfcGeometryParser geomParser;
    geomParser.parse(m_ifcFile, elemProcessor);
}
