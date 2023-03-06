#include "Material.h"
#include "Graphics.h"
#include "ShaderManagement.h"

namespace SoftRenderer
{

    void Material::setShaderProgram(std::shared_ptr<Program>& program)
    {
        // if(program)
        // {
        //     mProgram  = program;
        // }
    }

    // void Material::setModelMatrix(const glm::mat4& modelMatrix)
    // {
    //     if(mUniforms != nullptr)
    //     {
    //         mUniforms->modelMatrix = modelMatrix;
    //     }
    // }

    // void Material::setModelViewProjectMatrix(const glm::mat4& modelViewProjectMatrix)
    // {
    //     if(mUniforms != nullptr)
    //     {
    //         mUniforms->modelViewProjectMatrix = modelViewProjectMatrix;
    //     }
    // }

    // void Material::setInverseTransposeModelMatrix(const glm::mat3& inverseTransposeModelMatrix)
    // {
    //     if(mUniforms != nullptr)
    //     {
    //         mUniforms->inverseTransposeModelMatrix = inverseTransposeModelMatrix;
    //     }
    // }

    void Material::setLightPosition(const glm::vec3& lightPosition)
    {
        // if(mUniforms != nullptr)
        // {
        //     mUniforms->lightPosition = lightPosition;
        // }
    }

    void Material::setLightColor(const glm::vec3& lightColor)
    {
        // if(mUniforms != nullptr)
        // {
        //     mUniforms->lightColor = lightColor;
        // }
    }

    void Material::setLightDistance(const float lightDistance)
    {
        // if(mUniforms != nullptr)
        // {
        //     mUniforms->lightDistance = lightDistance;
        // }
    }
    
    void Material::setLightDecay(const float lightDecay)
    {

    }

    void Material::setCameraPosition(const glm::vec3& cameraPosition)
    {
        // if(mUniforms != nullptr)
        // {
        //     mUniforms->cameraPostion = cameraPosition;
        // }
    }

    void Material::bind()
    {
        Graphics::instance().useProgram(mProgram);
    }

    void Material::unbind()
    {
        
    }

    BlinnPhongMaterial::BlinnPhongMaterial()
    {
        mProgram = ShaderManager::instance().getShaderProgram("BlinnPhong");
        mUniforms = std::make_shared<BlinnPhongShader::ShaderUniforms>();
    }

    BlinnPhongMaterial::~BlinnPhongMaterial()
    {

    }

    void BlinnPhongMaterial::setDiffuseColor(const glm::vec3 &diffuseColor)
    {
        mDiffuseColor = diffuseColor;
    }

    void BlinnPhongMaterial::setSpecularColor(const glm::vec3 &specularColor)
    {
        mSpecularColor = specularColor;
    }

    void BlinnPhongMaterial::setSpecularShininess(float specularShininess)
    {
        mSpecularShininess = specularShininess;
    }

    void BlinnPhongMaterial::setSpecularStrength(float specularStrength)
    {
        mSpecularStrength = specularStrength;
    }

    void BlinnPhongMaterial::setEmmissiveTexture(std::shared_ptr<Texture> emissiveTexture)
    {
        mEmissiveTexture = emissiveTexture;
    }

    void BlinnPhongMaterial::setDiffuseTexture(std::shared_ptr<Texture> diffuseTexture)
    {
        mDiffuseTexture = diffuseTexture;
    }

    void BlinnPhongMaterial::setSpecularTexture(std::shared_ptr<Texture> specularTexture)
    {
        mSpecularTexture = specularTexture;
    }

    void BlinnPhongMaterial::setNormalTexture(std::shared_ptr<Texture> normalTexture)
    {
        mNormalTexture = normalTexture;
    }

    void BlinnPhongMaterial::setAOTexture(std::shared_ptr<Texture> aoTexture)
    {
        mAOTexture = aoTexture;
    }

    void BlinnPhongMaterial::updateParameters()
    {
        if(mProgram != nullptr)
        {
            if(mUniforms != nullptr)
            {
                mUniforms->diffuseColor      = mDiffuseColor;
                mUniforms->specularColor     = mSpecularColor;
                mUniforms->specularShininess = mSpecularShininess;
                mUniforms->specularStrength  = mSpecularStrength; 

                if(mEmissiveTexture) mUniforms->emissiveMap.bindTexture(mEmissiveTexture.get());
                if(mDiffuseTexture)  mUniforms->diffuseMap.bindTexture(mDiffuseTexture.get());
                if(mNormalTexture)   mUniforms->normalMap.bindTexture(mNormalTexture.get());
                if(mAOTexture)       mUniforms->aoMap.bindTexture(mAOTexture.get());

                mProgram->bindUniform(mUniforms.get(), sizeof(BlinnPhongShader::ShaderUniforms));
            }
        }
    }

    SkyboxMaterial::SkyboxMaterial()
    {
        mProgram = ShaderManager::instance().getShaderProgram("Skybox");
        mUniforms = std::make_shared<SkyboxShader::ShaderUniforms>();
    }

    SkyboxMaterial::~SkyboxMaterial()
    {

    }

    void SkyboxMaterial::setModelViewProjectMatrix(const glm::mat4& modelViewProjectMatrix)
    {
        if(mUniforms != nullptr)
        {
            mUniforms->modelViewProjectMatrix = modelViewProjectMatrix;
        }
    }

    void SkyboxMaterial::setCubemapTexture(std::shared_ptr<Texture>& texture, CubeMapFace face)
    {
        mCubemapTextures[(int)face] = texture;
    }

    void SkyboxMaterial::updateParameters()
    {
        if(mProgram != nullptr)
        {
            if(mUniforms != nullptr)
            {
                if(mCubemapTextures[0]) mUniforms->cubeMap.bindTexture(mCubemapTextures[0].get(), CubeMapFace::TEXTURE_CUBE_MAP_POSITIVE_X);
                if(mCubemapTextures[1]) mUniforms->cubeMap.bindTexture(mCubemapTextures[1].get(), CubeMapFace::TEXTURE_CUBE_MAP_NEGATIVE_X);
                if(mCubemapTextures[2]) mUniforms->cubeMap.bindTexture(mCubemapTextures[2].get(), CubeMapFace::TEXTURE_CUBE_MAP_POSITIVE_Y);
                if(mCubemapTextures[3]) mUniforms->cubeMap.bindTexture(mCubemapTextures[3].get(), CubeMapFace::TEXTURE_CUBE_MAP_NEGATIVE_Y);
                if(mCubemapTextures[4]) mUniforms->cubeMap.bindTexture(mCubemapTextures[4].get(), CubeMapFace::TEXTURE_CUBE_MAP_POSITIVE_Z);
                if(mCubemapTextures[5]) mUniforms->cubeMap.bindTexture(mCubemapTextures[5].get(), CubeMapFace::TEXTURE_CUBE_MAP_NEGATIVE_Z);

                mProgram->bindUniform(mUniforms.get(), sizeof(SkyboxShader::ShaderUniforms));
            }
        }
    }

}