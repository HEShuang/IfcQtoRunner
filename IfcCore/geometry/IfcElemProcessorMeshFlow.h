#ifndef IFCPROCESSOR_MESHFLOW_H
#define IFCPROCESSOR_MESHFLOW_H

#include "IfcElemProcessorBase.h"
#include "SceneData.h"

class IfcElemProcessorMeshFlow : public IfcElemProcessorBase
{
public:
    // Define callback types
    using Callback_ObjectReady = std::function<void(std::shared_ptr<SceneData::Object> objectData)>;
    using Callback_ParseFinished = std::function<void(bool success, const std::string& message)>;

    IfcElemProcessorMeshFlow(Callback_ObjectReady onObjectReady, Callback_ParseFinished onParseFinished);

    bool process(const IfcGeom::Element* pElement) override;
    void onStart() override;
    void onFinish(bool success, const std::string& message) override;

private:

    Callback_ObjectReady m_func_onObjectReady;
    Callback_ParseFinished m_func_onParseFinished;

    std::string m_lastGeometryId;
    std::shared_ptr<std::vector<SceneData::Mesh>> m_spLastCreatedMeshes = nullptr;
};

#endif // IFCPROCESSOR_MESHFLOW_H
