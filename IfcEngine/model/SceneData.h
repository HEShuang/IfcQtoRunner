#ifndef SCENEDATA_H
#define SCENEDATA_H

#include <vector>
#include <string>

class SceneData {
public:
    struct Vec3f {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
    };

    struct ColorRGBA {
        float r = 0.8f;
        float g = 0.8f;
        float b = 0.8f;
        float a = 1.0f;
    };

    // Represents a mesh with a single material
    // Vertices are ordered to form triangles (e.g., v0,v1,v2, v3,v4,v5, ...)
    struct Mesh {
        std::vector<Vec3f> vertices;  // Local coordinates
        std::vector<Vec3f> normals;   // Per-vertex normals, same count as vertices
        ColorRGBA color;
    };

    // Represents a 4x4 transformation matrix (column-major)
    // m[12], m[13], m[14] are the translation components (dx, dy, dz)
    struct Matrix4x4 {
        float m[16];



        // m[0] m[4] m[8]  m[12]
        // m[1] m[5] m[9]  m[13]
        // m[2] m[6] m[10] m[14]
        // m[3] m[7] m[11] m[15]
        Matrix4x4() { // Constructor initializes to identity matrix
            std::fill(m, m + 16, 0.0f);
            m[0] = m[5] = m[10] = m[15] = 1.0f;
        }
    };

    // Represents a distinct object in the IFC scene
    // It has a transformation and consists of one or more meshes.
    struct Object {
        std::string name;                   // IFC element name (e.g., "Wall01")
        std::string type;                   // IFC element type (e.g., "IfcWall")
        std::string geometryId;             // Internal ID of the geometry representation, used for instancing
        std::string guid;
        Matrix4x4 transform;                // Local-to-world transformation for this object's meshes
        std::shared_ptr<std::vector<Mesh>> meshes;           // List of meshes that make up this object
        // std::string ifcProductGlobalId;  // Optional: IfcGloballyUniqueId of the product
    };

};

#endif // SCENEDATA_H
