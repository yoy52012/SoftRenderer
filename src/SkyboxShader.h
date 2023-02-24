#pragma once
#include "Shader.h"

namespace SoftRenderer
{

    #define SkyboxShaderAttributes BaseShaderAttributes

    struct SkyboxShaderUniforms : BaseShaderUniforms
    {
        SamplerCube cubeMap;
        Sampler2D   equirectangularMap;
    };

    struct SkyboxShaderVaryings : BaseShaderVaryings
    {
        glm::vec3 textureCoord;
    };

    class SkyboxVertexShader : public BaseVertexShader
    {
    public:
        void shaderMain() override
        {
            auto *a = (SkyboxShaderAttributes *) attributes;
            auto *u = (SkyboxShaderUniforms *) uniforms;
            auto *v = (SkyboxShaderVaryings *) varyings;

            glm::vec4 position = glm::vec4(a->position, 1.0f);
            gl_Position = u->modelViewProjectMatrix * position;
            gl_Position.z = gl_Position.w;  // depth -> 1.0

            v->textureCoord = a->position;
        }
        CLONE_VERTEX_SHADER(SkyboxVertexShader)
    };

    class SkyboxFragmentShader : public BaseFragmentShader
    {
    public:
        glm::vec2 SampleSphericalMap(glm::vec3 dir) 
        {
            const glm::vec2 invAtan = glm::vec2(0.1591f, 0.3183f);
            glm::vec2 uv = glm::vec2(glm::atan(dir.z, dir.x), asin(dir.y));
            uv *= invAtan;
            uv += 0.5f;
            return uv;
        }

        void shaderMain() override
        {
            BaseFragmentShader::shaderMain();

            SkyboxShaderUniforms *u = (SkyboxShaderUniforms *) uniforms;
            SkyboxShaderVaryings *v = (SkyboxShaderVaryings *) varyings;

            if (!u->equirectangularMap.isEmpty()) 
            {
                glm::vec2 uv = SampleSphericalMap(glm::normalize(v->textureCoord));
                gl_FragColor = u->equirectangularMap.texture2D(uv);
            } 
            else if (!u->cubeMap.isEmpty()) 
            {
                gl_FragColor = u->cubeMap.textureCube(v->textureCoord);
            } 
            else {
                discard = true;
            }
        }
        CLONE_FRAGMENT_SHADER(SkyboxFragmentShader)

    };

    class SkyboxProgram : public Program
    {
    private:
        size_t getVaringsCount() override
        {
            return sizeof(SkyboxShaderVaryings) / sizeof(float);
        }
    };

}