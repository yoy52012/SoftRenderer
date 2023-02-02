#include "ShaderManagement.h"

namespace SoftRenderer
{
    void ShaderManager::init()
    {
        createShader();
    }

    void ShaderManager::deinit()
    {

    }

    std::shared_ptr<Program> ShaderManager::getShaderProgram(const std::string& shaderName)
    {
        if(mShaders.find(shaderName) == mShaders.end())
            return nullptr;
        return mShaders[shaderName];
    }

    void ShaderManager::createShader()
    {
        auto baseProgram          = std::make_shared<Program>();
        auto baseVertexShader     = std::make_shared<BaseVertexShader>();
        auto baseFragmentShader   = std::make_shared<BaseFragmentShader>();
        auto baseUniforms         = std::make_shared<BaseShaderUniforms>();
        baseProgram->vertexShader = baseVertexShader;
        baseProgram->fragmentShader = baseFragmentShader;
        baseProgram->uniforms = baseUniforms;
        baseProgram->link();
        mShaders.emplace("Base", baseProgram);


        auto blinnPhongProgram          = std::make_shared<BlinnPhongProgram>();
        auto blinnPhongVertexShader     = std::make_shared<BlinnPhongVertexShader>();
        auto blinnPhongFragmentShader   = std::make_shared<BlinnPhongFragmentShader>();
        auto blinnPhongUniforms         = std::make_shared<BlinnPhongShaderUniforms>();
        blinnPhongProgram->vertexShader = blinnPhongVertexShader;
        blinnPhongProgram->fragmentShader = blinnPhongFragmentShader;
        blinnPhongProgram->uniforms = blinnPhongUniforms;
        blinnPhongProgram->link();
        mShaders.emplace("BlinnPhong", blinnPhongProgram);
        
    }
}