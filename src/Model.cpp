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

	static void loadMesh(const aiScene* ai_scene, const aiMesh* ai_mesh,  SubMesh& out_mesh)
	{
		std::vector<Vertex> vertices;
		std::vector<int> indices;

		for (size_t i = 0; i < ai_mesh->mNumVertices; i++)
		{
			Vertex vertex = {};
			if (ai_mesh->HasPositions())
			{
				vertex.position.x = ai_mesh->mVertices[i].x;
				vertex.position.y = ai_mesh->mVertices[i].y;
				vertex.position.z = ai_mesh->mVertices[i].z;
			}

			if(ai_mesh->HasTextureCoords(0))
			{
				vertex.texcoord.x = ai_mesh->mTextureCoords[0][i].x;
				vertex.texcoord.y = ai_mesh->mTextureCoords[0][i].y;
			}

			if (ai_mesh->HasNormals())
			{
				vertex.normal.x = ai_mesh->mNormals[i].x;
				vertex.normal.y = ai_mesh->mNormals[i].y;
				vertex.normal.z = ai_mesh->mNormals[i].z;
			}

			if (ai_mesh->HasTangentsAndBitangents())
			{
				vertex.tangent.x = ai_mesh->mTangents[i].x;
				vertex.tangent.y = ai_mesh->mTangents[i].y;
				vertex.tangent.z = ai_mesh->mTangents[i].z;
			}

			if (ai_mesh->HasVertexColors(0))
			{
				vertex.color.x = ai_mesh->mColors[0][i].r;
				vertex.color.y = ai_mesh->mColors[0][i].g;
				vertex.color.z = ai_mesh->mColors[0][i].b;
				vertex.color.w = ai_mesh->mColors[0][i].a;
			}

			vertices.push_back(vertex);
		}

		for (size_t i = 0; i < ai_mesh->mNumFaces; i++)
		{
			const aiFace face = ai_mesh->mFaces[i];
			if (face.mNumIndices != 3)
			{
				std::cout << "Warning: loadMesh, mesh is not a triangle mesh." << std::endl;
				return;
			}

			for (size_t j = 0; j < face.mNumIndices; j++)
			{
				indices.push_back((int)(face.mIndices[j]));
			}
		}

		if (ai_mesh->mMaterialIndex >= 0)
		{
			const aiMaterial* material = ai_scene->mMaterials[ai_mesh->mMaterialIndex];
			aiString alphaMode;

		}

		out_mesh.vertices = vertices;
		out_mesh.indices  = indices;
	}

	static bool loadMaterial()
	{

	}

	static bool LoadNode( const aiScene* ai_scene, const aiNode* ai_node, aiMatrix4x4 transform, ModelNode& out_node, glm::mat4& transform)
	{
		if (!ai_node) 
		{
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