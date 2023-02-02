#pragma once

#include "MathUtils.h"
#include "BlinnPhongShader.h"

namespace SoftRenderer
{
    class Material
    {
    public:
        Material() = default;
        ~Material() = default;

        void setShaderProgram(std::shared_ptr<Program>& program);

        void setModelMatrix(const glm::mat4& modelMatrix);
        void setModelViewProjectMatrix(const glm::mat4& modelViewProjectMatrix);
        void setInverseTransposeModelMatrix(const glm::mat3& inverseTransposeModelMatrix);

        void setLightPosition(const glm::vec3& lightPosition);
        void setLightColor(const glm::vec3& lightColor);
        void setLightDistance(const float lightDistance);
        void setLightDecay(const float lightDecay);

        void setCameraPosition(const glm::vec3& cameraPosition);

        void bind();
        void unbind();

    protected:
        std::shared_ptr<Program>            mProgram  = nullptr;
        std::shared_ptr<BaseShaderUniforms> mUniforms = nullptr;
    };

    class BlinnPhongMaterial : public Material
    {
    public:
        BlinnPhongMaterial() = default;
        ~BlinnPhongMaterial() = default;

        void setDiffuseColor(const glm::vec3 &diffuseColor);
        void setSpecularColor(const glm::vec3 &specularColor);
        void setSpecularShininess(float specularShininess);
        void setSpecularStrength(float specularStrength);

        void setEmmissiveTexture(std::shared_ptr<Texture> emissiveTexture);
        void setDiffuseTexture(std::shared_ptr<Texture> diffuseTexture);
        void setSpecularTexture(std::shared_ptr<Texture> specularTexture);
        void setNormalTexture(std::shared_ptr<Texture> normalTexture);
        void setAOTexture(std::shared_ptr<Texture> aoTexture);

        void updateParameters();
    private:

    private:
        glm::vec3 mDiffuseColor = glm::vec3(0);
        glm::vec3 mSpecularColor = glm::vec3(0);
        float mSpecularShininess = 128.0f;
        float mSpecularStrength = 1.0f;

        std::shared_ptr<Texture> mEmissiveTexture = nullptr;
        std::shared_ptr<Texture> mDiffuseTexture  = nullptr;
        std::shared_ptr<Texture> mSpecularTexture = nullptr;
        std::shared_ptr<Texture> mNormalTexture   = nullptr;
        std::shared_ptr<Texture> mAOTexture       = nullptr;

    };
}
