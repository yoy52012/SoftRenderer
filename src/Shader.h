#pragma once

#include <memory>
#include <iostream>

#include "MathUtils.h"
#include "Mesh.h"
#include "Texture.h"

namespace SoftRenderer
{
    class Shader
    {
    public:
        using Ptr = std::shared_ptr<Shader>;

        struct VertexData
        {
            glm::vec4 worldPosition;
            glm::vec3 worldNormal;
            glm::vec2 texcoord;
            glm::vec4 color;
            glm::vec4 clipPostion;
            glm::vec2 screenPosition;
            float clipW;

            static VertexData lerp(const VertexData& v1, const VertexData& v2, float f)
            {
                VertexData o = {};
                o.worldPosition = glm::lerp(v1.worldPosition, v2.worldPosition, f);
                o.worldNormal = glm::lerp(v1.worldNormal, v2.worldNormal, f);
                o.texcoord = glm::lerp(v1.texcoord, v2.texcoord, f);
                o.color = glm::lerp(v1.color, v2.color, f);
                o.clipPostion = glm::lerp(v1.clipPostion, v2.clipPostion, f);
                o.screenPosition = glm::lerp(v1.screenPosition, v2.screenPosition, f);

                return o;
            }

            static VertexData barycentricLerp(const VertexData& v0, const VertexData& v1, const VertexData& v2, const glm::vec3& w)
            {
                VertexData o = {};
                o.worldPosition = w.x * v0.worldPosition + w.y * v1.worldPosition + w.z * v2.worldPosition;
                o.worldNormal = w.x * v0.worldNormal + w.y * v1.worldNormal + w.z * v2.worldNormal;
                o.texcoord = w.x * v0.texcoord + w.y * v1.texcoord + w.z * v2.texcoord;
                o.color = w.x * v0.color + w.y * v1.color + w.z * v2.color;
                o.screenPosition = w.x * v0.screenPosition + w.y * v1.screenPosition + w.z * v2.screenPosition;
                o.clipW = w.x * v0.clipW + w.y * v1.clipW + w.z * v2.clipW;
                return o;
            }
        };


        struct Uniforms
        {
            Sampler2D albedoMap;
        };

    public:
        Shader() = default;
        ~Shader() = default;

        Shader& operator= (const Shader& shader)
        {
            mModelMatrix = shader.mModelMatrix;
            mViewMatrix = shader.mViewMatrix;
            mProjMatrix = shader.mProjMatrix;

            return *this;
        }

        void setModelMatrix(const glm::mat4& modelMat)
        {
            mModelMatrix = modelMat;
        }

        void setViewMatrix(const glm::mat4& viewMat)
        {
            mViewMatrix = viewMat;
        }

        void setProjMatrix(const glm::mat4& projMat)
        {
            mProjMatrix = projMat;
        }

        virtual VertexData vertexShader(const Vertex& iv);

        virtual glm::vec4 fragmentShader(const VertexData& v2f);

        Uniforms mUniforms;


    protected:
        glm::mat4 mModelMatrix = glm::mat4(1.0f);
        glm::mat4 mViewMatrix = glm::mat4(1.0f);
        glm::mat4 mProjMatrix = glm::mat4(1.0f);

    };

    #define CLONE_VERTEX_SHADER(T) \
    std::shared_ptr<BaseVertexShader> clone() override \
    { \
        auto ret = std::make_shared<T>(); \
        ret->uniforms = uniforms; \
        ret->attributes = attributes; \
        return ret; \
    } \

    #define CLONE_FRAGMENT_SHADER(T) \
    std::shared_ptr<BaseFragmentShader> clone() override \
    { \
        auto ret = std::make_shared<T>(); \
        ret->uniforms = uniforms; \
        return ret; \
    } \

    struct BaseShaderAttributes
    {
        glm::vec3 position;
        glm::vec2 textureCoord;
        glm::vec3 normal;
        glm::vec3 tangent;
    };

    struct BaseShaderUniforms
    {
        BaseShaderUniforms() = default;
        virtual ~BaseShaderUniforms() = default;

        glm::mat4 modelMatrix;
        glm::mat4 modelViewProjectMatrix;
        glm::mat3 inverseTransposeModelMatrix;

        glm::vec3 lightPosition;
        glm::vec3 lightColor;
        float lightIntensity = 1.0f;
        float lightDistance  = 0.0f;
        float lightDecay = 2.0f;

        glm::vec3 cameraPostion;

        Sampler2D uAlbedoMap;
    };

    struct BaseShaderVaryings
    {
    };

    struct BaseShader
    {
    public:
        virtual void shaderMain() = 0;
    };

    class BaseVertexShader : BaseShader
    {
    public:
        // Built-in variables
        glm::vec4 gl_Position;

        // Custom variables
        BaseShaderAttributes* attributes = nullptr;
        BaseShaderUniforms*   uniforms = nullptr;
        BaseShaderVaryings*   varyings = nullptr;

        void shaderMain() override
        {
            auto* a = (BaseShaderAttributes*)attributes;
            auto* u = (BaseShaderUniforms*)uniforms;
            auto* v = (BaseShaderVaryings*)varyings;

            glm::vec4 position = glm::vec4(a->position, 1.0f);
            gl_Position = u->modelViewProjectMatrix * position;
        }

        virtual std::shared_ptr<BaseVertexShader> clone() 
        {
            auto ret = std::make_shared<BaseVertexShader>();
            ret->uniforms = uniforms;
            ret->attributes = attributes;
            return ret;
        }

    };

    class BaseFragmentShader : BaseShader
    {
    public:

        // inner input
        glm::vec4 gl_FragCoord;
        bool gl_FrontFacing;

        // inner output
        float gl_FragDepth;
        glm::vec4 gl_FragColor;
        bool discard = false;

        // Custom variables
        BaseShaderUniforms* uniforms = nullptr;
        BaseShaderVaryings* varyings = nullptr;

        float LinearizeDepth(float depth)
        {
            float n = 0.1;
            float f = 100.0;
            float z = (depth * 2.0 - 1.0); // Back to NDC
            return (2.0 * n * f) / (f + n - z * (f - n));
        }


        void shaderMain() override
        {
            gl_FragDepth = gl_FragCoord.z;
        }

        virtual std::shared_ptr<BaseFragmentShader> clone() 
        {
            auto ret = std::make_shared<BaseFragmentShader>();
            ret->uniforms = uniforms;
            return ret;
        }

    };

    struct Program
    {
        size_t varyingsCount;
        std::shared_ptr<BaseShaderUniforms> uniforms;
        std::shared_ptr<BaseVertexShader> vertexShader;
        std::shared_ptr<BaseFragmentShader> fragmentShader;

        void attach(std::shared_ptr<BaseVertexShader> inVertexShader, std::shared_ptr<BaseFragmentShader> inFragmentShader, std::shared_ptr<BaseShaderUniforms> inUniforms)
        {
            vertexShader = inVertexShader;
            fragmentShader = inFragmentShader;
            uniforms = inUniforms;
        }
    
        bool link()
        {
            if (vertexShader == nullptr)
            {
                std::cerr << "Program link error : Vertex shader is null." << std::endl;
                return false;
            }

            if (fragmentShader == nullptr)
            {
                std::cerr << "Program link error : Fragment shade is null." << std::endl;
                return false;
            }
            
            vertexShader->uniforms = uniforms.get();
            fragmentShader->uniforms = uniforms.get();
            varyingsCount = getVaringsCount();

            return true;
        }

    private:
        virtual size_t getVaringsCount()
        {
            return sizeof(BaseShaderVaryings) / sizeof(float);
        }

    };

}