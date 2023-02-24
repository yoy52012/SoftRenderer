#include "Texture.h"

namespace SoftRenderer
{
    static int roundupPowerOf2(int n)
    {
        static const int max = 1 << 30;
        n -= 1;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        return n < 0 ? 1 : (n >= max ? max : n + 1);
    }

    void Texture::generateMipmaps()
    {
        if (mMipmapReadyFlag || mMipmapGeneratingFlag)
        {
            return;
        }

        auto generatePo2Mipmap = [](TextureBuffer<glm::vec4>* inBuffer, TextureBuffer<glm::vec4>* outBuffer) -> void
        {
            const float ratioX = (float)inBuffer->getWidth() / (float)outBuffer->getWidth();
            const float ratioY = (float)inBuffer->getHeight() / (float)outBuffer->getHeight();

            for (uint32_t y = 0; y < outBuffer->getHeight(); ++y)
            {
                for (uint32_t x = 0; x < outBuffer->getWidth(); ++x)
                {
                    glm::vec2 uv = glm::vec2((float)x * ratioX, (float)y * ratioY);

                    const int32_t ix = (int32_t)uv.x;
                    const int32_t iy = (int32_t)uv.y;

                    glm::vec2 f = glm::fract(glm::vec2(x, y));

                    auto p0 = BaseSampler::sampleBufferWithWrapMode(inBuffer, ix, iy, WrapMode::WRAP_CLAMP_TO_EDGE);
                    auto p1 = BaseSampler::sampleBufferWithWrapMode(inBuffer, ix + 1, iy, WrapMode::WRAP_CLAMP_TO_EDGE);
                    auto p2 = BaseSampler::sampleBufferWithWrapMode(inBuffer, ix, iy + 1, WrapMode::WRAP_CLAMP_TO_EDGE);
                    auto p3 = BaseSampler::sampleBufferWithWrapMode(inBuffer, ix + 1, iy + 1, WrapMode::WRAP_CLAMP_TO_EDGE);


                    auto pixel = glm::mix(glm::mix(p0, p1, f.x), glm::mix(p2, p3, f.x), f.y);

                    outBuffer->set(x, y, pixel);
                }
            }
        };

        //mMipmapThread = std::make_shared<std::thread>([&]() 

        {
            mMipmapGeneratingFlag = true;

            mMipmaps.clear();

            const uint32_t originWidth = mBuffer->getWidth();
            const uint32_t originHeight = mBuffer->getHeight();

            // alloc memory for mip 0
            uint32_t mipSize = std::max(roundupPowerOf2(originWidth), roundupPowerOf2(originHeight));

            mMipmaps.emplace_back(createTextureBuffer(mipSize, mipSize));

            auto currentBufferPtr = mMipmaps.back().get();
            auto originBufferPtr  = mBuffer.get();

            if (mipSize == originBufferPtr->getWidth() && mipSize == originBufferPtr->getHeight())
            {
                originBufferPtr->copyTo(currentBufferPtr->getData());
            }
            else
            {
                generatePo2Mipmap(originBufferPtr, currentBufferPtr);
            }

            // other levels
            while (mipSize > 1)
            {
                mipSize /= 2;

                mMipmaps.emplace_back(createTextureBuffer(mipSize, mipSize));

                currentBufferPtr = mMipmaps.back().get();

                originBufferPtr = mMipmaps[mMipmaps.size() - 2].get();

                generatePo2Mipmap(originBufferPtr, currentBufferPtr);


                std::vector<uint8_t> out;
                for (size_t y = 0; y < mipSize; y++)
                {
                    for (size_t x = 0; x < mipSize; x++)
                    {
                        glm::vec4* pixel = currentBufferPtr->get(x, y);

                        out.push_back((*pixel).r);
                        out.push_back((*pixel).g);
                        out.push_back((*pixel).b);
                        out.push_back((*pixel).a);
                    }
                }

                auto outImage = Image::create(mipSize, mipSize, Image::PixelFormat::PF_RGBA8888, out);

                outImage->save("E:\\test.png", Image::PNG);

            }

            mMipmapGeneratingFlag = false;
            mMipmapReadyFlag = true;
        }
        //});
    }


    Texture2D::Texture2D(int32_t width, int32_t height)
        :Texture(width, height)
    {
    }

