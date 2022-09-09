#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>

namespace SoftRenderer
{
	struct Vertex final
	{
		glm::vec3 position;
		glm::vec2 texcoord;
		glm::vec3 normal;
		glm::vec4 tangent;
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
		enum  MeshFlags : uint32_t
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

		SubMesh(const std::vector<glm::vec3>& positions, const std::vector<glm::vec3>& normals, const std::vector<glm::vec2>& uvs, const std::vector<int>& indices);

		SubMesh(const SubMesh& mesh)
		{
			this->vertices = mesh.vertices;
			this->indices = mesh.indices;
		}

		SubMesh& operator=(const SubMesh& mesh)
		{
			if (&mesh == this) {
				return *this;
			}
			this->vertices = mesh.vertices;
			this->indices = mesh.indices;
			return *this;
		}

		void addTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2);

		SubMesh& setPositions(const std::vector<glm::vec3>& positions);
		SubMesh& setNormals(const std::vector<glm::vec3>& normals);
		SubMesh& setTangents(const std::vector<glm::vec4>& tangents);
		SubMesh& setColors(const std::vector <glm::vec4>& colors);
		SubMesh& setUvs(const std::vector<glm::vec2>& uvs);
		SubMesh& setIndices(const std::vector<int>& indices);
		SubMesh& build();
	
		std::vector<Vertex> vertices;
		std::vector<int> indices;

		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec4> tangents;
		std::vector<glm::vec2> uvs;
		std::vector<glm::vec4> colors;

		uint32_t flag = 0;
	};

	class Mesh
	{
	public:
		void addSubMesh(std::shared_ptr<SubMesh> subMesh);

        static std::shared_ptr<Mesh> createPlaneMesh();
        static std::shared_ptr<Mesh> createPlaneMesh2();

        static std::shared_ptr<Mesh> createBoxMesh();
        static std::shared_ptr<Mesh> createBoxMesh2();

	public:

		std::vector<std::shared_ptr<SubMesh>> subMeshs;
	};
}