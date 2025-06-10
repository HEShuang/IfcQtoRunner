#include "IfcGeometryParser.h"
#include <thread>
#include <ifcgeom/Iterator.h>

void IfcGeometryParser::parse(IfcParse::IfcFile& ifcFile, IfcElemProcessorBase& elemProcessor) {
    std::string Prefix("[IfcGeometryParser] ");
    //Logger::SetOutput(&std::cout, &std::cerr);
    Logger::Notice(Prefix + "parseGeometry begins");

    if(!ifcFile.good())
    {
        Logger::Error(Prefix + "Failed to parse ifc file");
        return;
    }

    ifcopenshell::geometry::Settings settings;
    settings.set("use-world-coords", false);
    settings.set("weld-vertices", false);
    settings.set("apply-default-materials", true);

    int num_thread = std::thread::hardware_concurrency();
    Logger::Notice(Prefix + "num_thread:" + std::to_string(num_thread));

    IfcGeom::Iterator it = IfcGeom::Iterator("opencascade", settings, &ifcFile, num_thread);
    if(!it.initialize())
    {
        Logger::Error(Prefix + "Failed to initialize geometry iterator");
        return;
    }

    int nTotal = 0, nSuccess = 0;

    elemProcessor.onStart();
    do {
        nTotal++;
        if(elemProcessor.process(it.get()))
            nSuccess++;

    } while (it.next());

    if(!nSuccess)
        elemProcessor.onFinish(false, "No geometry loaded");
    else
        elemProcessor.onFinish(true, std::to_string(nSuccess) + "/" + std::to_string(nTotal) + " geometry loaded");
}
