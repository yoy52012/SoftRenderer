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
    std::shared_ptr<BaseVertexShader> clone() \
    { \
        return std::make_shared<T>(*this); \
    } \

    #define CLONE_FRAGMENT_SHADER(T) \
    std::shared_ptr<BaseFragmentShader> clone() \
    { \
        return std::make_shared<T>(*this); \
    } \

    // struct BaseShaderBuiltin
    // {

    // };

    // struct BaseShaderAttributes
    // {
    //     glm::vec3 position;
    //     glm::vec2 textureCoord;
    //     glm::vec3 normal;
    //     glm::vec3 tangent;
    // };

    // struct BaseShaderUniforms
    // {
    //     glm::mat4 modelMatrix;
    //     glm::mat4 modelViewProjectMatrix;
    //     glm::mat3 inverseTransposeModelMatrix;

    //     glm::vec3 lightPosition;
    //     glm::vec3 lightColor;
    //     float lightIntensity = 1.0f;
    //     float lightDistance  = 0.0f;
    //     float lightDecay = 2.0f;

    //     glm::vec3 cameraPostion;

    //     Sampler2D uAlbedoMap;
    // };

    // struct BaseShaderVaryings
    // {
    // };

    class BaseShader
    {
    public:
        virtual void shaderMain() = 0;

        virtual void bindShaderAttributes(void *ptr) = 0;
        virtual void bindShaderUniforms(void *ptr) = 0;
        virtual void bindShaderVaryings(void *ptr) = 0;

        virtual size_t getShaderUniformsSize() = 0;
        virtual size_t getShaderVaryingsSize() = 0;

        int32_t getUniformLocation(const std::string &name) 
        {
            // auto &desc = getUniformsDesc();
            // for (int i = 0; i < desc.size(); i++) 
            // {
            //     if (desc[i].name == name) 
            //     {
            //         return i;
            //     }
            // }
            return -1;
        }

        int32_t getUniformOffset(int loc) 
        {
            // auto &desc = getUniformsDesc();
            // if (loc < 0 || loc > desc.size()) 
            // {
            //     return -1;
            // }
            // return desc[loc].offset;
            return -1;
        }

    };

    class BaseVertexShader : public BaseShader
    {
    public:
        // Built-in variables
        glm::vec4 gl_Position;

        virtual std::shared_ptr<BaseVertexShader> clone() = 0;
    };

    class BaseFragmentShader : public BaseShader
    {
    public:
        // Built-in variables
        glm::vec4 gl_FragCoord;
        bool gl_FrontFacing;
        float gl_FragDepth;
        glm::vec4 gl_FragColor;
        bool discard = false;

        virtual std::shared_ptr<BaseFragmentShader> clone() = 0;
    };

    #define CREATE_SHADER                                 \
    ShaderAttributes *a = nullptr;                    \
    ShaderUniforms *u = nullptr;                      \
    ShaderVaryings *v = nullptr;                      \
                                                          \
                                                          \
    void bindShaderAttributes(void *ptr) override          \
    {                                                     \
        a = static_cast<ShaderAttributes *>(ptr);     \
    }                                                     \
                                                          \
    void bindShaderUniforms(void *ptr) override            \
    {                                                     \
        u = static_cast<ShaderUniforms *>(ptr);       \
    }                                                     \
                                                          \
    void bindShaderVaryings(void *ptr) override            \
    {                                                     \
        v = static_cast<ShaderVaryings *>(ptr);       \
    }                                                     \
                                                          \
    size_t getShaderUniformsSize() override               \
    {                                                     \
        return sizeof(ShaderUniforms);                \
    }                                                     \
                                                          \
    size_t getShaderVaryingsSize() override               \
    {                                                     \
        return sizeof(ShaderVaryings);                \
    }

    #define CREATE_SHADER_CLONE(T)                        \
    std::shared_ptr<ShaderSoft> clone() override \
    {        \
        return std::make_shared<T>(*this);                  \
    }

    struct Program
    {
        Program() = default;

        //size_t varyingsCount;
        std::shared_ptr<uint8_t> uniforms;
        std::shared_ptr<BaseVertexShader> vertexShader;
        std::shared_ptr<BaseFragmentShader> fragmentShader;

        void attach(std::shared_ptr<BaseVertexShader> inVertexShader, std::shared_ptr<BaseFragmentShader> inFragmentShader)
        {
            vertexShader = inVertexShader;
            fragmentShader = inFragmentShader;

            // setup unifroms
            uniforms = std::shared_ptr<uint8_t>(new uint8_t[vertexShader->getShaderUniformsSize()], [](const uint8_t *ptr) { delete[] ptr; });
            vertexShader->bindShaderUniforms(uniforms.get());
            fragmentShader->bindShaderUniforms(uniforms.get());
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
            
            //vertexShader->uniforms = uniforms.get();
            //fragmentShader->uniforms = uniforms.get();
            //varyingsCount = getVaringsCount();

            return true;
        }

        void bindVertexAttributes(void *ptr) 
        {
            vertexShader->bindShaderAttributes(ptr);
        }

        void bindUniform(void *data, size_t len) 
        {
            memcpy(uniforms.get(), data, len);
        }

        void bindVertexShaderVaryings(void* ptr)
        {
            vertexShader->bindShaderVaryings(ptr);
        }

        void bindFragmentShaderVaryings(void* ptr)
        {
            fragmentShader->bindShaderVaryings(ptr);
        }

        size_t getShaderVaryingsSize() 
        {
            return vertexShader->getShaderVaryingsSize();
        }

        uint32_t getUniformLocation(const std::string &name) 
        {
            return vertexShader->getUniformLocation(name);
        }

        void executeVertexShader()
        {
            if(vertexShader)
            {
                vertexShader->shaderMain();
            }
        }

        void executeFragmentShader()
        {
            if(fragmentShader)
            {
                fragmentShader->shaderMain();
            }
        }

        std::shared_ptr<Program> clone() const 
        {
            auto ret = std::make_shared<Program>(*this);

            ret->vertexShader = vertexShader->clone();
            ret->fragmentShader = fragmentShader->clone();
            //ret->vertex_shader_->BindBuiltin(&ret->builtin_);
            //ret->fragment_shader_->BindBuiltin(&ret->builtin_);

            return ret;
        }

    private:
        //virtual size_t getVaringsCount()
        //{
        //    return sizeof(BaseShaderVaryings) / sizeof(float);
        //}

    };

}