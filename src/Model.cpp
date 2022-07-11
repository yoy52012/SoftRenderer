#include "Model.h"

#include <assimp/Exporter.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <sstream>
#include <iostream>

namespace SoftRenderer

{
	Model::Model()
	{

	}

	Model::~Model()
	{

	}

	static bool LoadNode( const aiScene* ai_scene, const aiNode* ai_node, aiMatrix4x4 transform, ModelNode& out_node, glm::mat4& transform)
	{
		if (!ai_node) {
			return false;
		}

		auto curr_transform = transform * ai_node->mTransformation;

		out_node.transform = ConvertMatrix(ai_node->mTransformation);
		auto curr_transform = transform * out_node.transform;

		for (size_t i = 0; i < ai_node->mNumMeshes; i++) 
		{
			const aiMesh* ai_mesh = ai_scene->mMeshes[ai_node->mMeshes[i]];

			if (ai_mesh) 
			{
				ModelMesh mesh;
				if (ProcessMesh(ai_scene, ai_mesh, mesh))
				{
					curr_model_->mesh_count++;
					curr_model_->face_count += mesh.face_cnt;
					curr_model_->vertex_count += mesh.vertexes.size();
					mesh.idx = curr_model_->mesh_count - 1;

					// bounding box
					auto bounds = mesh.bounding_box.Transform(curr_transform);
					curr_model_->root_bounding_box.Merge(bounds);

					out_node.meshes.push_back(std::move(mesh));
				}
			}
		}

		for (size_t i = 0; i < ai_node->mNumChildren; i++) 
		{
			ModelNode child_node;
			if (ProcessNode(ai_node->mChildren[i], ai_scene, child_node, curr_transform)) {
				out_node.children.push_back(std::move(child_node));
			}
		}
		return true;
	}


	bool Model::loadFromFile(const std::string& filepath)
	{
		// load model
		Assimp::Importer importer;
		if (filepath.empty()) 
		{
			std::cout << "Model's file path is empty." << std::endl;
			return false;
		}

		// aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_SplitLargeMeshes | aiProcess_FixInfacingNormals
		const aiScene* scene = importer.ReadFile(filepath,
			aiProcess_Triangulate |
			aiProcess_CalcTangentSpace |
			aiProcess_GenBoundingBoxes);

		if (!scene
			|| scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE
			|| !scene->mRootNode) {
			std::cerr << "Parsing scene: " << filepath << "error! " << "Description: "<< importer.GetErrorString() << std::endl;
			return false;
		}

	}
}