    const glm::vec4 BaseSampler::BORDER_COLOR(0);

    glm::vec4 BaseSampler::sampleBufferWithWrapMode(TextureBuffer<glm::vec4>* buffer, int32_t x, int32_t y, WrapMode wrapMode)
    {
        const int32_t width = buffer->getWidth();
        const int32_t height = buffer->getHeight();

        auto uvMod = [](int32_t i, int32_t n)
        {
            return (i & (n - 1) + n) & (n - 1); // (i % n + n) % n
        };

        auto uvMirror = [](int32_t i)
        {
            return i >= 0 ? i : (-1 - i);
        };

        int32_t sampleX = x;
        int32_t sampleY = y;
        switch (wrapMode)
        {
        case WrapMode::WRAP_REPEAT:
            sampleX = uvMod(x, width);
            sampleY = uvMod(y, height);
            break;
        case WrapMode::WRAP_MIRRORED_REPEAT:
            sampleX = uvMod(x, 2 * width);
            sampleY = uvMod(y, 2 * height);

            sampleX -= width;
            sampleY -= height;

            sampleX = uvMirror(sampleX);
            sampleY = uvMirror(sampleY);

            sampleX = width - 1 - sampleX;
            sampleY = height - 1 - sampleY;
            break;
        case WrapMode::WRAP_CLAMP_TO_EDGE:
            if (x < 0) sampleX = 0;
            if (y < 0) sampleY = 0;
            if (x >= width) sampleX = width - 1;
            if (y >= height) sampleY = height - 1;
            break;
        case WrapMode::WRAP_CLAMP_TO_BORDER:
            if (x < 0 || x >= width) return BORDER_COLOR;
            if (y < 0 || y >= height) return BORDER_COLOR;
            break;
        case WrapMode::WRAP_CLAMP_TO_ZERO:
            if (x < 0 || x >= width) return glm::vec4(0);
            if (y < 0 || y >= height) return glm::vec4(0);
            break;
        default:
            break;
        }

        glm::vec4* pixel = buffer->get(sampleX, sampleY);

        if (pixel)
        {
            return *pixel;
        }

        return glm::vec4(0);
    }

    glm::vec4 BaseSampler::sampleBufferNearest(TextureBuffer<glm::vec4>* buffer, const glm::vec2& uv, WrapMode wrapMode, const glm::ivec2& offset)
    {
        const int32_t x = (int32_t)(uv.x * (buffer->getWidth() - 1) - 0.5f) + offset.x;
        const int32_t y = (int32_t)(uv.y * (buffer->getHeight() - 1) - 0.5f) + offset.y;

        return sampleBufferWithWrapMode(buffer, x, y, wrapMode);
    }

    glm::vec4 BaseSampler::sampleBufferBilinear(TextureBuffer<glm::vec4>* buffer, const glm::vec2& uv, WrapMode wrapMode, const glm::ivec2& offset)
    {
        const float x = (uv.x * buffer->getWidth() - 0.5f) + offset.x;
        const float y = (uv.y * buffer->getHeight() - 0.5f) + offset.y;
        const int32_t ix = (int32_t)x;
        const int32_t iy = (int32_t)y;

        glm::vec2 f = glm::fract(glm::vec2(x, y));

        /*********************
         *   p2--p3
         *   |   |
         *   p0--p1
         * Note: p0 is (ix,iy)
         ********************/

        auto p0 = sampleBufferWithWrapMode(buffer, ix, iy, wrapMode);
        auto p1 = sampleBufferWithWrapMode(buffer, ix + 1, iy, wrapMode);
        auto p2 = sampleBufferWithWrapMode(buffer, ix, iy + 1, wrapMode);
        auto p3 = sampleBufferWithWrapMode(buffer, ix + 1, iy + 1, wrapMode);


        return glm::mix(glm::mix(p0, p1, f.x), glm::mix(p2, p3, f.x), f.y);
    }

