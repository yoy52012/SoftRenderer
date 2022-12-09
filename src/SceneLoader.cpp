#include "SceneLoader.h"

#include <iostream>
#include <set>
#include <assimp/Importer.hpp>
#include <assimp/matrix4x4.h>
#include <assimp/postprocess.h>
#include <assimp/GltfMaterial.h>


namespace SoftRenderer
{
    
#define FILE_SEPARATOR '/'

    bool SceneLoader::loadModel(const std::string &filepath, std::shared_ptr<Mesh> mesh)
    {
        // std::lock_guard<std::mutex> lk(model_load_mutex_);
        if (filepath.empty()) 
        {
            std::cerr << "[Error] loadModel, empty model file path." << std::endl;
            return false;
        }

        // auto it = model_cache_.find(filepath);
        // if (it != model_cache_.end()) 
        // {
        //     curr_model_ = &it->second;
        //     return true;
        // }

        // model_cache_[filepath] = ModelContainer();
        // curr_model_ = &model_cache_[filepath];

        std::cout << "load model path: " << filepath << std::endl;

        // load model
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(filepath,
                                                aiProcess_Triangulate |
                                                aiProcess_CalcTangentSpace |
                                                aiProcess_GenBoundingBoxes);
        if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
        {
            std::cerr << "[Error] loadModel error: " << importer.GetErrorString() << std::endl;
            return false;
        }

        auto model_file_dir = filepath.substr(0, filepath.find_last_of(FILE_SEPARATOR));

        // preload textures
        //PreloadSceneTextureFiles(scene, curr_model_->model_file_dir);

        aiMatrix4x4 identity;
        if (!processNode(scene->mRootNode, scene, identity, mesh)) 
        {
            std::cerr << "[Error] loadModel, process node failed." << std::endl;
            return false;
        }

        return true;
    }

    bool SceneLoader::processNode(const aiNode *inNode, const aiScene *inScene, const aiMatrix4x4& transform, std::shared_ptr<Mesh> mesh) 
    {
        if (!inNode) 
        {
            return false;
        }

        //outNode.transform = convertMatrix(inNode->mTransformation);

        auto currentTransform = transform * inNode->mTransformation;

        for (size_t i = 0; i < inNode->mNumMeshes; i++) 
        {
            const aiMesh *meshPtr = inScene->mMeshes[inNode->mMeshes[i]];
            if (meshPtr) 
            {
                if (processMesh(meshPtr, inScene, currentTransform, mesh)) 
                {
                    //curr_model_->mesh_count++;
                    //curr_model_->face_count += mesh.face_cnt;
                    //curr_model_->vertex_count += mesh.vertexes.size();
                    //mesh.idx = curr_model_->mesh_count - 1;

                    // bounding box
                    //auto bounds = mesh.bounding_box.Transform(currentTransform);
                    //curr_model_->root_bounding_box.Merge(bounds);

                    //outNode.meshes.emplace_back(std::move(mesh));
                }
            }
        }

        for (size_t i = 0; i < inNode->mNumChildren; i++) 
        {
            processNode(inNode->mChildren[i], inScene, currentTransform, mesh);

            //SceneNode childNode;
            
            // if () 
            // {
            //     outNode.children.emplace_back(std::move(childNode));
            // }
        }

        return true;
    }

    BoxSphereBounds convertBoundingBox(const aiAABB &aabb, const aiMatrix4x4& meshTransform) 
    {
        aiVector3D bmin = meshTransform * aabb.mMin;
        aiVector3D bmax = meshTransform * aabb.mMax;
        Box box(glm::vec3(bmin.x, bmin.y, bmin.z), glm::vec3(bmax.x, bmax.y, bmax.z));
        BoxSphereBounds bound(box);
        return bound;
    }

    glm::mat4 convertMatrix(const aiMatrix4x4& m) 
    {
        glm::mat4 ret;
        for (int i = 0; i < 4; i++) 
        {
            for (int j = 0; j < 4; j++) 
            {
                ret[j][i] = m[i][j];
            }
        }
        return ret;
    }

