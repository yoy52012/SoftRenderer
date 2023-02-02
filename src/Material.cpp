#include "Material.h"
#include "Graphics.h"

namespace SoftRenderer
{

    void Material::setShaderProgram(std::shared_ptr<Program>& program)
    {
        if(program)
        {
            mProgram  = program;
            mUniforms = program->uniforms; 
        }
    }

    void Material::setModelMatrix(const glm::mat4& modelMatrix)
    {
        if(mUniforms != nullptr)
        {
            mUniforms->modelMatrix = modelMatrix;
        }
    }

    void Material::setModelViewProjectMatrix(const glm::mat4& modelViewProjectMatrix)
    {
        if(mUniforms != nullptr)
        {
            mUniforms->modelViewProjectMatrix = modelViewProjectMatrix;
        }
    }

    void Material::setInverseTransposeModelMatrix(const glm::mat3& inverseTransposeModelMatrix)
    {
        if(mUniforms != nullptr)
        {
            mUniforms->inverseTransposeModelMatrix = inverseTransposeModelMatrix;
        }
    }

    void Material::setLightPosition(const glm::vec3& lightPosition)
    {
        if(mUniforms != nullptr)
        {
            mUniforms->lightPosition = lightPosition;
        }
    }

    void Material::setLightColor(const glm::vec3& lightColor)
    {
        if(mUniforms != nullptr)
        {
            mUniforms->lightColor = lightColor;
        }
    }

    void Material::setLightDistance(const float lightDistance)
    {
        if(mUniforms != nullptr)
        {
            mUniforms->lightDistance = lightDistance;
        }
    }
    
    void Material::setLightDecay(const float lightDecay)
    {
        if(mUniforms != nullptr)
        {
            mUniforms->lightDecay = lightDecay;
        }
    }

    void Material::setCameraPosition(const glm::vec3& cameraPosition)
    {
        if(mUniforms != nullptr)
        {
            mUniforms->cameraPostion = cameraPosition;
        }
    }

    void Material::bind()
    {
        Graphics::instance().useProgram(mProgram);
    }

    void Material::unbind()
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
                std::shared_ptr<BlinnPhongShaderUniforms> bpUniforms = std::dynamic_pointer_cast<BlinnPhongShaderUniforms>(mUniforms);
                bpUniforms->diffuseColor      = mDiffuseColor;
                bpUniforms->specularColor     = mSpecularColor;
                bpUniforms->specularShininess = mSpecularShininess;
                bpUniforms->specularStrength  = mSpecularStrength; 

                if(mEmissiveTexture) bpUniforms->emissiveMap.bindTexture(mEmissiveTexture.get());
                if(mDiffuseTexture)  bpUniforms->diffuseMap.bindTexture(mDiffuseTexture.get());
                if(mNormalTexture)   bpUniforms->normalMap.bindTexture(mNormalTexture.get());
                if(mAOTexture)       bpUniforms->aoMap.bindTexture(mAOTexture.get());
            }
        }
    }

}