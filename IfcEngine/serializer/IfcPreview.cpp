#include "IfcPreview.h"

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

#include "IfcParseHelper.h"

#define IFC_SCHEMA_SEQ (Ifc4x3_add2)(Ifc4x3)(Ifc4x2)(Ifc4x1)(Ifc4)(Ifc2x3)
#define PROCESS_FOR_SCHEMA(r, data, elem)                  \
if (schema_version == BOOST_PP_STRINGIZE(elem))            \
{                                                          \
    auto parser = IfcParseHelper<elem>();                  \
    return parser.createPreviewTree(m_ifcFile); \
}                                                          \
else                                                       \

IfcPreview::IfcPreview(const std::string& file): m_sFile(file), m_ifcFile(file)
{
    if(!m_ifcFile.good())
        std::cerr << "Unable to parse .ifc file" << std::endl;
}


std::unique_ptr<DataNode::Base> IfcPreview::createPreviewTree()
{
    auto schema_version = m_ifcFile.schema()->name().substr(3);
    std::transform(schema_version.begin(), schema_version.end(), schema_version.begin(), [](const char& c) {
        return std::tolower(c);
    });
    schema_version = "Ifc" + schema_version;

    BOOST_PP_SEQ_FOR_EACH(PROCESS_FOR_SCHEMA,  , IFC_SCHEMA_SEQ)
    {
        // The final else to catch unhandled schema version
        throw std::invalid_argument("IFC Schema " + schema_version + " not supported");
        return nullptr;
    }
}

std::unique_ptr<std::vector<SceneData::Object>> IfcPreview::parseGeometry()
{
    std::string Prefix("[IfcQtoRunner] ");

    //Logger::SetOutput(&std::cout, &std::cerr);
    Logger::Notice(Prefix + "parseGeometry begins");

    IfcParse::IfcFile ifcFile(m_sFile);
    if(!ifcFile.good())
    {
        Logger::Error(Prefix + "Failed to parse ifc file");
        return nullptr;
    }

    auto upSceneObjects = std::make_unique<std::vector<SceneData::Object>>();

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
        return nullptr;
    }

    std::string lastGeometryId;
    std::shared_ptr<std::vector<SceneData::Mesh>> spLastCreatedMeshes = nullptr;

    do{
        const auto* triElem = dynamic_cast<const IfcGeom::TriangulationElement*>(it.get());
        if(!triElem)
        {
            Logger::Error(Prefix + "null or not triangulation element");
            continue;
        }

        SceneData::Object currentObject;
        //basic infos
        currentObject.name = triElem->name();
        currentObject.type = triElem->type();
        currentObject.geometryId = triElem->geometry().id();
        currentObject.guid = triElem->guid();

        Logger::Notice(Prefix + "Iteratoring geom "
                       + currentObject.type
                       + ":" + currentObject.name
                       + " geometryId :" + currentObject.geometryId
                       + " guid: " + currentObject.guid );

        //Transformation
        SceneData::Matrix4x4 matrix;
        const auto& transform4x4 = triElem->transformation().data()->components();
        for (int row = 0; row < 4; row++)
            for (int col = 0; col < 4; col++)
                matrix.m[row * 4 + col] = transform4x4(row,col);
        currentObject.transform = std::move(matrix);

        std::string m("----matrix----");
        for(int i=0; i<16; ++i)
            m+= std::to_string(matrix.m[i]) + ",";
        m+="\n";
        Logger::Notice(Prefix + m);


        // If it's the same geometry, reuse it
        const auto & curGeometryId = triElem->geometry().id();
        if(curGeometryId == lastGeometryId)
        {
            Logger::Notice(Prefix + "same geometry ID, reuse last created meshes");
            currentObject.meshes = spLastCreatedMeshes;
            upSceneObjects->push_back(std::move(currentObject));
            continue;
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
            continue;
        }
        if (n%3 != 0)
        {
            Logger::Error(Prefix + "Failed: vertices of faces are incomplet !");
            continue;
        }
        if (n/3 > materialIds.size())
        {
            Logger::Error(Prefix + "Failed to map all the faces to material IDs!");
            continue;
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

        lastGeometryId = curGeometryId;
        spLastCreatedMeshes = spCurrentMeshes;

        currentObject.meshes = spCurrentMeshes;
        upSceneObjects->push_back(std::move(currentObject));

    }while(it.next());

    return upSceneObjects;
}
