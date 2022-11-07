#pragma once


#include "Mesh.h"
#include "Shader.h"
#include "FrameBuffer.h"

#include <BS_thread_pool_light.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/type_aligned.hpp>


namespace SoftRenderer
{
#define SOFTGL_ALIGNMENT 32

    class Memory
    {
    public:

        static void* alignedMalloc(size_t size, size_t alignment = SOFTGL_ALIGNMENT)
        {
            assert((alignment & (alignment - 1)) == 0);

            size_t extra = alignment + sizeof(void*);
            void* data = malloc(size + extra);
            if (!data) 
            {
                return nullptr;
            }
            
            size_t addr = (size_t)data + extra;
            
            void* aligned_ptr = (void*)(addr - (addr % alignment));
            
            *((void**)aligned_ptr - 1) = data;
            
            return aligned_ptr;
        }

        static void alignedFree(void* ptr)
        {
            if (ptr) 
            {
                free(((void**)ptr)[-1]);
            }
        }
    };




    class Graphics
    {
    public:
        struct DepthRange
        {
            float n;
            float f;
        };

        struct Viewport
        {
            float x;
            float y;
            float width;
            float height;
        };

        struct FaceResource
        {
            int32_t indices[3]{ -1, -1, -1 };

            bool discard = false;
            bool frontFacing = true;
        };

        using FaceBuffer = std::vector<FaceResource>;

        struct VertexResource
        {
            size_t  id = 0;
            Vertex  vertex = {};

            float* varyings = nullptr;

            glm::vec4 position = glm::vec4(0);
        };

        using VertexBuffer = std::vector<VertexResource>;

        struct Fragment 
        {
            glm::aligned_vec4 position = glm::aligned_vec4(0);
            glm::aligned_vec4 barycentric = glm::aligned_vec4(0);
            float* interpolatedVaryings;
            bool inside = false;
        };

        struct FragmentQuad
        {
            /**
             *   p2--p3
             *   |   |
             *   p0--p1
             */
            Fragment pixels[4];

            BaseFragmentShader frag_shader;

            // Triangular vertex screen space position
            glm::aligned_vec4 triangularVertexScreenPosition[3];
            glm::aligned_vec4 triangularVertexScreenPositionFlat[4];
            glm::aligned_vec4 triangularVertexClipZ = glm::aligned_vec4(1.0f);
            const float* triangularVertexVarings[3];

            std::shared_ptr<float> interpolatedVaryingsBuffer = nullptr;

            bool front_facing = true;

            FragmentQuad(size_t varyingsAlignedSize = 0)
            {
                if (varyingsAlignedSize > 0) 
                {
                    interpolatedVaryingsBuffer = std::shared_ptr<float>((float*)Memory::alignedMalloc(4 * varyingsAlignedSize), [](const float* ptr) { Memory::alignedFree((void*)ptr); });
                    for (int i = 0; i < 4; i++) 
                    {
                        pixels[i].interpolatedVaryings = interpolatedVaryingsBuffer.get() + i * varyingsAlignedSize / 4;
                    }
                }
            }

            void init(int x, int y) 
            {
                pixels[0].position.x = x;
                pixels[0].position.y = y;

                pixels[1].position.x = x + 1;
                pixels[1].position.y = y;

                pixels[2].position.x = x;
                pixels[2].position.y = y + 1;

                pixels[3].position.x = x + 1;
                pixels[3].position.y = y + 1;
            }

            bool isInside()
            {
                return pixels[0].inside || pixels[1].inside || pixels[2].inside || pixels[3].inside;
            }

        };


        struct RenderContex
        {
            inline static size_t calculateVaryingsAlignedSize(size_t varySize) 
            {
                return SOFTGL_ALIGNMENT * std::ceil(varySize * sizeof(float) / (float)SOFTGL_ALIGNMENT);
            }

            void createVertexBuffer(const std::vector<Vertex>& vertices)
            {
                const size_t vertexCount = vertices.size();
                vertexBuffer.resize(vertexCount);

                for (int32_t i = 0; i < vertexCount; ++i)
                {
                    vertexBuffer[i].id = i;
                    vertexBuffer[i].vertex = vertices[i];
                    vertexBuffer[i].position = glm::vec4(0);
                }
            }