    glm::vec4 BaseSampler::sampleTexture(Texture* texture, const glm::vec2& uv, float lod, const glm::vec2& offset)
    {
        if (texture == nullptr || texture->isEmpty())
        {
            return glm::vec4(0);
        }

        const FilterMode filterMode = mFilterMode;
        const WrapMode   wrapMode = mWrapMode;

        if (filterMode == FilterMode::FILTER_NEAREST)
        {
            return sampleBufferNearest(texture->mBuffer.get(), uv, wrapMode, offset);
        }

        if (filterMode == FilterMode::FILTER_LINEAR)
        {
            return sampleBufferBilinear(texture->mBuffer.get(), uv, wrapMode, offset);
        }

        if (!texture->isMipmapsReady())
        {
            texture->generateMipmaps();

            if (filterMode == FilterMode::FILTER_NEAREST_MIPMAP_NEAREST || filterMode == FilterMode::FILTER_NEAREST_MIPMAP_LINEAR)
            {
                return sampleBufferNearest(texture->mBuffer.get(), uv, wrapMode, offset);
            }

            if (filterMode == FilterMode::FILTER_LINEAR_MIPMAP_NEAREST || filterMode == FilterMode::FILTER_LINEAR_MIPMAP_LINEAR)
            {
                return sampleBufferBilinear(texture->mBuffer.get(), uv, wrapMode, offset);
            }
        }

        const int32_t maxLevel = (int32_t)texture->mMipmaps.size() - 1;

        if (filterMode == FilterMode::FILTER_NEAREST_MIPMAP_NEAREST || filterMode == FilterMode::FILTER_LINEAR_MIPMAP_NEAREST)
        {
            int32_t level = glm::clamp((int32_t)std::round(lod), 0, maxLevel);

            if (filterMode == FilterMode::FILTER_NEAREST_MIPMAP_NEAREST)
            {
                return sampleBufferNearest(texture->mMipmaps[level].get(), uv, wrapMode, offset);
            }
            else
            {
                return sampleBufferBilinear(texture->mMipmaps[level].get(), uv, wrapMode, offset);
            }
        }

        if (filterMode == FilterMode::FILTER_NEAREST_MIPMAP_LINEAR || filterMode == FilterMode::FILTER_LINEAR_MIPMAP_LINEAR)
        {
            int32_t level1 = glm::clamp((int32_t)std::floor(lod), 0, maxLevel);
            int32_t level2 = glm::clamp((int32_t)std::ceil(lod), 0, maxLevel);

            glm::vec4 texel1, texel2;
            if (filterMode == FilterMode::FILTER_NEAREST_MIPMAP_LINEAR)
            {
                texel1 = sampleBufferNearest(texture->mMipmaps[level1].get(), uv, wrapMode, offset);
            }
            else
            {
                texel1 = sampleBufferBilinear(texture->mMipmaps[level1].get(), uv, wrapMode, offset);
            }

            if (level1 == level2)
            {
                return texel1;
            }
            else
            {
                if (filterMode == FilterMode::FILTER_NEAREST_MIPMAP_LINEAR)
                {
                    texel2 = sampleBufferNearest(texture->mMipmaps[level2].get(), uv, wrapMode, offset);
                }
                else
                {
                    texel2 = sampleBufferBilinear(texture->mMipmaps[level2].get(), uv, wrapMode, offset);
                }
            }

            float f = glm::fract(lod);

            return glm::mix(texel1, texel2, f);
        }

        return glm::vec4(0);
    }

    void BaseSampler2D::bindTexture(Texture* texture)
    {
        mTexture = texture;
        mWidth  = (texture == nullptr || texture->isEmpty()) ? 0 : texture->getWidth();
        mHeight = (texture == nullptr || texture->isEmpty()) ? 0 : texture->getHeight();
        mUsemipmaps = (mFilterMode != FilterMode::FILTER_NEAREST && mFilterMode != FilterMode::FILTER_LINEAR);
    }

    bool BaseSampler2D::isEmpty() const
    {
        return mTexture == nullptr;
    }

    glm::vec4 BaseSampler2D::texture2DImpl(glm::vec2& uv, float bias) 
    {
        if (mTexture == nullptr)
        {
            return { 0, 0, 0, 0 };
        }

        float lod = bias;
        return texture2DLodImpl(uv, lod);
    }

    glm::vec4 BaseSampler2D::texture2DLodImpl(glm::vec2& uv, float lod, glm::ivec2 offset) 
    {
        glm::vec4 color = sampleTexture(mTexture, uv, lod, offset);
        return color;
    }