    bool SceneLoader::processMesh(const aiMesh *inMesh, const aiScene *inScene, const aiMatrix4x4& meshTransform, std::shared_ptr<Mesh> outMesh) 
    {
        std::vector<Vertex> vertexes;
        std::vector<uint32_t> indices;

        //std::unordered_map<int, Texture> textures;

        for (size_t i = 0; i < inMesh->mNumVertices; i++) 
        {
            Vertex vertex {};

            if (inMesh->HasPositions()) 
            {
                aiVector3D position	= meshTransform * inMesh->mVertices[i];

                vertex.position.x = position.x;
                vertex.position.y = position.y;
                vertex.position.z = position.z;
            }

            if (inMesh->HasTextureCoords(0)) 
            {
                vertex.texcoord.x = inMesh->mTextureCoords[0][i].x;
                vertex.texcoord.y = inMesh->mTextureCoords[0][i].y;
            } 
            else 
            {
                vertex.texcoord = glm::vec2(0.0f, 0.0f);
            }

            if (inMesh->HasNormals()) 
            {
                aiVector3D normal = meshTransform *inMesh->mNormals[i];

                vertex.normal.x = normal.x;
                vertex.normal.y = normal.y;
                vertex.normal.z = normal.z;
            }
            else
            {
                vertex.normal = glm::vec3(0.0f, 0.0f, 0.0f);
            }


            if (inMesh->HasTangentsAndBitangents()) 
            {
                aiVector3D tangent = meshTransform * inMesh->mTangents[i];

                vertex.tangent.x = tangent.x;
                vertex.tangent.y = tangent.y;
                vertex.tangent.z = tangent.z;
            }
            else
            {
                vertex.tangent = glm::vec3(0.0f, 0.0f, 0.0f);
            }
            
            vertexes.push_back(vertex);
        }

        for (size_t i = 0; i < inMesh->mNumFaces; i++) 
        {
            aiFace face = inMesh->mFaces[i];
            if (face.mNumIndices != 3) 
            {
                std::cerr << "[Error] processMesh, mesh not transformed to triangle mesh." << std::endl;
                return false;
            }

            for (size_t j = 0; j < face.mNumIndices; ++j) 
            {
                indices.push_back(face.mIndices[j]);
            }
        }

        // if (ai_mesh->mMaterialIndex >= 0) 
        // {
        //     const aiMaterial *material = ai_scene->mMaterials[ai_mesh->mMaterialIndex];
        //     aiString alphaMode;
        //     material->Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode);
        //     if (aiString("MASK") == alphaMode) {
        //     out_mesh.alpha_mode = Alpha_Mask;
        //     } else if (aiString("BLEND") == alphaMode) {
        //     out_mesh.alpha_mode = Alpha_Blend;
        //     } else {
        //     out_mesh.alpha_mode = Alpha_Opaque;
        //     }
        //     material->Get(AI_MATKEY_GLTF_ALPHACUTOFF, out_mesh.alpha_cutoff);
        //     material->Get(AI_MATKEY_TWOSIDED, out_mesh.double_sided);

        //     aiShadingMode shading_mode;
        //     material->Get(AI_MATKEY_SHADING_MODEL, shading_mode);
        //     if (aiShadingMode_Blinn == shading_mode) {
        //     out_mesh.shading_type = ShadingType_PBR_BRDF;
        //     } else if (aiShadingMode_PBR_BRDF == shading_mode) {
        //     out_mesh.shading_type = ShadingType_PBR_BRDF;
        //     } else {
        //     out_mesh.shading_type = ShadingType_UNKNOWN;
        //     }

        //     for (int i = 0; i <= AI_TEXTURE_TYPE_MAX; i++) {
        //     ProcessMaterial(material, static_cast<aiTextureType>(i), textures);
        //     }
        // }

        std::shared_ptr<SubMesh> submesh = std::make_shared<SubMesh>(vertexes, indices);

        outMesh->addSubMesh(submesh);
        outMesh->mBounds += convertBoundingBox(inMesh->mAABB, meshTransform);

        return true;
    }


}