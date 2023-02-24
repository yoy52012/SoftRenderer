#pragma once

#include <string>
#include <unordered_map>

#include "Singleton.h"
#include "BlinnPhongShader.h"
#include "SkyboxShader.h"

namespace SoftRenderer
{
    class ShaderManager : public Singleton<ShaderManager>
    {
        friend class Singleton<ShaderManager>;
    public:
        void init();
        void deinit();
        std::shared_ptr<Program> getShaderProgram(const std::string& shaderName);

    private:
        void createShader();

    private:
        std::unordered_map<std::string, std::shared_ptr<Program>> mShaders;

    };
} 