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
    parser.createPreviewTree(ifcFile, m_upTreeRoot.get()); \
}                                                          \
else                                                       \

bool IfcPreview::execute()
{
    IfcParse::IfcFile ifcFile(m_sFile);

    if(!ifcFile.good())
    {
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

bool IfcPreview::parseGeometry(std::vector<SceneData::Object>& sceneObjects)
{
    std::string Prefix("[IfcQtoRunner] ");

    //Logger::SetOutput(&std::cout, &std::cerr);
    Logger::Notice(Prefix + "parseGeometry begins");

    IfcParse::IfcFile ifcFile(m_sFile);
    if(!ifcFile.good())
    {
        Logger::Error(Prefix + "Failed to parse ifc file");
        return false;
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
        return false;
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
            sceneObjects.push_back(std::move(currentObject));
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
        sceneObjects.push_back(std::move(currentObject));

    }while(it.next());

    return true;
}
/*
bool IfcPreview::parseGeometryOSG(std::vector<osg::ref_ptr<osg::MatrixTransform>>& osgMatrixTransforms)
{
    std::cout << "get into parseGeometry " << std::endl;

    IfcParse::IfcFile ifcFile(m_sFile);

    if(!ifcFile.good()) {
        std::cerr << "Unable to parse .ifc file" << std::endl;
        return false;
    }

    ifcopenshell::geometry::Settings settings;
    settings.set("use-world-coords", true);
    settings.set("weld-vertices", false);
    settings.set("apply-default-materials", true);

    IfcGeom::Iterator it = IfcGeom::Iterator(settings, &ifcFile);
    if(!it.initialize())
    {
        std::cerr << "Error: Iterator failed to initialize! Aborting.";
        return false;
    }

    std::string lastId;
    std::vector<osg::ref_ptr<osg::Geometry>> lastOSGGeometries;

    std::cout << "get into loop " << std::endl;
    do {

        const auto* triElem = static_cast<const IfcGeom::TriangulationElement*>(it.get());

        if(triElem)
        {
            std::cout << "triElem not null" << std::endl;
            std::cout << triElem->type()<< std::endl;
            std::cout <<triElem->name()<< std::endl;

            std::string elemInfo = triElem->type();
            elemInfo += triElem->name() == "" ? "" : ": " + triElem->name();
            std::cout<<elemInfo << std::endl;
        }
        else
        {
            std::cout << "Error: IfcGeom::Iterator returned null element inside loop. Breaking." << std::endl;
            break;
        }

        std::cout << "before" <<std::endl;
        // Print element info
        std::string elemInfo = triElem->type();
        elemInfo += triElem->name() == "" ? "" : ": " + triElem->name();
        std::cout<<elemInfo<< std::endl;

        std::cout << "Transformation Matrix" <<std::endl;
        // Transformation Matrix
        std::vector<double> matrixData;
        const auto& transform4x4 = triElem->transformation().data()->components();
        for (int col = 0; col<4; col++)
        {
            for (int row =0; row<3; row++)
            {
                matrixData.push_back(transform4x4(row,col));
            }

        }

        const osg::Matrixd matrixd (
            matrixData[0], matrixData[1], matrixData[2], 0,
            matrixData[3], matrixData[4], matrixData[5], 0,
            matrixData[6], matrixData[7], matrixData[8], 0,
            matrixData[9], matrixData[10], matrixData[11], 1
            );

        std::cout << "after osg::Matrixd" <<std::endl;
        const osg::ref_ptr<osg::MatrixTransform> osgMatrixTrsf= new osg::MatrixTransform;
        osgMatrixTrsf->setMatrix(matrixd);

        // It's the same geometry
        const auto & id = triElem->geometry().id();
        if(id == lastId)
        {
            std::cout << "same ID" <<std::endl;
            for (const auto& geometry : lastOSGGeometries)
            {
                osgMatrixTrsf->addChild(geometry);
            }
            osgMatrixTransforms.push_back(osgMatrixTrsf);
            continue;
        }

        std::cout << "not same ID" <<std::endl;
        // Not the same geometry :

        // Get the actual geometry triangulation pointer
        const auto& spGeomTri = triElem->geometry_pointer();

        const std::vector<double>& coordsVertices = spGeomTri->verts(); // x1, y1, z1, x2, y2, z2, ...
        const std::vector<double>& coordsNormals = spGeomTri->normals();// nx1, ny1, nz1, nx2, ny2, nz2, ...
        const std::vector<int>& indicesFaces = spGeomTri->faces(); // Indices into coordsVertices defining face triangles
        const std::vector<ifcopenshell::geometry::taxonomy::style::ptr>& materials = spGeomTri->materials();
        const std::vector<int>& materialIds = spGeomTri->material_ids();

        std::cout << "coordsVertices:" << coordsVertices.size() << std::endl;
        std::cout << "coordsNormals:" << coordsNormals.size() << std::endl;
        std::cout << "materials:" << materials.size() << std::endl;
        std::cout << "materialIds:" << materialIds.size() << std::endl;

        auto n = indicesFaces.size();       
        std::cout << "face indices number:" << n << std::endl;

        if (n == 0)
            std::cout << "** Failed: no faces found!" << std::endl;
        if (n%3 != 0)
            std::cout << "** Failed: vertices of faces are incomplet !" << std::endl;
        if (n/3 > materialIds.size())
            std::cout << "** Failed to map all the faces to material IDs!" << std::endl;

        // Group faces by material ID
        //std::map<int, osg::ref_ptr<osg::Material>> uniqueMaterials; // Declared but unused in the provided snippet
        std::map<int, std::vector<int>> groupedFaces; //materialId_{1,2,3,5,7,6, etc ...} face0(1,2,3), face1(5,7,6), etc
        auto itFace = indicesFaces.begin();
        for (const int& matId : materialIds)
        {
            if(itFace != indicesFaces.end())
                for (int i = 0; i < 3; ++i) {
                    groupedFaces[matId].push_back(*itFace++);
                }
        }

        std::cout << "groupedFaces:" << groupedFaces.size() << std::endl;


    auto addVertexAndNormal = [&coordsVertices, &coordsNormals]
        (osg::ref_ptr<osg::Vec3Array> osgVertices, osg::ref_ptr<osg::Vec3Array> osgNormals, const std::vector<int>& vertIds,size_t i){

            // Check if vertIds[i] is a valid index into vertIds itself
            if (i >= vertIds.size()) {
                std::cout << "ERROR: addVertexAndNormal received out-of-bounds 'i' for vertIds!" << std::endl;
                return; // Prevent crash
            }

            size_t vertexLookupIndex = vertIds[i]; // This is the logical vertex ID
            size_t initVertIndex = 3 * vertexLookupIndex; // This is the physical index in coordsVertices

            // Add comprehensive boundary checks
            if (initVertIndex + 2 >= coordsVertices.size()) {
                std::cout << "CRITICAL ERROR: Calculated index (3 * vertIds[i]) is out of bounds for coordsVertices!" << std::endl;
                std::cout << "  i: " << i << ", vertIds[i]: " << vertexLookupIndex << std::endl;
                std::cout << "  Calculated initVertIndex: " << initVertIndex << ", coordsVertices.size(): " << coordsVertices.size() << std::endl;
                // You might want to log more context or throw an exception here
                return; // Prevent crash
            }

            osgVertices->push_back(osg::Vec3(coordsVertices[initVertIndex],
                                             coordsVertices[initVertIndex + 1],
                                             coordsVertices[initVertIndex + 2]));

            if (coordsNormals.size() > 0) {
                if (initVertIndex + 2 >= coordsNormals.size()) {
                    std::cout << "CRITICAL ERROR: Calculated index (3 * vertIds[i]) is out of bounds for coordsNormals!" << std::endl;
                    std::cout << "  i: " << i << ", vertIds[i]: " << vertexLookupIndex << std::endl;
                    std::cout << "  Calculated initVertIndex: " << initVertIndex << ", coordsNormals.size(): " << coordsNormals.size() << std::endl;
                    return; // Prevent crash
                }
                osgNormals->push_back(osg::Vec3(coordsNormals[initVertIndex],
                                                coordsNormals[initVertIndex + 1],
                                                coordsNormals[initVertIndex + 2]));
            }
        };

        std::vector<osg::ref_ptr<osg::Geometry>> osgGeometries;

        for (const auto& group : groupedFaces) {
            std::cout << "in loop grouped faces" << std::endl;

            int matId = group.first;
            const std::vector<int>& vertIds = group.second;

             std::cout << "matID:" << matId << std::endl;

            osg::ref_ptr<osg::Vec3Array> osgVertices = new osg::Vec3Array;
            osg::ref_ptr<osg::Vec3Array> osgNormals = new osg::Vec3Array;

            for (size_t i = 0; i < vertIds.size(); i += 3) {
                //buildTriangleNodes(vertIds, coordsVertices, coordsNormals, i,osgVertices, osgNormals);
                addVertexAndNormal(osgVertices, osgNormals, vertIds, i);
                addVertexAndNormal(osgVertices, osgNormals, vertIds, i + 1);
                addVertexAndNormal(osgVertices, osgNormals, vertIds, i + 2);
            }

            std::cout << "osgVertices:" << osgVertices->size() << std::endl;
            std::cout << "osgNormals:" << osgNormals->size() << std::endl;

            osg::ref_ptr<osg::Geometry> osgGeometry = new osg::Geometry;

            osgGeometry->setVertexArray(osgVertices);
            osgGeometry->setNormalArray(osgNormals);
            osgGeometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

            osg::ref_ptr<osg::DrawArrays> drawArrays = new osg::DrawArrays(
                osg::PrimitiveSet::TRIANGLES, 0, osgVertices->size());
            osgGeometry->addPrimitiveSet(drawArrays);

            //osg material
            const auto& material = materials[matId];
            ifcopenshell::geometry::taxonomy::colour diffuseColor = material->diffuse;
            osg::Vec4 matOsgDiffuseColor(
                diffuseColor.r(),
                diffuseColor.g(),
                diffuseColor.b(),
                material->transparency);

            osg::ref_ptr<osg::Material> osgMaterial = new osg::Material;
            osgMaterial->setDiffuse(osg::Material::FRONT_AND_BACK, matOsgDiffuseColor);


            osgGeometry->getOrCreateStateSet()->setAttributeAndModes(osgMaterial.get());

            osgMatrixTrsf->addChild(osgGeometry);

            osgGeometries.push_back(osgGeometry);
        }

        osgMatrixTransforms.push_back(osgMatrixTrsf);

        lastId = id;
        lastOSGGeometries = osgGeometries;

        std::cout << "end of the loop" << std::endl;

    } while (it.next());

    std::cout << "finished loop" << std::endl;
    return true;
}
*/
