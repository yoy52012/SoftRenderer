#include "Graphics.h"

#include <iostream>
#include <algorithm>
#include <array>
#include "Utils.h"

namespace SoftRenderer
{

    void Graphics::init(int width, int height)
    {
        mWidth = width;
        mHeight = height;

        // create back buffer
        mFrontBuffer = std::make_shared<FrameBuffer>(width, height);
        mBackBuffer = std::make_shared<FrameBuffer>(width, height);

        // set viewport
        mViewport.x = 0;
        mViewport.y = 0;
        mViewport.width  = (float)width;
        mViewport.height = (float)height;

        // set depth range
        mDepthRange.n = 0.1f;
        mDepthRange.f = 100.f;





        //Texture2D::Ptr albedo = std::make_shared<Texture2D>();
        //albedo->initFromImage(image);
        //mShader->mUniforms.albedo = albedo;
    }

    void Graphics::clear(float r, float g, float b, float a)
    {
        mFrontBuffer->clearColor(r, g, b, a);
        mBackBuffer->clearColor(r, g, b, a);
    }

    void Graphics::drawTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2)
    {
        Shader::VertexData vd0, vd1, vd2;
        vd0 = mShader->vertexShader(v0);
        vd1 = mShader->vertexShader(v1);
        vd2 = mShader->vertexShader(v2);

        glm::vec3 ndcPosition0 = perspectiveDivide(vd0.clipPostion);
        glm::vec3 ndcPosition1 = perspectiveDivide(vd1.clipPostion);
        glm::vec3 ndcPosition2 = perspectiveDivide(vd2.clipPostion);

        vd0.clipW = 1.0f / vd0.clipPostion.w;
        vd1.clipW = 1.0f / vd1.clipPostion.w;
        vd2.clipW = 1.0f / vd2.clipPostion.w;

        vd0.texcoord *= vd0.clipW;
        vd1.texcoord *= vd1.clipW;
        vd2.texcoord *= vd2.clipW;

        glm::vec3 tmpPos0 = viewportTransform(ndcPosition0);
        glm::vec3 tmpPos1 = viewportTransform(ndcPosition1);
        glm::vec3 tmpPos2 = viewportTransform(ndcPosition2);

        vd0.screenPosition = glm::vec2(tmpPos0.x, tmpPos0.y);
        vd1.screenPosition = glm::vec2(tmpPos1.x, tmpPos1.y);
        vd2.screenPosition = glm::vec2(tmpPos2.x, tmpPos2.y);

        rasterizeTriangle3(vd0, vd1, vd2);
    }

    void Graphics::drawMesh(const Mesh* mesh)
    {
        for (const auto subMesh : mesh->subMeshs)
        {
            if (subMesh->indices.empty() || subMesh->vertices.empty())
                return;

            for (int i = 0; i < subMesh->indices.size(); i += 3)
            {
                Vertex v0, v1, v2;
                v0 = subMesh->vertices[subMesh->indices[i]];
                v1 = subMesh->vertices[subMesh->indices[i + 1]];
                v2 = subMesh->vertices[subMesh->indices[i + 2]];

                drawTriangle(v0, v1, v2);
            }
        }
    }

    void Graphics::drawMesh1(const Mesh* mesh)
    {
        for (const auto subMesh : mesh->subMeshs)
        {
            if (subMesh->indices.empty() || subMesh->vertices.empty())
                return;

            uploadVertexData(subMesh->vertices, subMesh->indices);
            processVertexShader();
            processFrustumClip();
            processPerspectiveDivide();
            processViewportTransform();
            processBackFaceCulling();
            processRasterization();
        }
    }

    void Graphics::clearColor(const glm::vec4& color)
    {
        mBackBuffer->clearColor(color);
    }

    void Graphics::clearDepth(float depth)
    {
        mBackBuffer->clearDepth(depth);
    }

    void Graphics::swapBuffer()
    {
        auto tmp = std::move(mFrontBuffer);
        mFrontBuffer = std::move(mBackBuffer);
        mBackBuffer = std::move(tmp);
    }

    FrameBuffer::Ptr& Graphics::getOutput()
    {
        return mFrontBuffer;
    }

    void Graphics::setModelMatrix(const glm::mat4& mat)
    {
        mShader->setModelMatrix(mat);
    }

    void Graphics::setViewMatrix(const glm::mat4& mat)
    {
        mShader->setViewMatrix(mat);
    }

    void Graphics::setProjMatrix(const glm::mat4& mat)
    {
        mShader->setProjMatrix(mat);
    }

    bool Graphics::edgeFunction(const glm::ivec2& a, const glm::ivec2& b, const glm::ivec2& c)
    {
        return ((c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x) >= 0 ) ;	
    }

    glm::vec3 Graphics::perspectiveDivide(const glm::vec4& pos)
    {
        float invW = 1.0f / pos.w;
        float x = pos.x * invW;
        float y = pos.y * invW;
        float z = pos.z * invW;

        return glm::vec3(x, y, z);
    }

    void Graphics::perspectiveDivide1(glm::vec4& pos)
    {
        float invW = 1.0f / pos.w;
        pos.x *= invW;
        pos.y *= invW;
        pos.z *= invW;
    }

    /*
     * for viewport transformation, see subsection 2.12.1 of
     * https://www.khronos.org/registry/OpenGL/specs/es/2.0/es_full_spec_2.0.pdf
     */
    glm::vec3 Graphics::viewportTransform(const glm::vec3& ndcCoord)
    {
        float x = (ndcCoord.x + 1.0f) * 0.5f * static_cast<float>(mWidth);
        float y = (ndcCoord.y + 1.0f) * 0.5f * static_cast<float>(mHeight);
        float z = (ndcCoord.z + 1.0f) * 0.5f;
        return glm::vec3(x, y, z);
    }

    void Graphics::viewportTransform1(glm::vec4& pos)
    {
        pos.x = (pos.x + 1.0f) * 0.5f * mViewport.width  + mViewport.x;
        pos.y = (pos.y + 1.0f) * 0.5f * mViewport.height + mViewport.y;

        // Reversed-Z  [-1, 1] -> [far, near]
        // -(f-n)/2 * z + (f+n)/2
        auto sum = mDepthRange.f + mDepthRange.n;
        auto diff = mDepthRange.f - mDepthRange.n;
        pos.z = 0.5f * (sum - diff * pos.z);
    }

    void Graphics::perspectiveCorrectInterpolation(FragmentQuad& quad)
    {
        glm::aligned_vec4* vert = quad.triangularVertexScreenPositionFlat;

        for (auto& pixel : quad.pixels) 
        {
            auto& bc = pixel.barycentric;

            // barycentric correction
            bc /= quad.triangularVertexClipZ;
            bc /= (bc.x + bc.y + bc.z);

            // interpolate z, w
            pixel.position.z = glm::dot(vert[2], bc);
            pixel.position.w = glm::dot(vert[3], bc);
        }
    }

    void Graphics::varyingInterpolate(float* out_vary, const float* in_varyings[], size_t elem_cnt, glm::aligned_vec4& bc) 
    {
        const float* in_vary0 = in_varyings[0];
        const float* in_vary1 = in_varyings[1];
        const float* in_vary2 = in_varyings[2];

        for (int i = 0; i < elem_cnt; i++) 
        {
            auto a = glm::vec4(*(in_vary0 + i), *(in_vary1 + i), *(in_vary2 + i), 1.0f);
            out_vary[i] = glm::dot(glm::vec4(bc.x, bc.y, bc.z, bc.w), a);
        }
    }

    bool Graphics::depthTest(uint32_t x, uint32_t y, float depth)
    {
        // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDepthFunc.xhtml
        // Even if the depth buffer exists and the depth mask is non-zero, the depth buffer is not updated if the depth test is disabled. 
        // In order to unconditionally write to the depth buffer, the depth test should be enabled and set to GL_ALWAYS.
        if(!mEnableDepthTest)
            return true;

        float z = mBackBuffer->getDepth(x, y);
        if (depthFuncTest(depth, z, mDepthFunc))
        {
            if (mEnableDepthMask)
            {
                mBackBuffer->writeDepth(x, y, depth);
            }
            return true;
        }

        return false;
    }

    bool Graphics::depthFuncTest(float depth, float z, DepthFunc func)
    {
        switch (func)
        {
        case DepthFunc::DEPTH_NEVER:return false;
        case DepthFunc::DEPTH_LESS:return depth < z;
        case DepthFunc::DEPTH_EQUAL:return std::fabs(depth - z) <= std::numeric_limits<float>::epsilon();
        case DepthFunc::DEPTH_LEQUAL:return depth <= z;
        case DepthFunc::DEPTH_GREATER:return depth > z;
        case DepthFunc::DEPTH_NOTEQUAL:return std::fabs(depth - z) > std::numeric_limits<float>::epsilon();
        case DepthFunc::DEPTH_GEQUAL:return depth >= z;
        case DepthFunc::DEPTH_ALWAYS:return true;
        }
        return depth < z;
    }

    float Graphics::interpolateDepth(const std::array<float, 3>& screenDepth, const glm::vec3& weight)
    {
        float d0 = screenDepth[0] * weight.x;
        float d1 = screenDepth[1] * weight.y;
        float d2 = screenDepth[2] * weight.z;

        return d0 + d1 + d2;
    }

    glm::vec2 interpolateTexcoord(const std::array<glm::vec2, 3>& texcoord, const glm::vec3& weight, const std::array<float, 3>& recipW)
    {
        float weight0 = recipW[0] * weight.x;
        float weight1 = recipW[1] * weight.y;
        float weight2 = recipW[2] * weight.z;
        float normlWeight = 1.0f / (weight0 + weight1 + weight2);

        float x = (texcoord[0].x * weight0 + texcoord[1].x * weight1 + texcoord[2].x * weight2) * normlWeight;
        float y = (texcoord[0].y * weight0 + texcoord[1].y * weight1 + texcoord[2].y * weight2) * normlWeight;

        return { x, y };
    }

    void Graphics::scanLine(const Shader::VertexData& left, const Shader::VertexData& right)
    {
        int length = right.screenPosition.x - left.screenPosition.x + 1;
        for (int i = 0; i <= length; i++)
        {
            Shader::VertexData v = Shader::VertexData::lerp(left, right, static_cast<float>(i / length));
            v.screenPosition.x = left.screenPosition.x + i;
            v.screenPosition.y = left.screenPosition.y;
            mBackBuffer->writeColor(v.screenPosition.x, v.screenPosition.y, glm::vec4(1.0, 1.0, 1.0, 1.0));
        }
        
    }

    void Graphics::rasterizeTopTriangle(const Shader::VertexData& v0, const Shader::VertexData& v1, const Shader::VertexData& v2)
    {
        Shader::VertexData top, left, right;
        Shader::VertexData newLeft, newRight;
        left  = v1.screenPosition.x > v2.screenPosition.x ? v2 : v1;
        right = v1.screenPosition.x > v2.screenPosition.x ? v1 : v2;
        top   = v0;

        int dy = top.screenPosition.y - left.screenPosition.y + 1;
        int nowY = top.screenPosition.y;

        for (int i = dy; i >= 0; --i)
        {
            float weight = 0;
            if(dy != 0)
                weight =  (float) i/ dy;
            newLeft  = Shader::VertexData::lerp(left, top, weight);
            newRight = Shader::VertexData::lerp(right, top, weight);
            newLeft.screenPosition.y = newRight.screenPosition.y = nowY;
            scanLine(newLeft, newRight);
            nowY--;
        }
    }

    void Graphics::rasterizeBottomTriangle(const Shader::VertexData& v0, const Shader::VertexData& v1, const Shader::VertexData& v2)
    {
        Shader::VertexData left, right, bottom;
        Shader::VertexData newLeft, newRight;
        left  = v1.screenPosition.x > v2.screenPosition.x ? v2 : v1;
        right = v1.screenPosition.x > v2.screenPosition.x ? v1 : v2;
        bottom = v0;

        int dy = left.screenPosition.y - bottom.screenPosition.y + 1;
        int nowY = left.screenPosition.y;

        for (int i = 0; i < dy; ++i)
        {
            float weight = 0;
            if(dy != 0)
                weight = (float) i / dy;
            newLeft  = Shader::VertexData::lerp(left, bottom, weight);
            newRight = Shader::VertexData::lerp(right, bottom, weight);
            newLeft.screenPosition.y = newRight.screenPosition.y = nowY;
            scanLine(newLeft, newRight);
            nowY--;
        }
    } 

    // Test whether two float or double numbers are equal.
    // ulp: units in the last place.
    template <typename T>
    typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
    isEqual(T x, T y, int ulp = 2) {
    // the machine epsilon has to be scaled to the magnitude of the values used
    // and multiplied by the desired precision in ULPs (units in the last place)
    return std::fabs(x - y) <
                std::numeric_limits<T>::epsilon() * std::fabs(x + y) * ulp
            // unless the result is subnormal
            || std::fabs(x - y) < std::numeric_limits<T>::min();
    }


    void Graphics::rasterizeTriangle2(const Shader::VertexData& vertex0, const Shader::VertexData& vertex1, const Shader::VertexData& vertex2)
    {
        Shader::VertexData verts[3] = {vertex0, vertex1, vertex2};
        Shader::VertexData tmp;

        // sort verts's in axisY v0 < v1 < v2
        if(verts[0].screenPosition.y > verts[1].screenPosition.y)
        {
            tmp= verts[0];
            verts[0] = verts[1];
            verts[1] = tmp; 
        }
        if(verts[1].screenPosition.y > verts[2].screenPosition.y)
        {
            tmp = verts[1];
            verts[1] = verts[2];
            verts[2] = tmp;
        }
        if(verts[0].screenPosition.y > verts[1].screenPosition.y)
        {
            tmp = verts[0];
            verts[0] = verts[1];
            verts[1] = tmp;
        }

        if(isEqual(verts[1].screenPosition.y, verts[2].screenPosition.y))
        {
            rasterizeBottomTriangle(verts[0], verts[1], verts[2]);
        }
        else if(isEqual(verts[1].screenPosition.y, verts[0].screenPosition.y))
        {
            rasterizeTopTriangle(verts[2], verts[1], verts[0]);
        }
        else
        {
            float weight = (verts[2].screenPosition.y - verts[1].screenPosition.y) / (verts[2].screenPosition.y - verts[0].screenPosition.y);
            Shader::VertexData newVert = Shader::VertexData::lerp(verts[2], verts[0], weight);
            rasterizeTopTriangle(verts[2], verts[1], newVert);
            rasterizeBottomTriangle(verts[0], verts[1], newVert);
        }
    }

    void Graphics::rasterizeTriangle3(const Shader::VertexData& v0, const Shader::VertexData& v1, const Shader::VertexData& v2)
    {
            const glm::ivec2& A = glm::ivec2((int)(v0.screenPosition.x + 0.5f), (int)(v0.screenPosition.y + 0.5f));
            const glm::ivec2& B = glm::ivec2((int)(v1.screenPosition.x + 0.5f), (int)(v1.screenPosition.y + 0.5f));
            const glm::ivec2& C = glm::ivec2((int)(v2.screenPosition.x + 0.5f), (int)(v2.screenPosition.y + 0.5f));

            //float Z[3] = { v0.screenPosition.z, v1.screenPosition.z, v2.screenPosition.z };

            int minX = std::max(std::min(A.x, std::min(B.x, C.x)), 0);
            int minY = std::max(std::min(A.y, std::min(B.y, C.y)), 0);
            int maxX = std::min(std::max(A.x, std::max(B.x, C.x)), mWidth-1);
            int maxY = std::min(std::max(A.y, std::max(B.y, C.y)), mHeight-1);

            //I1 = Ay - By, I2 = By - Cy, I3 = Cy - Ay
            int I01 = A.y - B.y;
            int I02 = B.y - C.y;
            int I03 = C.y - A.y;
    
            //J1 = Bx - Ax, J2 = Cx - Bx, J3 = Ax - Cx 
            int J01 = B.x - A.x; 
            int J02 = C.x - B.x; 
            int J03 = A.x - C.x; 

            //K1 = AxBy - AyBx, K2 = BxCy - ByCx, K3 = CxAy - CyAx
            int K01 = A.x * B.y - A.y * B.x;
            int K02 = B.x * C.y - B.y * C.x;
            int K03 = C.x * A.y - C.y * A.x;

            //F1 = I1 * Px + J1 * Py + K1
            //F2 = I2 * Px + J2 * Py + K2
            //F3 = I3 * Px + J3 * Py + k3
            int F01 = (I01 * minX) + (J01 * minY) + K01;
            int F02 = (I02 * minX) + (J02 * minY) + K02;
            int F03 = (I03 * minX) + (J03 * minY) + K03;

            // Area = 1/2 * |AB| * |AC| * sin(|AB|, |AC|) = 1/2 * (AxBy - AyBx + BxCy - ByCx + CxAy - CyAx) = F1 + F2 + F3
            int delta = F01 + F02 + F03;

            //Degenerated to a line or a point
            if (delta <= 0)
                return;
           
            float oneDivideDelta = 1 / (float)delta;

            //Z[1] = (Z[1] - Z[0]) * OneDivideDelta;
            //Z[2] = (Z[2] - Z[0]) * OneDivideDelta;


            int Cy1 = F01, Cy2 = F02, Cy3 = F03;

            for(int y = minY; y < maxY; ++y)
            {
                int Cx1 = Cy1;
                int Cx2 = Cy2;
                int Cx3 = Cy3;
                
                //float depth = Z[0] + Cx3 * Z[1] + Cx1 * Z[2];

                for(int x = minX; x < maxX; ++x)
                {
                    const int mask = Cx1 | Cx2 | Cx3;
                    if(mask >= 0)
                    {
                        glm::vec3 weights = glm::vec3(Cx2 * oneDivideDelta, Cx3 * oneDivideDelta, Cx1 * oneDivideDelta);

                        auto v = Shader::VertexData::barycentricLerp(v0, v1, v2, weights);

                        std::array<glm::vec2, 3> texcoords = { v0.texcoord, v1.texcoord, v2.texcoord };
                        std::array<float, 3> reclipW = { v0.clipW, v1.clipW, v2.clipW };

                        v.texcoord = interpolateTexcoord(texcoords, weights, reclipW);
                        v.texcoord /= v.clipW;

                        //mBackBuffer->writeColor(x, y, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
                        mBackBuffer->writeColor(x, y, mShader->fragmentShader(v));

                        //mBackBuffer->WriteDepth(x, y, depth);
                    }

                    Cx1 += I01;
                    Cx2 += I02;
                    Cx3 += I03;
                }

                Cy1 += J01;
                Cy2 += J02;
                Cy3 += J03;
            }
    }

    void Graphics::uploadVertexData(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
    {
        mRenderContex.createVertexBuffer(vertices);
        mRenderContex.createIndexBuffer(indices);
        mRenderContex.allocVertexVaringMemory(mProgram->varyingsCount);
    }

    void Graphics::processVertexShader()
    {
        auto vertexShader = mProgram->vertexShader;

        for (auto& vertex : mRenderContex.vertexBuffer) 
        {
            vertexShader->attributes = (BaseShaderAttributes*)&vertex.vertex;

            if (vertex.varyings == nullptr)
            {
                vertexShader->varyings = nullptr;
            }
            else 
            {
                vertexShader->varyings = (BaseShaderVaryings*)vertex.varyings;
            }

            vertexShader->shaderMain();

            vertex.position = vertexShader->gl_Position;
        }

    }

    void Graphics::processFrustumClip()
    {
    }

    void Graphics::processPerspectiveDivide()
    {
        for (auto& vertex : mRenderContex.vertexBuffer)
        {
            perspectiveDivide1(vertex.position);
        }
    }

    void Graphics::processViewportTransform()
    {
        for (auto& vertex : mRenderContex.vertexBuffer)
        {
            viewportTransform1(vertex.position);
        }
    }

    void Graphics::processBackFaceCulling()
    {
        for (auto& face : mRenderContex.faceBuffer)
        {
            if (face.discard)
            {
                continue;
            }

            glm::vec4 v0 = mRenderContex.vertexBuffer[face.indices[0]].position;
            glm::vec4 v1 = mRenderContex.vertexBuffer[face.indices[1]].position;
            glm::vec4 v2 = mRenderContex.vertexBuffer[face.indices[2]].position;

            glm::vec3 n = glm::cross(glm::vec3(v1 - v0), glm::vec3(v2- v0));
            float d = glm::dot(n, glm::vec3(0, 0, 1));

            face.frontFacing = d > 0;


            if (mEnableBackfaceCull)
            {
                face.discard = !face.frontFacing;  // discard back face
            }
        }
    }

    void Graphics::processRasterization()
    {
        for (auto& face : mRenderContex.faceBuffer) 
        {
            if (!face.discard) 
            {
                rasterizeTriangle4(face);
            }
        }
    }


    void Graphics::rasterizeTriangle(const Shader::VertexData& vertex0, const Shader::VertexData& vertex1, const Shader::VertexData& vertex2)
    {
        auto v0 = glm::ivec2(vertex0.screenPosition.x, vertex0.screenPosition.y);
        auto v1 = glm::ivec2(vertex1.screenPosition.x, vertex1.screenPosition.y);
        auto v2 = glm::ivec2(vertex2.screenPosition.x, vertex2.screenPosition.y);

        int min_x = std::max(std::min(v0.x, std::min(v1.x, v2.x)), 0);
        int min_y = std::max(std::min(v0.y, std::min(v1.y, v2.y)), 0);
        int max_x = std::min(std::max(v0.x, std::max(v1.x, v2.x)), mWidth-1);
        int max_y = std::min(std::max(v0.y, std::max(v1.y, v2.y)), mHeight-1);

        //Strict barycenteric weights calculation
        auto barycentericWeight = [&](const float& x, const float& y) -> glm::vec3
        {
            glm::vec3 s[2];
            s[0] = glm::vec3(vertex2.screenPosition.x - vertex0.screenPosition.x, vertex1.screenPosition.x - vertex0.screenPosition.x, vertex0.screenPosition.x - x);
            s[1] = glm::vec3(vertex2.screenPosition.y - vertex0.screenPosition.y, vertex1.screenPosition.y - vertex0.screenPosition.y, vertex0.screenPosition.y - y);
            auto uf = glm::cross(s[0], s[1]);
            return glm::vec3(1.f - (uf.x + uf.y) / uf.z, uf.y / uf.z, uf.x / uf.z);
        };

        auto barycenterWeight1 = [&](const float& x, const float& y) -> glm::vec3
        {
            float x0 = vertex0.screenPosition.x;
            float x1 = vertex1.screenPosition.x;
            float x2 = vertex2.screenPosition.x;
            float y0 = vertex0.screenPosition.y;
            float y1 = vertex1.screenPosition.y;
            float y2 = vertex2.screenPosition.y;

            
            float a = (-(x - x1) * (y2 - y1) + (y - y1) * (x2 - x1)) / (-(x0 - x1) * (y2 - y1) + (y0 - y1) * (x2 - x1));
            float b = (-(x - x2) * (y0 - y2) + (y - y2) * (x0 - x2)) / (-(x1 - x2) * (y0 - y2) + (y1 - y2) * (x0 - x2));
            float c = 1 - a - b;

            return glm::vec3(a, b, c);
        };

        for (int x = min_x; x <= max_x; ++x)
        {
            for (int y = min_y; y <= max_y; ++y)
            {
                glm::vec2 v = { x, y };
                bool insied = true;
                insied &= edgeFunction(v0, v1, v);
                insied &= edgeFunction(v1, v2, v);
                insied &= edgeFunction(v2, v0, v);
                if (insied)
                {
                    auto weight = barycentericWeight(x, y);
                    auto weight1 = barycenterWeight1(x, y);
                    auto v = Shader::VertexData::barycentricLerp(vertex0, vertex1, vertex2, weight);

                    std::array<glm::vec2, 3> texcoords = {vertex0.texcoord, vertex1.texcoord, vertex2.texcoord};
                    std::array<float, 3> reclipW = { vertex0.clipW, vertex1.clipW, vertex2.clipW };

                    //v.texcoord = interpolateTexcoord(texcoords, weight, reclipW);
                    v.texcoord /= v.clipW;

                    //mBackBuffer->writeColor(x, y, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
                    mBackBuffer->writeColor(x, y, mShader->fragmentShader(v));
                }
            }
        }
    }

    bool isSamplingInside(int32_t x, int32_t y, int32_t Cx1, int32_t Cx2, int32_t Cx3, int32_t boundingMaxX, int32_t boundingMaxY)
    {
        //Invalid fragment
        if (x > boundingMaxX || y > boundingMaxY)
        {
            return false;
        }

        bool atLeastOneInside = false;

        {
            //Edge function
            const float E1 = Cx1;
            const float E2 = Cx2;
            const float E3 = Cx3;

            //Note: Counter-clockwise winding order
            if (E1  >= 0 && E2 >= 0 && E3 >= 0)
            {
                atLeastOneInside = true;
                //Note: each sampling point should have its own depth
                //glm::vec3 uvw = glm::vec3(E2, E3, E1) * one_div_delta;
            }
        }

        return atLeastOneInside;
    }

    bool Barycentric(glm::aligned_vec4* vert, glm::aligned_vec4& v0, glm::aligned_vec4& p, glm::aligned_vec4& bc)
    {
        glm::vec3 u = glm::cross(glm::vec3(vert[0]) - glm::vec3(v0.x, v0.x, p.x + 0.5f), glm::vec3(vert[1]) - glm::vec3(v0.y, v0.y, p.y + 0.5f));
        if (std::abs(u.z) < 1.192092896e-07F) {
            return false;
        }

        u /= u.z;
        bc = { 1.f - (u.x + u.y), u.y, u.x, 0.f };

        if (bc.x < 0 || bc.y < 0 || bc.z < 0) {
            return false;
        }

        return true;
    }

    void Graphics::rasterizeTriangle4(FaceResource& face)
    {
        glm::aligned_vec4 screenPosition[3];
        for (int32_t i = 0; i < 3; i++) 
        {
            auto& vertex = mRenderContex.vertexBuffer[face.indices[i]];
            screenPosition[i] = vertex.position;
        }

        const glm::ivec2& A = glm::ivec2((int32_t)(screenPosition[0].x), (int32_t)(screenPosition[0].y));
        const glm::ivec2& B = glm::ivec2((int32_t)(screenPosition[1].x), (int32_t)(screenPosition[1].y));
        const glm::ivec2& C = glm::ivec2((int32_t)(screenPosition[2].x), (int32_t)(screenPosition[2].y));

        int32_t minX = std::max(std::min(A.x, std::min(B.x, C.x)), 0);
        int32_t minY = std::max(std::min(A.y, std::min(B.y, C.y)), 0);
        int32_t maxX = std::min(std::max(A.x, std::max(B.x, C.x)), mWidth - 1);
        int32_t maxY = std::min(std::max(A.y, std::max(B.y, C.y)), mHeight - 1);

        //I1 = Ay - By, I2 = By - Cy, I3 = Cy - Ay
        int32_t I01 = A.y - B.y;
        int32_t I02 = B.y - C.y;
        int32_t I03 = C.y - A.y;

        //J1 = Bx - Ax, J2 = Cx - Bx, J3 = Ax - Cx 
        int32_t J01 = B.x - A.x;
        int32_t J02 = C.x - B.x;
        int32_t J03 = A.x - C.x;

        //K1 = AxBy - AyBx, K2 = BxCy - ByCx, K3 = CxAy - CyAx
        int32_t K01 = A.x * B.y - A.y * B.x;
        int32_t K02 = B.x * C.y - B.y * C.x;
        int32_t K03 = C.x * A.y - C.y * A.x;

        //F1 = I1 * Px + J1 * Py + K1
        //F2 = I2 * Px + J2 * Py + K2
        //F3 = I3 * Px + J3 * Py + k3
        int32_t F01 = (I01 * minX) + (J01 * minY) + K01;
        int32_t F02 = (I02 * minX) + (J02 * minY) + K02;
        int32_t F03 = (I03 * minX) + (J03 * minY) + K03;

        // Area = 1/2 * |AB| * |AC| * sin(|AB|, |AC|) = 1/2 * (AxBy - AyBx + BxCy - ByCx + CxAy - CyAx) = F1 + F2 + F3
        int32_t delta = F01 + F02 + F03;

        //Degenerated to a line or a point
        if (delta == 0)
            return;

        const float oneDivideDelta = 1 / (float)delta;

        int32_t Cy1 = F01, Cy2 = F02, Cy3 = F03;

        auto block_size = (float)32;
        int32_t blockCountX = (int32_t)((maxX - minX + block_size - 1.0f) / block_size);
        int32_t blockCountY = (int32_t)((maxY - minY + block_size - 1.0f) / block_size);    

        for (int32_t blockY = 0; blockY < blockCountY; blockY++)
        {
            for (int32_t blockX = 0; blockX < blockCountX; blockX++)
            {
                mThreadPool.push_task([&, block_size, blockX, blockY]
                {
                    FragmentQuad fragementQuad(mRenderContex.varyingsAlignedSize);

                    fragementQuad.front_facing = face.frontFacing;

                    for (int32_t i = 0; i < 3; i++)
                    {
                        auto& vertex = mRenderContex.vertexBuffer[face.indices[i]];
                        fragementQuad.triangularVertexScreenPosition[i] = vertex.position;
                        fragementQuad.triangularVertexVarings[i] = vertex.varyings;
                        fragementQuad.triangularVertexClipZ[i] = (mDepthRange.f + mDepthRange.n - vertex.position.z) * vertex.position.w;  // [far, near] -> [near, far]
                    }

                    const glm::aligned_vec4* triangularVertexScreenPosition = fragementQuad.triangularVertexScreenPosition;
                    fragementQuad.triangularVertexScreenPositionFlat[0] = { triangularVertexScreenPosition[2].x, triangularVertexScreenPosition[1].x, triangularVertexScreenPosition[0].x, 0.f };
                    fragementQuad.triangularVertexScreenPositionFlat[1] = { triangularVertexScreenPosition[2].y, triangularVertexScreenPosition[1].y, triangularVertexScreenPosition[0].y, 0.f };
                    fragementQuad.triangularVertexScreenPositionFlat[2] = { triangularVertexScreenPosition[0].z, triangularVertexScreenPosition[1].z, triangularVertexScreenPosition[2].z, 0.f };
                    fragementQuad.triangularVertexScreenPositionFlat[3] = { triangularVertexScreenPosition[0].w, triangularVertexScreenPosition[1].w, triangularVertexScreenPosition[2].w, 0.f };

                    int32_t Dy1 = Cy1 + block_size * blockY * J01;
                    int32_t Dy2 = Cy2 + block_size * blockY * J02;
                    int32_t Dy3 = Cy3 + block_size * blockY * J03;

                    // block rasterization
                    int32_t blockStartX = (int32_t)(minX + blockX * block_size);
                    int32_t blockStartY = (int32_t)(minY + blockY * block_size);

                    for (int32_t y = blockStartY; y < blockStartY + block_size && y <= maxY; y += 2)
                    {
                        
                        int32_t Dx1 = Dy1 + block_size * blockX * I01;
                        int32_t Dx2 = Dy2 + block_size * blockX * I02;
                        int32_t Dx3 = Dy3 + block_size * blockX * I03;

                        for (int32_t x = blockStartX; x < blockStartX + block_size && x <= maxX; x += 2)
                        {
                            fragementQuad.init(x, y);

                            fragementQuad.frag_shader.uniforms = mProgram->fragmentShader->uniforms;

                            bool inside0 = isSamplingInside(x, y, Dx1, Dx2, Dx3, maxX, maxY);
                            bool inside1 = isSamplingInside(x + 1, y, Dx1 + I01, Dx2 + I02, Dx3 + I03, maxX, maxY);
                            bool inside2 = isSamplingInside(x, y + 1, Dx1 + J01, Dx2 + J02, Dx3 + J03, maxX, maxY);
                            bool inside3 = isSamplingInside(x + 1, y + 1, Dx1 + J01 + I01, Dx2 + J02 + I02, Dx3 + J03 + I03, maxX, maxY);

                            if (inside0 || inside1 || inside2 || inside3)
                            {

                                if (inside0)
                                {
                                    glm::vec3 uvw(Dx2, Dx3, Dx1);
                                    glm::vec3 weights = uvw * oneDivideDelta;
                                    fragementQuad.pixels[0].barycentric = glm::aligned_vec4(weights, 0.0f);
                                    //glm::aligned_vec4* vert = fragementQuad.vert_flat;
                                    //glm::aligned_vec4& v0 = fragementQuad.screen_pos[0];
                                    //Barycentric(vert, v0, fragementQuad.pixels[0].position, fragementQuad.pixels[0].barycentric);
                                    fragementQuad.pixels[0].inside = true;
                                }
                                else
                                {
                                    glm::aligned_vec4* vert = fragementQuad.triangularVertexScreenPositionFlat;
                                    glm::aligned_vec4& v0 = fragementQuad.triangularVertexScreenPosition[0];
                                    Barycentric(vert, v0, fragementQuad.pixels[0].position, fragementQuad.pixels[0].barycentric);
                                    fragementQuad.pixels[0].inside = false;
                                }

                                if (inside1)
                                {
                                    glm::vec3 uvw(Dx2 + I02, Dx3 + I03, Dx1 + I01);
                                    glm::vec3 weights = uvw * oneDivideDelta;
                                    fragementQuad.pixels[1].barycentric = glm::aligned_vec4(weights, 0.0f);
                                    //glm::aligned_vec4* vert = fragementQuad.vert_flat;
                                    //glm::aligned_vec4& v0 = fragementQuad.screen_pos[0];
                                    //Barycentric(vert, v0, fragementQuad.pixels[1].position, fragementQuad.pixels[1].barycentric);
                                    fragementQuad.pixels[1].inside = true;
                                }
                                else
                                {
                                    glm::aligned_vec4* vert = fragementQuad.triangularVertexScreenPositionFlat;
                                    glm::aligned_vec4& v0 = fragementQuad.triangularVertexScreenPosition[0];
                                    Barycentric(vert, v0, fragementQuad.pixels[1].position, fragementQuad.pixels[1].barycentric);
                                    fragementQuad.pixels[1].inside = false;
                                }

                                if (inside2)
                                {
                                    glm::vec3 uvw(Dx2 + J02, Dx3 + J03, Dx1 + J01);
                                    glm::vec3 weights = uvw * oneDivideDelta;
                                    fragementQuad.pixels[2].barycentric = glm::aligned_vec4(weights, 0.0f);
                                    //glm::aligned_vec4* vert = fragementQuad.vert_flat;
                                    //glm::aligned_vec4& v0 = fragementQuad.screen_pos[0];
                                    //Barycentric(vert, v0, fragementQuad.pixels[2].position, fragementQuad.pixels[2].barycentric);
                                    fragementQuad.pixels[2].inside = true;
                                }
                                else
                                {
                                    glm::aligned_vec4* vert = fragementQuad.triangularVertexScreenPositionFlat;
                                    glm::aligned_vec4& v0 = fragementQuad.triangularVertexScreenPosition[0];
                                    Barycentric(vert, v0, fragementQuad.pixels[2].position, fragementQuad.pixels[2].barycentric);
                                    fragementQuad.pixels[2].inside = false;
                                }

                                if (inside3)
                                {
                                    glm::vec3 uvw(Dx2 + J02 + I02, Dx3 + J03 + I03, Dx1 + J01 + I01);
                                    glm::vec3 weights = uvw * oneDivideDelta;
                                    fragementQuad.pixels[3].barycentric = glm::aligned_vec4(weights, 0.0f);
                                    //glm::aligned_vec4* vert = fragementQuad.vert_flat;
                                    //glm::aligned_vec4& v0 = fragementQuad.screen_pos[0];
                                    //Barycentric(vert, v0, fragementQuad.pixels[3].position, fragementQuad.pixels[3].barycentric);
                                    fragementQuad.pixels[3].inside = true;
                                }
                                else
                                {
                                    glm::aligned_vec4* vert = fragementQuad.triangularVertexScreenPositionFlat;
                                    glm::aligned_vec4& v0 = fragementQuad.triangularVertexScreenPosition[0];
                                    Barycentric(vert, v0, fragementQuad.pixels[3].position, fragementQuad.pixels[3].barycentric);
                                    fragementQuad.pixels[3].inside = false;
                                }

                                // barycentric correction
                                perspectiveCorrectInterpolation(fragementQuad);

                                // varying interpolate
                                // note: all quad pixels should perform varying interpolate to enable varying partial derivative
                                for (auto& pixel : fragementQuad.pixels)
                                {
                                    varyingInterpolate((float*)pixel.interpolatedVaryings, fragementQuad.triangularVertexVarings, mRenderContex.varyingsCount, pixel.barycentric);
                                }

                                // fragment quad shading
                                pixelShading(fragementQuad);
                            }

                            Dx1 += 2 * I01; Dx2 += 2 * I02; Dx3 += 2 * I03;
                        }
                        Dy1 += 2 * J01;	Dy2 += 2 * J02; Dy3 += 2 * J03;
                    }
                });
            }
        }

        mThreadPool.wait_for_tasks();
    }

    void Graphics::pixelShading(FragmentQuad& fragementQuad)
    {
        for (auto& pixel : fragementQuad.pixels)
        {
            glm::aligned_vec4& pos = pixel.position;
            if (!pixel.inside)
            {
                continue;
            }

            uint32_t x = (uint32_t)pos.x;
            uint32_t y = (uint32_t)pos.y;

            BaseFragmentShader& fragmentShader = fragementQuad.frag_shader;
            fragmentShader.varyings = (BaseShaderVaryings*)pixel.interpolatedVaryings;
            fragmentShader.gl_FragCoord = glm::vec4(pos.x, pos.y, pos.z, 1.0f / pos.w);

            // pixel shading
            fragmentShader.shaderMain();

            // pixel color
            glm::vec4 color = glm::clamp(fragmentShader.gl_FragColor, 0.0f, 1.0f);

            // pixel depth
            float depth = fragmentShader.gl_FragDepth;

            if (depthTest(x, y, depth))
            {
                mBackBuffer->writeColor(x, y, color);
            }

        }
    }

    void Graphics::drawLine(const glm::vec2& v0, const glm::vec2& v1)
    {
        int32_t x0 = (int32_t)v0.x;
        int32_t y0 = (int32_t)v0.y;
        int32_t x1 = (int32_t)v1.x;
        int32_t y1 = (int32_t)v1.y;

        bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);

        //check line's slop
        if (steep)
        {
            std::swap(x0, y0);
            std::swap(x1, y1);
        }

        //check line's direction
        if (x0 > x1)
        {
            std::swap(x0, x1);
            std::swap(y0, y1);
        }

        int32_t dx = x1 - x0;
        int32_t dy = y1 - y0;

        int32_t d2x = dx << 1;
        int32_t d2y = dy << 1; 
        int32_t d2xd2y = d2x - d2y;

        int32_t x = x0;
        int32_t y = y0;

        int32_t p = dx - d2y;
        for (int32_t i = 0; i <= dx; ++i)
        {
            if (steep)
            {
                mBackBuffer->writeColor(y, x, glm::vec4(1.0, 1.0, 1.0, 1.0));
            }
            else {
                mBackBuffer->writeColor(x, y, glm::vec4(1.0, 1.0, 1.0, 1.0));
            }

            if (p <= 0)
            {
                p += d2xd2y;
                y += 1;
            }
            else
            {
                p -= d2y; 
            }
            x += 1;
        }
    }

    void Graphics::useProgram(std::shared_ptr<Program> program)
    {
        mProgram = program;
    }

    void Graphics::setDepthTestEnable(bool enable)
    {
        mEnableDepthTest = enable;
    }

    void Graphics::setDepthWriteMask(bool enable)
    {
        mEnableDepthMask = enable;
    }

    void Graphics::setDepthFunc(DepthFunc func)
    {
        mDepthFunc = func;
    }
}


