#include "ShaderManagement.h"
#include "BlinnPhongShader.h"
#include "SkyboxShader.h"

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
        auto blinnPhongProgram          = std::make_shared<Program>();
        auto blinnPhongVertexShader     = std::make_shared<BlinnPhongShader::BlinnPhongVertexShader>();
        auto blinnPhongFragmentShader   = std::make_shared<BlinnPhongShader::BlinnPhongFragmentShader>();
        blinnPhongProgram->attach(blinnPhongVertexShader, blinnPhongFragmentShader);
        blinnPhongProgram->link();
        mShaders.emplace("BlinnPhong", blinnPhongProgram);


        auto skyboxProgram          = std::make_shared<Program>();
        auto skyboxVertexShader     = std::make_shared<SkyboxShader::SkyboxVertexShader>();
        auto skyboxFragmentShader   = std::make_shared<SkyboxShader::SkyboxFragmentShader>();
        skyboxProgram->attach(skyboxVertexShader, skyboxFragmentShader);
        skyboxProgram->link();
        mShaders.emplace("Skybox", skyboxProgram);
        
    }
}