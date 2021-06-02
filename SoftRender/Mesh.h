#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace SoftRenderer
{
	struct Vertex final
	{
		glm::vec3 position;
		glm::vec2 texcoord;
		glm::vec3 normal;
		glm::vec4 tangent;
		glm::vec4 color = glm::vec4(0.6f,0.6f,0.6f,1.0f);

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

	class Mesh
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
		Mesh() = default;
		~Mesh() = default;

		Mesh(const std::vector<glm::vec3>& positions, const std::vector<glm::vec3>& normals, const std::vector<glm::vec2>& uvs, const std::vector<int>& indices);

		Mesh(const Mesh& mesh)
		{
			this->vertices = mesh.vertices;
			this->indices = mesh.indices;
		}

		Mesh& operator=(const Mesh& mesh)
		{
			if (&mesh == this) {
				return *this;
			}
			this->vertices = mesh.vertices;
			this->indices = mesh.indices;
			return *this;
		}

		void addTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2);

		Mesh& setPositions(const std::vector<glm::vec3>& positions);
		Mesh& setNormals(const std::vector<glm::vec3>& normals);
		Mesh& setTangents(const std::vector<glm::vec4>& tangents);
		Mesh& setColors(const std::vector <glm::vec4>& colors);
		Mesh& setUvs(const std::vector<glm::vec2>& uvs);
		Mesh& setIndices(const std::vector<int>& indices);
		Mesh& build();

		static Mesh createPlaneMesh();
		static Mesh createPlaneMesh2();

		static Mesh createBoxMesh();
		static Mesh createBoxMesh2();
	
		std::vector<Vertex> vertices;
		std::vector<int> indices;

		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec4> tangents;
		std::vector<glm::vec2> uvs;
		std::vector<glm::vec4> colors;

		uint32_t flag = 0;
	};
}