    glm::vec4 Sampler2D::texture2D(glm::vec2 uv, float bias)
    {
        return texture2DImpl(uv, bias) / 255.0f;
    }

    glm::vec4 Sampler2D::texture2DLod(glm::vec2 uv, float lod) 
    {
        return texture2DLodImpl(uv, lod) / 255.f;
    }

    glm::vec4 Sampler2D::texture2DLodOffset(glm::vec2 uv, float lod, glm::ivec2 offset) 
    {
        return texture2DLodImpl(uv, lod, offset) / 255.f;
    }

    BaseSamplerCube::BaseSamplerCube()
    {
        mWrapMode = WrapMode::WRAP_CLAMP_TO_EDGE;
        mFilterMode = FilterMode::FILTER_LINEAR;
    }

    bool BaseSamplerCube::isEmpty() const
    {
        return mTextures[0] == nullptr;
    }

    void BaseSamplerCube::bindTexture(Texture* texture, CubeMapFace face)
    {
        mTextures[(int)face] = texture;
        if((int)face == 0)
        {
            mWidth  = (texture == nullptr || texture->isEmpty()) ? 0 : texture->getWidth();
            mHeight = (texture == nullptr || texture->isEmpty()) ? 0 : texture->getHeight();
            mUsemipmaps = (mFilterMode != FilterMode::FILTER_NEAREST && mFilterMode != FilterMode::FILTER_LINEAR);
        }
    }

    glm::vec4 BaseSamplerCube::textureCubeImpl(const glm::vec3& coord, float bias)
    {
        float lod = bias;
        return textureCubeLodImpl(coord, lod);
    }

    glm::vec4 BaseSamplerCube::textureCubeLodImpl(const glm::vec3& coord, float lod)
    {
        int32_t index;
        glm::vec2 uv;
        ConvertXYZ2UV(coord.x, coord.y, coord.z, &index, &uv.x, &uv.y);
        Texture* texture = mTextures[index];
        if (texture == nullptr) 
        {
            return glm::vec4(0);
        }

        glm::vec4 color = sampleTexture(texture, uv, lod);
        return color;
    }

    void BaseSamplerCube::ConvertXYZ2UV(float x, float y, float z, int *index, float *u, float *v)
    {
        // Ref: https://en.wikipedia.org/wiki/Cube_mapping
        float absX = std::fabs(x);
        float absY = std::fabs(y);
        float absZ = std::fabs(z);

        bool isXPositive = x > 0;
        bool isYPositive = y > 0;
        bool isZPositive = z > 0;

        float maxAxis, uc, vc;

        // POSITIVE X
        if (isXPositive && absX >= absY && absX >= absZ) 
        {
            maxAxis = absX;
            uc = -z;
            vc = y;
            *index = 0;
        }
        // NEGATIVE X
        if (!isXPositive && absX >= absY && absX >= absZ) 
        {
            maxAxis = absX;
            uc = z;
            vc = y;
            *index = 1;
        }
        // POSITIVE Y
        if (isYPositive && absY >= absX && absY >= absZ) 
        {
            maxAxis = absY;
            uc = x;
            vc = -z;
            *index = 2;
        }
        // NEGATIVE Y
        if (!isYPositive && absY >= absX && absY >= absZ) 
        {
            maxAxis = absY;
            uc = x;
            vc = z;
            *index = 3;
        }
        // POSITIVE Z
        if (isZPositive && absZ >= absX && absZ >= absY) 
        {
            maxAxis = absZ;
            uc = x;
            vc = y;
            *index = 4;
        }
        // NEGATIVE Z
        if (!isZPositive && absZ >= absX && absZ >= absY) 
        {
            maxAxis = absZ;
            uc = -x;
            vc = y;
            *index = 5;
        }

        // Convert range from -1 to 1 to 0 to 1
        *u = 0.5f * (uc / maxAxis + 1.0f);
        *v = 0.5f * (vc / maxAxis + 1.0f);
    }

    glm::vec4 SamplerCube::textureCube(const glm::vec3& coord, float bias)
    {
        return textureCubeImpl(coord, bias) / 255.0f;
    }

    glm::vec4 SamplerCube::textureCubeLod(const glm::vec3& coord, float lod)
    {
        return textureCubeLodImpl(coord, lod) / 255.0f;
    }
}


