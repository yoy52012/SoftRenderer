#pragma once

#include<string>
#include <unordered_map>
#include <assimp/scene.h>

#include"Singleton.h"
#include "SceneNode.h"

namespace SoftRenderer
{
    class SceneLoader : public Singleton<SceneLoader>
    {
        friend class Singleton<SceneLoader>;
    public:
        bool loadModel(const std::string &filepath, std::shared_ptr<Mesh> mesh);

    protected:
        SceneLoader() =  default;
        ~SceneLoader() = default;

    private:
        bool processNode(const aiNode *inNode, const aiScene *inScene, const aiMatrix4x4& transform, std::shared_ptr<Mesh> mesh);
        bool processMesh(const aiMesh *inMesh, const aiScene *inScene, const aiMatrix4x4& meshTransform, std::shared_ptr<Mesh> outMesh);
        //bool processMaterial(const aiMaterial *ai_material, aiTextureType texture_type, std::unordered_map<int, Texture> &textures);

    private:
        //std::unordered_map<std::string, ModelContainer>
    };
} // namespace SoftRenderer
