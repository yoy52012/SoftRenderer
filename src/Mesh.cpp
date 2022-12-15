#include "Mesh.h"

#include "MathUtils.h"

namespace SoftRenderer
{
    SubMesh::SubMesh(const std::vector<glm::vec3>& positions, const std::vector<glm::vec3>& normals, const std::vector<glm::vec2>& uvs, const std::vector<glm::vec3>& tangents, const std::vector<glm::vec4>& colors, const std::vector<uint32_t>& indices)
    {
        this->positions = positions;
        this->normals = normals;
        this->uvs = uvs;
        this->tangents = tangents;
        this->colors = colors;
        this->indices = indices;

        flag = HasPosition | HasNormal | HasTexcoord | HasTangent | HasColor;

        build();
    }

    SubMesh::SubMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
    {
        this->vertices = vertices;
        this->indices = indices;
    }

    SubMesh::SubMesh(const SubMesh& mesh)
    {
        this->vertices = mesh.vertices;
        this->indices = mesh.indices;
    }

    SubMesh& SubMesh::operator=(const SubMesh& mesh)
    {
        if (&mesh == this) 
        {
            return *this;
        }
        this->vertices = mesh.vertices;
        this->indices = mesh.indices;
        return *this;
    }

    void SubMesh::addTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2)
    {
        uint32_t size = indices.size();
        vertices.push_back(v0);
        vertices.push_back(v1);
        vertices.push_back(v2);

        indices.push_back(size);
        indices.push_back(size + 1);
        indices.push_back(size + 2);
    }

    SubMesh& SubMesh::setPositions(const std::vector<glm::vec3>& positions)
    {
        this->positions = positions;
        flag |= MeshFlags::HasPosition;
        return *this;
    }

    SubMesh& SubMesh::setNormals(const std::vector<glm::vec3>& normals)
    {
        this->normals = normals;
        flag |= MeshFlags::HasNormal;
        return *this;
    }

    SubMesh& SubMesh::setTangents(const std::vector<glm::vec3>& tangents)
    {
        this->tangents = tangents;
        flag |= MeshFlags::HasTangent;
        return *this;
    }

    SubMesh& SubMesh::setColors(const std::vector<glm::vec4>& colors)
    {
        this->colors = colors;
        flag |= MeshFlags::HasColor;
        return *this;
    }

    SubMesh& SubMesh::setUVs(const std::vector<glm::vec2>& uvs)
    {
        this->uvs = uvs;
        flag |= MeshFlags::HasTexcoord;
        return *this;
    }

    SubMesh& SubMesh::setIndices(const std::vector<uint32_t>& indices)
    {
        this->indices = indices;
        return *this;
    }

    SubMesh& SubMesh::build()
    {
        if (indices.empty() || positions.empty())
            return SubMesh();

        for (int i = 0; i < positions.size(); i++)
        {
            Vertex vertex;
            vertex.position = positions[i];
            vertices.emplace_back(vertex);
        }

        if ((flag & SubMesh::MeshFlags::HasTexcoord) && (positions.size() == uvs.size()))
        {
            for (uint32_t i = 0; i < uvs.size(); i++)
            {
                auto& vertex = vertices[i];
                vertex.texcoord = uvs[i];
            }
        }

        if ((flag & SubMesh::MeshFlags::HasNormal) && (positions.size() == normals.size()))
        {
            for (uint32_t i = 0; i < normals.size(); i++)
            {
                auto& vertex = vertices[i];
                vertex.normal = normals[i];
            }
        }

        if ((flag & SubMesh::MeshFlags::HasTangent) && (positions.size() == tangents.size()))
        {
            for (uint32_t i = 0; i < tangents.size(); i++)
            {
                auto& vertex = vertices[i];
                vertex.tangent = tangents[i];
            }
        }

        if ((flag & SubMesh::MeshFlags::HasColor) && (positions.size() == colors.size()))
        {
            for (uint32_t i = 0; i < colors.size(); i++)
            {
                auto& vertex = vertices[i];
                vertex.color = colors[i];
            }
        }

        return *this;
    }

    void Mesh::addSubMesh(std::shared_ptr<SubMesh> subMesh)
    {
        subMeshs.push_back(subMesh);
    }

    // Ref: https://github.com/mrdoob/three.js/blob/master/src/geometries/PlaneGeometry.js
    std::shared_ptr<Mesh> Mesh::createPlane(uint32_t width, uint32_t height, uint32_t widthSegments, uint32_t heightSegments)
    {
        const float widthHalf  = static_cast<float>(width) / 2.0f;
        const float heightHalf = static_cast<float>(height) / 2.0f;

        const uint32_t gridX = widthSegments;
        const uint32_t gridY = heightSegments;

        const uint32_t gridX1 = gridX + 1;
        const uint32_t gridY1 = gridY + 1;

        const float segmentWidth  = static_cast<float>(width) / static_cast<float>(gridX);
        const float segmentHeight = static_cast<float>(height) / static_cast<float>(gridY);

        std::vector<uint32_t> indices;
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uvs;

        for (uint32_t iy = 0; iy < gridY1; iy++) 
        {
            const float y = static_cast<float>(iy) * segmentHeight - heightHalf;

            for (uint32_t ix = 0; ix < gridX1; ix++)
            {
                const float x = static_cast<float>(ix) * segmentWidth - widthHalf;

                positions.push_back(glm::vec3(x, -y, 0));
                normals.push_back(glm::vec3(0, 0, 1));
                uvs.push_back(glm::vec2(ix / gridX, 1 - (iy / gridY)));
            }
        }

        for (uint32_t iy = 0; iy < gridY; iy++) 
        {
            for (uint32_t ix = 0; ix < gridX; ix++) 
            {
                uint32_t a = ix + gridX1 * iy;
                uint32_t b = ix + gridX1 * (iy + 1);
                uint32_t c = (ix + 1) + gridX1 * (iy + 1);
                uint32_t d = (ix + 1) + gridX1 * iy;

                indices.push_back(d);
                indices.push_back(b);
                indices.push_back(a);

                indices.push_back(d);
                indices.push_back(c);
                indices.push_back(b);
            }
        }

        std::shared_ptr<SubMesh> submesh = std::make_shared<SubMesh>();
        submesh->setPositions(positions);
        submesh->setUVs(uvs);
        submesh->setNormals(normals);
        submesh->setIndices(indices);
        submesh->build();

        std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
        mesh->addSubMesh(submesh);

        return mesh;
    }

    // Ref: https://github.com/mrdoob/three.js/blob/master/src/geometries/BoxGeometry.js
    std::shared_ptr<Mesh> Mesh::createBox(uint32_t width, uint32_t height, uint32_t depth, uint32_t widthSegments, uint32_t heightSegments, uint32_t depthSegments)
    {
        std::vector<uint32_t> indices;
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uvs;

        int32_t numberOfVertices = 0;

        auto buildPlane = [&](uint32_t u, uint32_t v, uint32_t w, float udir, float vdir, int32_t width, int32_t height, int32_t depth, uint32_t gridX, uint32_t gridY)
        {
            const float segmentWidth  = static_cast<float>(width)  / static_cast<float>(gridX);
            const float segmentHeight = static_cast<float>(height) / static_cast<float>(gridY);

            const float widthHalf  = static_cast<float>(width)  / 2.0f;
            const float heightHalf = static_cast<float>(height) / 2.0f;
            const float depthHalf  = static_cast<float>(depth)  / 2.0f;

            const uint32_t gridX1 = gridX + 1;
            const uint32_t gridY1 = gridY + 1;

            uint32_t vertexCounter = 0;

            glm::vec3 poistion;
            glm::vec3 normal;
            glm::vec2 uv;

            for (uint32_t iy = 0; iy < gridY1; iy++) 
            {
                const float y = static_cast<float>(iy) * segmentHeight - heightHalf;

                for (uint32_t  ix = 0; ix < gridX1; ix++) 
                {
                    const float x = static_cast<float>(ix) * segmentWidth - widthHalf;

                    // set values to correct vector component
                    poistion[u] = x * udir;
                    poistion[v] = y * vdir;
                    poistion[w] = depthHalf;

                    // now apply poistion to vertex buffer
                    positions.push_back(poistion);

                    // set values to correct vector component
                    normal[u] = 0;
                    normal[v] = 0;
                    normal[w] = depth > 0 ? 1 : -1;

                    // now apply normal to normal buffer
                    normals.push_back(normal);

                    // uvs
                    uv.x = ix / gridX;
                    uv.y = 1 - (iy / gridY);

                    //now apply uv to uv buffer
                    uvs.push_back(uv);

                    // counters
                    vertexCounter += 1;
                }
            }

            for (uint32_t iy = 0; iy < gridY; iy++) 
            {
                for (uint32_t ix = 0; ix < gridX; ix++) 
                {
                    uint32_t a = numberOfVertices + ix + gridX1 * iy;
                    uint32_t b = numberOfVertices + ix + gridX1 * (iy + 1);
                    uint32_t c = numberOfVertices + (ix + 1) + gridX1 * (iy + 1);
                    uint32_t d = numberOfVertices + (ix + 1) + gridX1 * iy;

                    // faces
                    indices.push_back(a);
                    indices.push_back(b);
                    indices.push_back(d);

                    indices.push_back(b);
                    indices.push_back(c);
                    indices.push_back(d);
                }

            }

            // update total number of vertices
            numberOfVertices += vertexCounter;
        };

        buildPlane(2, 1, 0, -1, -1, static_cast<int32_t>(depth), static_cast<int32_t>(height),  static_cast<int32_t>(width),  depthSegments, heightSegments); // right
        buildPlane(2, 1, 0, 1, -1,  static_cast<int32_t>(depth), static_cast<int32_t>(height), -static_cast<int32_t>(width),  depthSegments, heightSegments); // left
        buildPlane(0, 2, 1, 1, 1,   static_cast<int32_t>(width), static_cast<int32_t>(depth),   static_cast<int32_t>(height), widthSegments, depthSegments); // top
        buildPlane(0, 2, 1, 1, -1,  static_cast<int32_t>(width), static_cast<int32_t>(depth),  -static_cast<int32_t>(height), widthSegments, depthSegments); // bottom
        buildPlane(0, 1, 2, 1, -1,  static_cast<int32_t>(width), static_cast<int32_t>(height),  static_cast<int32_t>(depth),  widthSegments, heightSegments); // front
        buildPlane(0, 1, 2, -1, -1, static_cast<int32_t>(width), static_cast<int32_t>(height), -static_cast<int32_t>(depth),  widthSegments, heightSegments); // back

        std::shared_ptr<SubMesh> submesh = std::make_shared<SubMesh>();
        submesh->setPositions(positions);
        submesh->setNormals(normals);
        submesh->setUVs(uvs);
        submesh->setIndices(indices);
        submesh->build();

        // Vertex v1;
        // v1.position = glm::vec3(1.0f, 0.454172879f, 1.0f);
        // v1.normal = glm::vec3(1.0f, 0.0f, 0.0f);
        // v1.texcoord = glm::vec2(0.0f, 0.727086425);

        // Vertex v2;
        // v1.position = glm::vec3(1.0f, 1.0f, -0.545838356f);
        // v1.normal = glm::vec3(1.0f, 0.0f, 0.0f);
        // v1.texcoord = glm::vec2(0.772919178f, 1.0f);

        // Vertex v3;
        // v1.position = glm::vec3(0.850769639f, 1.0f, 1.0f);
        // v1.normal = glm::vec3(0.0f, 1.0f, 0.0f);
        // v1.texcoord = glm::vec2(0.925384820f, 0.0);

        // Vertex v4;
        // v1.position = glm::vec3(1.0f, 1.0f, -0.545838356f);
        // v1.normal = glm::vec3(0.0f, 1.0f, 0.0f);
        // v1.texcoord = glm::vec2(1.0f, 0.772919179);

        // Vertex v5;
        // v1.position = glm::vec3(0.882809758f, 0.882809758f, 1.0f);
        // v1.normal = glm::vec3(0.0f, 0.0f, 1.0f);
        // v1.texcoord = glm::vec2(0.941404879f, 0.941404879);

        // Vertex v6;
        // v1.position = glm::vec3(0.850769639f, 1.0f, 1.0f);
        // v1.normal = glm::vec3(0.0f, 0.0f, 1.0f);
        // v1.texcoord = glm::vec2(0.925384820f, 1.0);

        // Vertex v7;
        // v1.position = glm::vec3(1.0f, 0.454172850f, 1.0f);
        // v1.normal = glm::vec3(0.0f, 0.0f, 1.0f);
        // v1.texcoord = glm::vec2(1.0f, 0.727086425);

        // Vertex v8;
        // v1.position = glm::vec3(0.882809758f, 0.882809758f, 1.0f);
        // v1.normal = glm::vec3(0.0f, 0.0f, 1.0f);
        // v1.texcoord = glm::vec2(0.941404879f, 0.941404879);
        
        // submesh->vertices.push_back(v1);
        // submesh->vertices.push_back(v2);
        // submesh->vertices.push_back(v3);
        // submesh->vertices.push_back(v4);
        // submesh->vertices.push_back(v5);
        // submesh->vertices.push_back(v6);
        // submesh->vertices.push_back(v7);
        // submesh->vertices.push_back(v8);

        // submesh->indices.push_back(24);
        // submesh->indices.push_back(1);
        // submesh->indices.push_back(25);

        // submesh->indices.push_back(10);
        // submesh->indices.push_back(27);
        // submesh->indices.push_back(9);

        // submesh->indices.push_back(16);
        // submesh->indices.push_back(28);
        // submesh->indices.push_back(29);

        // submesh->indices.push_back(18);
        // submesh->indices.push_back(30);
        // submesh->indices.push_back(31);

        std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
        mesh->addSubMesh(submesh);

        return mesh;
    }

    // Ref: https://github.com/mrdoob/three.js/blob/master/src/geometries/SphereGeometry.js
    std::shared_ptr<Mesh> Mesh::createSphere(float radius, float phiStart, float phiLength, float thetaStart, float thetaLength)
    {
        uint32_t widthSegments = 18;
        uint32_t heightSegments = 10;

        widthSegments  = std::max<uint32_t>(3, widthSegments);
        heightSegments = std::max<uint32_t>(2, heightSegments);

        const float thetaEnd = std::min<float>(thetaStart + thetaLength, Math::PI);

        uint32_t index = 0;
        std::vector<std::vector<uint32_t>> grid;
        
        std::vector<uint32_t> indices;
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uvs;

        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;

        for (uint32_t iy = 0; iy <= heightSegments; iy++) 
        {
            std::vector<uint32_t> verticesRow;

            const float v = static_cast<float>(iy) / static_cast<float>(heightSegments);

            // special case for the poles

            float uOffset = 0.0f;

            if (iy == 0 && thetaStart == 0) 
            {
                uOffset = 0.5f / static_cast<float>(widthSegments);
            }
            else if (iy == heightSegments && thetaEnd == Math::PI)
            {
                uOffset = -0.5f / static_cast<float>(widthSegments);
            }

            for (uint32_t ix = 0; ix <= widthSegments; ix++) 
            {
                const float u = static_cast<float>(ix) / static_cast<float>(widthSegments);

                // vertex
                position.x = -radius * std::cos(phiStart + u * phiLength) * std::sin(thetaStart + v * thetaLength);
                position.y = radius  * std::cos(thetaStart + v * thetaLength);
                position.z = radius  * std::sin(phiStart + u * phiLength) * std::sin(thetaStart + v * thetaLength);
                positions.push_back(position);

                // normal
                normal = glm::normalize(position);
                normals.push_back(normal);

                // uv
                uv.x = u + uOffset;
                uv.y = 1 - v;
                uvs.push_back(uv);

                verticesRow.push_back(index++);
            }

            grid.push_back(verticesRow);

        }

        // indices
        for (uint32_t iy = 0; iy < heightSegments; iy++) 
        {
            for (uint32_t ix = 0; ix < widthSegments; ix++) 
            {
                const uint32_t a = grid[iy][ix + 1];
                const uint32_t b = grid[iy][ix];
                const uint32_t c = grid[iy + 1][ix];
                const uint32_t d = grid[iy + 1][ix + 1];

                if (iy != 0 || thetaStart > 0)
                {
                    indices.push_back(a);
                    indices.push_back(b);
                    indices.push_back(d);
                }


                if (iy != heightSegments - 1 || thetaEnd < Math::PI)
                {
                    indices.push_back(b);
                    indices.push_back(c);
                    indices.push_back(d);
                }
            }
        }

        std::shared_ptr<SubMesh> submesh = std::make_shared<SubMesh>();
        submesh->setPositions(positions);
        submesh->setNormals(normals);
        submesh->setUVs(uvs);
        submesh->setIndices(indices);
        submesh->build();

        std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
        mesh->addSubMesh(submesh);

        return mesh;
    }

    std::shared_ptr<Mesh> Mesh::createTorusKnot(float radius, float tube, uint32_t tubularSegments, uint32_t radialSegments, uint32_t p, uint32_t q )
    {
        // buffers
        std::vector<uint32_t> indices;
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uvs;

        // helper variables
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;

        glm::vec3 p1;
        glm::vec3 p2;

        glm::vec3 B;
        glm::vec3 T;
        glm::vec3 N;

        // this function calculates the current position on the torus curve
        auto calculatePositionOnCurve = []( float u, float p, float q, float radius, glm::vec3& position ) 
        {
            const float cu = std::cos( u );
            const float su = std::sin( u );
            const float quOverP = q / p * u;
            const float cs = std::cos( quOverP );

            position.x = radius * ( 2.0f + cs ) * 0.5f * cu;
            position.y = radius * ( 2.0f + cs ) * su * 0.5f;
            position.z = radius * std::sin( quOverP ) * 0.5f;
        };


        // generate vertices, normals and uvs

        for (uint32_t i = 0; i <= tubularSegments; ++ i ) 
        {
            // the radian "u" is used to calculate the position on the torus curve of the current tubular segment
            const float tu = float(i) / float(tubularSegments);
            const float u =  tu * float(p) * Math::PI * 2.0f;

            // now we calculate two points. P1 is our current position on the curve, P2 is a little farther ahead.
            // these points are used to create a special "coordinate space", which is necessary to calculate the correct vertex positions
            calculatePositionOnCurve( u, p, q, radius, p1 );
            calculatePositionOnCurve( u + 0.01f, p, q, radius, p2 );

            // calculate orthonormal basis
            T = p2 - p1;
            N = p2 + p1;
            B = glm::cross( T, N );
            N = glm::cross( B, T );

            // normalize B, N. T can be ignored, we don't use it

            B = glm::normalize(B);
            N = glm::normalize(N);

            for ( uint32_t j = 0; j <= radialSegments; ++ j ) 
            {
                // now calculate the vertices. they are nothing more than an extrusion of the torus curve.
                // because we extrude a shape in the xy-plane, there is no need to calculate a z-value.
                const float tv = float(j) / float(radialSegments);
                const float v  = tv * Math::PI * 2.0f;
                const float cx = -tube * std::cos( v );
                const float cy = tube * std::sin( v );

                // now calculate the final vertex position.
                // first we orient the extrusion with our basis vectors, then we add it to the current position on the curve
                position.x = p1.x + ( cx * N.x + cy * B.x );
                position.y = p1.y + ( cx * N.y + cy * B.y );
                position.z = p1.z + ( cx * N.z + cy * B.z );

                positions.push_back(position);

                // normal (P1 is always the center/origin of the extrusion, thus we can use it to calculate the normal)
                normal = glm::normalize(position - p1);
                normals.push_back(normal);

                // uv
                uv.x = tu;
                uv.y = tv;
                uvs.push_back(uv);
            }

        }

        // generate indices
        for ( uint32_t j = 1; j <= tubularSegments; j++ ) 
        {
            for ( uint32_t i = 1; i <= radialSegments; i++ ) 
            {
                // indices
                const uint32_t a = ( radialSegments + 1 ) * ( j - 1 ) + ( i - 1 );
                const uint32_t b = ( radialSegments + 1 ) * j + ( i - 1 );
                const uint32_t c = ( radialSegments + 1 ) * j + i;
                const uint32_t d = ( radialSegments + 1 ) * ( j - 1 ) + i;

                // faces
                indices.push_back(a);
                indices.push_back(b);
                indices.push_back(d);
                
                indices.push_back(b);
                indices.push_back(c);
                indices.push_back(d);
            }

        }

        // build geometry
        std::shared_ptr<SubMesh> submesh = std::make_shared<SubMesh>();
        submesh->setPositions(positions);
        submesh->setNormals(normals);
        submesh->setUVs(uvs);
        submesh->setIndices(indices);
        submesh->build();

        std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
        mesh->addSubMesh(submesh);

        return mesh;
    }
}


