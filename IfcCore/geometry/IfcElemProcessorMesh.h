#ifndef IFCELEMPROCESSORMESH_H
#define IFCELEMPROCESSORMESH_H

#include "IfcElemProcessorBase.h"
#include "SceneData.h"

class IfcElemProcessorMesh : public IfcElemProcessorBase
{
public:
    bool process(const IfcGeom::Element* pElement) override;
    void onStart() override;
    void onFinish(bool success, const std::string& message) override;

    inline std::shared_ptr<std::vector<SceneData::Object>> getSceneObjects() {return m_spSceneObjects;}

private:
    std::string m_lastGeometryId;
    std::shared_ptr<std::vector<SceneData::Mesh>> m_spLastCreatedMeshes = nullptr;
    std::shared_ptr<std::vector<SceneData::Object>> m_spSceneObjects = nullptr;
};

#endif // IFCELEMPROCESSORMESH_H
