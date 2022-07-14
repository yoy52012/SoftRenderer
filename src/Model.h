#pragma once

#include <string>

#include "Mesh.h"
#include "Transform.h"

namespace SoftRenderer
{

	struct ModelMesh {
		size_t idx = 0;
		size_t face_cnt = 0;
		std::vector<Vertex> vertexes;
		std::vector<int> indices;
		std::unordered_map<int, Texture> textures;
		BoundingBox bounding_box{};

		// material param
		ShadingType shading_type = ShadingType_UNKNOWN;
		AlphaMode alpha_mode = Alpha_Opaque;
		float alpha_cutoff = 0.5f;
		bool double_sided = false;

		ModelMesh(ModelMesh&& o) = default;
		ModelMesh() = default;
	};

	struct ModelNode {
		glm::mat4 transform;
		std::vector<ModelMesh> meshes;
		std::vector<ModelNode> children;
	};

	class Model
	{
	public:
		Model();
		~Model();

		bool loadFromFile(const std::string& filepath);

		unsigned int getId() const;

		Transform transform;

		Mesh mMesh;

	private:
		unsigned int mId;
	};


}