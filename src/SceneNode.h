#pragma once

#include <vector>

#include "Mesh.h"
#include "MathUtils.h"

namespace SoftRenderer
{
    using SceneId = unsigned int;

    class SceneNode
    {
    public:
        SceneNode(SceneId id);
        SceneNode() = default;
        
        ~SceneNode() = default;
        SceneNode(const SceneNode& other) = delete;
        SceneNode(SceneNode&& other) = default;
        SceneNode& operator=(const SceneNode& other) = default;

        SceneId getId() const;

        //Transform transform;

        std::vector<Mesh> meshes;
        glm::mat4 transform;

        std::vector<SceneNode> children;
        
    private:
    };


}