            void createIndexBuffer(const std::vector<int>& indices)
            {
                const size_t faceCount = indices.size() / 3;
                faceBuffer.resize(faceCount);

                for (int32_t i = 0; i < faceCount; ++i)
                {
                    faceBuffer[i].indices[0] = indices[i * 3 + 0];
                    faceBuffer[i].indices[1] = indices[i * 3 + 1];
                    faceBuffer[i].indices[2] = indices[i * 3 + 2];
                    faceBuffer[i].discard = false;
                }
            }

            void allocVertexVaringMemory(size_t varyCount = 0)
            {
                varyingsCount = varyCount;
                varyingsAlignedSize = calculateVaryingsAlignedSize(varyCount);
                varyingsBuffer = nullptr;

                if (varyingsAlignedSize > 0)
                {
                    varyingsBuffer = std::shared_ptr<float>((float*)Memory::alignedMalloc(varyingsAlignedSize * vertexBuffer.size()), [](const float* ptr) { Memory::alignedFree((void*)ptr); });

                    for (int32_t i = 0; i < vertexBuffer.size(); i++)
                    {
                        vertexBuffer[i].varyings = varyingsBuffer.get() + i * varyingsAlignedSize / sizeof(float);
                    }
                }
            }


            VertexBuffer vertexBuffer;
            FaceBuffer faceBuffer;

            size_t varyingsCount = 0;
            size_t varyingsAlignedSize = 0;
            std::shared_ptr<float> varyingsBuffer = nullptr;
        };

    public:
        void init(int width, int height);

        void clear(float r, float g, float b, float a);

        void drawTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2);

        void drawMesh(const Mesh* mesh);

        void drawMesh1(const Mesh* mesh);

        void clearBuffer(const glm::vec4& color);

        void swapBuffer();

        FrameBuffer::Ptr& getOutput();

        void setModelMatrix(const glm::mat4& mat);

        void setViewMatrix(const glm::mat4& mat);

        void setProjMatrix(const glm::mat4& mat);

        void drawLine(const glm::vec2& v0, const glm::vec2& v1);

        void useProgram(std::shared_ptr<Program> program);

    private:
        void uploadVertexData(const std::vector<Vertex>& vertices, const std::vector<int>& indices);

        void processVertexShader();

        void processFrustumClip();

        void processPerspectiveDivide();

        void processViewportTransform();

        void processBackFaceCulling();

        void processRasterization();

    private:
        glm::vec3 perspectiveDivide(const glm::vec4& pos);

        void perspectiveDivide1(glm::vec4& pos);

        glm::vec3 viewportTransform(const glm::vec3& ndcCoord);

        void viewportTransform1(glm::vec4& pos);

        void PerspectiveCorrectInterpolation(FragmentQuad& quad);

        void VaryingInterpolate(float* out_vary,
            const float* in_varyings[],
            size_t elem_cnt,
            glm::aligned_vec4& bc);

        float interpolateDepth(const std::array<float, 3>& screenDepth, const glm::vec3& weight);

        bool edgeFunction(const glm::ivec2& a, const glm::ivec2& b, const glm::ivec2& c);

        void rasterizeTriangle(const Shader::VertexData& vertex0, const Shader::VertexData& vertex1, const Shader::VertexData& vertex2);

        void rasterizeTriangle2(const Shader::VertexData& vertex0, const Shader::VertexData& vertex1, const Shader::VertexData& vertex2);

        void rasterizeTriangle3(const Shader::VertexData& vertex0, const Shader::VertexData& vertex1, const Shader::VertexData& vertex2);

        void rasterizeTriangle4(FaceResource& face);

        void rasterizeTopTriangle(const Shader::VertexData& v0, const Shader::VertexData& v1, const Shader::VertexData& v2);

        void rasterizeBottomTriangle(const Shader::VertexData& v0, const Shader::VertexData& v1, const Shader::VertexData& v2);

        void scanLine(const Shader::VertexData& left, const Shader::VertexData& right);


    private:
        Shader::Ptr mShader;

        FrameBuffer::Ptr mBackBuffer;
        FrameBuffer::Ptr mFrontBuffer;

        int mWidth;
        int mHeight;

        Viewport mViewport;
        DepthRange mDepthRange;

        bool mEnableBackfaceCull = true;

        RenderContex mRenderContex;

        std::shared_ptr<Program> mProgram = nullptr;

        BS::thread_pool_light thread_pool_;
    };
}