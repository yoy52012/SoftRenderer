#include "Mesh.h"

#include "MathUtils.h"

namespace SoftRenderer
{
	SubMesh::SubMesh(const std::vector<glm::vec3>& positions, const std::vector<glm::vec3>& normals, const std::vector<glm::vec2>& uvs, const std::vector<uint32_t>& indices)
	{
		this->positions = positions;
		this->normals = normals;
		this->uvs = uvs;
		this->indices = indices;

		flag = HasPosition | HasNormal | HasTexcoord;
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

	SubMesh& SubMesh::setTangents(const std::vector<glm::vec4>& tangents)
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
		subMeshs.emplace_back(subMesh);
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
}


