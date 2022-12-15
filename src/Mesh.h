#pragma once

#include <vector>
#include <memory>

#include "MathUtils.h"
#include "BoxSphereBounds.h"

namespace SoftRenderer
{
    struct Vertex final
    {
        glm::vec3 position;
        glm::vec2 texcoord;
        glm::vec3 normal;
        glm::vec3 tangent;
        glm::vec4 color;

        Vertex() = default;

        Vertex(const Vertex& v)
        {
            position = v.position;
            texcoord = v.texcoord;
            normal = v.normal;
            tangent = v.tangent;
            color = v.color;
        }

        Vertex& operator=(const Vertex& v)
        {
            position = v.position;
            texcoord = v.texcoord;
            normal = v.normal;
            tangent = v.tangent;
            color = v.color;
            return *this;
        }
    };

    class SubMesh
    {
    public:
        enum MeshFlags
        {
            HasPosition = 1,
            HasNormal = 1 << 1,
            HasTangent = 1 << 2,
            HasTexcoord = 1 << 3,
            HasColor = 1 << 4,
        };

    public:
        SubMesh() = default;
        ~SubMesh() = default;

        SubMesh(const std::vector<glm::vec3>& positions, const std::vector<glm::vec3>& normals, const std::vector<glm::vec2>& uvs, const std::vector<glm::vec3>& tangents, const std::vector<glm::vec4>& colors, const std::vector<uint32_t>& indices);
        SubMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
        SubMesh(const SubMesh& mesh);

        SubMesh& operator=(const SubMesh& mesh);

        void addTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2);

        SubMesh& setPositions(const std::vector<glm::vec3>& positions);
        SubMesh& setNormals(const std::vector<glm::vec3>& normals);
        SubMesh& setTangents(const std::vector<glm::vec3>& tangents);
        SubMesh& setColors(const std::vector <glm::vec4>& colors);
        SubMesh& setUVs(const std::vector<glm::vec2>& uvs);
        SubMesh& setIndices(const std::vector<uint32_t>& indices);
        SubMesh& build();
    
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec3> tangents;
        std::vector<glm::vec2> uvs;
        std::vector<glm::vec4> colors;

        uint32_t flag = 0;

        uint32_t bufferId = 0;
    };

    class Mesh
    {
    public:
        void addSubMesh(std::shared_ptr<SubMesh> subMesh);

        static std::shared_ptr<Mesh> createPlane(uint32_t width, uint32_t height, uint32_t widthSegments, uint32_t heightSegments);

        static std::shared_ptr<Mesh> createBox(uint32_t width, uint32_t height, uint32_t depth, uint32_t widthSegments, uint32_t heightSegments, uint32_t depthSegments);

        static std::shared_ptr<Mesh> createSphere(float radius, float phiStart, float phiLength, float thetaStart, float thetaLength);

        static std::shared_ptr<Mesh> createTorusKnot(float radius, float tube, uint32_t tubularSegments, uint32_t radialSegments, uint32_t p, uint32_t q);

    public:
        BoxSphereBounds mBounds;

        std::vector<std::shared_ptr<SubMesh>> subMeshs;
    };
}