#include "IfcPreview.h"

#include <ifcparse/Ifc2x3.h>
#include <ifcparse/Ifc4.h>
#include <ifcparse/Ifc4x1.h>
#include <ifcparse/Ifc4x2.h>
#include <ifcparse/Ifc4x3.h>
#include <ifcparse/Ifc4x3_add2.h>
#include <ifcparse/Ifc4.h>
#include <ifcparse/Ifc4x3.h>
#include <iostream>

// For BOOST_PP_SEQ_FOR_EACH and BOOST_PP_STRINGIZE preprocessor macro
#include <boost/preprocessor/seq/for_each.hpp>

#include "IfcParseHelper.h"

#define IFC_SCHEMA_SEQ (Ifc4x3_add2)(Ifc4x3)(Ifc4x2)(Ifc4x1)(Ifc4)(Ifc2x3)
#define PROCESS_FOR_SCHEMA(r, data, elem)                  \
if (schema_version == BOOST_PP_STRINGIZE(elem))            \
{                                                          \
    auto parser = IfcParseHelper<elem>();                  \
    parser.createPreviewTree(ifcFile, m_upTreeRoot.get()); \
}                                                          \
else                                                       \

bool IfcPreview::execute()
{
    IfcParse::IfcFile ifcFile(m_sFile);

    if(!ifcFile.good()) {
        std::cerr << "Unable to parse .ifc file" << std::endl;
        return false;
    }

    auto schema_version = ifcFile.schema()->name().substr(3);
    std::transform(schema_version.begin(), schema_version.end(), schema_version.begin(), [](const char& c) {
        return std::tolower(c);
    });
    schema_version = "Ifc" + schema_version;

    BOOST_PP_SEQ_FOR_EACH(PROCESS_FOR_SCHEMA,  , IFC_SCHEMA_SEQ)
    {
        // The final else to catch unhandled schema version
        throw std::invalid_argument("IFC Schema " + schema_version + " not supported");
        return false;
    }
    return true;

}
