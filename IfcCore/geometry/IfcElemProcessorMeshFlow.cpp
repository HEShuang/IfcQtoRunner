#include "IfcElemProcessorMeshFlow.h"

IfcElemProcessorMeshFlow::IfcElemProcessorMeshFlow(Callback_ObjectReady onObjectReady, Callback_ParseFinished onParseFinished)
    : m_func_onObjectReady(onObjectReady)
    , m_func_onParseFinished(onParseFinished)
{
}

void IfcElemProcessorMeshFlow::onStart() {
    m_lastGeometryId.clear();
    if(m_spLastCreatedMeshes)
        m_spLastCreatedMeshes->clear();
}

void IfcElemProcessorMeshFlow::onFinish(bool success, const std::string& message) {
    m_func_onParseFinished(success, message);
}

bool IfcElemProcessorMeshFlow::process(const IfcGeom::Element* pElement) {

    if(!pElement)
        return false;

    std::string Prefix("[ProcessorMesh] ");
    const auto* triElem = dynamic_cast<const IfcGeom::TriangulationElement*>(pElement);
    if(!triElem)
    {
        Logger::Error(Prefix + "null or not triangulation element");
        return false;
    }

    auto spCurrentObject = std::make_shared<SceneData::Object>();
    //basic infos
    spCurrentObject->name = triElem->name();
    spCurrentObject->type = triElem->type();
    spCurrentObject->geometryId = triElem->geometry().id();
    spCurrentObject->guid = triElem->guid();

    Logger::Notice(Prefix + "Iteratoring geom "
                   + spCurrentObject->type
                   + ":" + spCurrentObject->name
                   + " geometryId :" + spCurrentObject->geometryId
                   + " guid: " + spCurrentObject->guid );

    //Transformation
    SceneData::Matrix4x4 matrix;
    const auto& transform4x4 = triElem->transformation().data()->components();
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            matrix.m[row * 4 + col] = transform4x4(row,col);
    spCurrentObject->transform = std::move(matrix);

    std::string m("----matrix----");
    for(int i=0; i<16; ++i)
        m+= std::to_string(matrix.m[i]) + ",";
    m+="\n";
    Logger::Notice(Prefix + m);


    // If it's the same geometry, reuse it
    const auto & curGeometryId = triElem->geometry().id();
    if(curGeometryId == m_lastGeometryId && m_spLastCreatedMeshes)
    {
        Logger::Notice(Prefix + "same geometry ID, reuse last created meshes");
        spCurrentObject->meshes = m_spLastCreatedMeshes;

        m_func_onObjectReady(spCurrentObject);
        return true;
    }

    //Not the same geometry, create new geometry
    const auto& spGeomTri = triElem ->geometry_pointer();
    const std::vector<double>& coordsVertices = spGeomTri->verts(); // x1, y1, z1, x2, y2, z2, ...
    const std::vector<double>& coordsNormals = spGeomTri->normals();// nx1, ny1, nz1, nx2, ny2, nz2, ...
    const std::vector<int>& indicesFaces = spGeomTri->faces(); // Indices into coordsVertices defining face triangles
    const std::vector<ifcopenshell::geometry::taxonomy::style::ptr>& materials = spGeomTri->materials();
    const std::vector<int>& materialIds = spGeomTri->material_ids();


    auto n = indicesFaces.size();
    if (n == 0)
    {
        Logger::Error(Prefix + "Failed: no faces found!");
        return false;
    }
    if (n%3 != 0)
    {
        Logger::Error(Prefix + "Failed: vertices of faces are incomplet !");
        return false;
    }
    if (n/3 > materialIds.size())
    {
        Logger::Error(Prefix + "Failed to map all the faces to material IDs!");
        return false;
    }

    // Group faces by material ID
    std::map<int, std::vector<int>> groupedFaces; //materialId_{1,2,3,5,7,6, etc ...} face0(1,2,3), face1(5,7,6), etc
    auto itFace = indicesFaces.begin();
    for (const int& matId : materialIds)
    {
        if(itFace != indicesFaces.end())
            for (int i = 0; i < 3; ++i) {
                groupedFaces[matId].push_back(*itFace++);
            }
    }

    //Create mesh for each group of faces
    std::shared_ptr<std::vector<SceneData::Mesh>> spCurrentMeshes = std::make_shared<std::vector<SceneData::Mesh>>();
    for (const auto& group : groupedFaces)
    {
        int matId = group.first;
        const std::vector<int>& vertIndices = group.second;
        auto nVerts = vertIndices.size();

        std::vector<SceneData::Vec3f> vertices, normals;
        vertices.reserve(nVerts);
        normals.reserve(nVerts);
        for (int index : vertIndices)
        {
            int coordIndex = 3 * index; // This is the start index of the coordinates
            vertices.push_back(SceneData::Vec3f(coordsVertices[coordIndex],
                                                coordsVertices[coordIndex + 1],
                                                coordsVertices[coordIndex + 2]));
            normals.push_back(SceneData::Vec3f(coordsNormals[coordIndex],
                                               coordsNormals[coordIndex + 1],
                                               coordsNormals[coordIndex + 2]));
        }

        SceneData::Mesh mesh;
        mesh.vertices = std::move(vertices);
        mesh.normals = std::move(normals);

        if (const auto& pMaterial = materials[matId])
        {
            float alpha = 1.0f - pMaterial->transparency;
            if(std::isnan(alpha))
                alpha = 1.0;

            mesh.color = {
                (float)pMaterial->diffuse.r(),
                (float)pMaterial->diffuse.g(),
                (float)pMaterial->diffuse.b(),
                alpha
            };
        }
        else
        {
            Logger::Warning("Warning: Null material style pointer for material ID :" + std::to_string(matId));
        }

        spCurrentMeshes->push_back(std::move(mesh));
    }

    m_lastGeometryId = curGeometryId;
    m_spLastCreatedMeshes = spCurrentMeshes;
    spCurrentObject->meshes = spCurrentMeshes;

    m_func_onObjectReady(spCurrentObject);
    return true;
}
