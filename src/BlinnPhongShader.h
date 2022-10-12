#pragma once
#include "Shader.h"

namespace SoftRenderer
{
    #define BlinnPhongShaderAttributes BaseShaderAttributes

    struct BlinnPhongShaderVaryings : BaseShaderVaryings
    {
        glm::vec2 textureCoord;
        glm::vec3 worldNormal;
        glm::vec3 cameraDirection;
        glm::vec3 lightDirection;
    };

    struct BlinnPhongShaderUniforms : BaseShaderUniforms
    {
        Sampler2D emissiveMap;
        Sampler2D diffuseMap;
        //Sampler2D normalMap;
        //Sampler2D aoMap;
    };

    class BlinnPhongVertexShader : public BaseVertexShader
    {
    public:
        const glm::mat4& getModelMatrix(BlinnPhongShaderAttributes* attributes, BlinnPhongShaderUniforms* uniforms) const
        {
            return uniforms->modelMatrix;
        }


        void shaderMain() override
        {
            auto* a = (BlinnPhongShaderAttributes*)attributes;
            auto* u = (BlinnPhongShaderUniforms*)uniforms;
            auto* v = (BlinnPhongShaderVaryings*)varyings;

            glm::mat4 modelMatrix = u->modelMatrix;
            glm::mat4 modelViewProjMatrix = u->modelViewProjectMatrix;

            glm::vec4 localPosition = glm::vec4(a->position, 1.0f);
            glm::vec4 worldPosition = modelMatrix * localPosition;
            glm::vec4 clipPosition =  modelViewProjMatrix * localPosition;

            v->worldNormal = glm::normalize(u->inverseTransposeModelMatrix * a->normal);
            v->textureCoord = a->textureCoord;
            v->cameraDirection = u->cameraPostion - worldPosition;
            v->lightDirection  = u->lightPosition - worldPosition;

        }
    };

    class BlinnPhongFragmentShader : public BaseFragmentShader
    {
    public:
        
        void shaderMain() override
        {
            auto* u = (BlinnPhongShaderUniforms*)uniforms;
            auto* v = (BlinnPhongShaderVaryings*)varyings;

            glm::vec2 uv = v->textureCoord;

            glm::vec4 baseColor = u->diffuseMap.texture2D(uv);
            glm::vec3 baseColorRGB = glm::vec3(baseColor);


            glm::vec3 diffuseColor = glm::vec3(0.0f);
            glm::vec3 specularColor = glm::vec3(0.0f);

            // emissive
            glm::vec3 emissiveColor = u->emissiveMap.texture2D(uv);

            // ambient
            glm::vec3 ambientColor = baseColorRGB * u->ambientColor;

            const float pointLightRangeInverse = 1.0f / 5.f;
            const float specularExponent = 128.f;

            glm::vec3 lightDirection = v->lightDirection;
            glm::vec3 worldNormal = v->worldNormal;

            // diffuse
            glm::vec3 lightDirectionInverse = lightDirection * pointLightRangeInverse;
            float attenuation = glm::clamp(1.0f - glm::dot(lightDirection, lightDirection), 0.0f, 1.0f);

            glm::vec3 L = glm::normalize(lightDirection);
            glm::vec3 N = glm::normalize(worldNormal);
            float diffuse = glm::max(glm::dot(N, L), 0.0f);

            glm::vec3 pointLightColor = glm::vec3(1.0f, 0.0f, 0.0f);

            diffuseColor = pointLightColor * baseColorRGB * diffuse * attenuation;

            // specular
            glm::vec3 cameraDirection = glm::normalize(v->cameraDirection);
            glm::vec3 H = glm::normalize(lightDirection + cameraDirection);
            float specularAngle = glm::max(glm::dot(N, H), 0.0f);
            specularColor = glm::vec3(glm::pow(specularAngle, specularExponent));


            gl_FragColor = glm::vec4(ambientColor + diffuseColor + specularColor + emissiveColor, baseColor.a);

        }
    };
}