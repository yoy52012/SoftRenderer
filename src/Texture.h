#pragma once

#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <functional>
#include <atomic>
#include <thread>

#include "MathUtils.h"
#include "Image.h"
#include "FrameBuffer.h"
#include "Buffer.h"

namespace SoftRenderer
{
	enum TextureType 
	{
		TextureType_NONE = 0,
		TextureType_DIFFUSE,
		TextureType_NORMALS,
		TextureType_EMISSIVE,
		TextureType_PBR_ALBEDO,
		TextureType_PBR_METAL_ROUGHNESS,
		TextureType_PBR_AMBIENT_OCCLUSION,

		TextureType_CUBE_MAP_POSITIVE_X,
		TextureType_CUBE_MAP_NEGATIVE_X,
		TextureType_CUBE_MAP_POSITIVE_Y,
		TextureType_CUBE_MAP_NEGATIVE_Y,
		TextureType_CUBE_MAP_POSITIVE_Z,
		TextureType_CUBE_MAP_NEGATIVE_Z,

		TextureType_EQUIRECTANGULAR,
	};

	enum class WrapMode
	{
		WRAP_REPEAT,
		WRAP_MIRRORED_REPEAT,
		WRAP_CLAMP_TO_EDGE,
		WRAP_CLAMP_TO_BORDER,
		WRAP_CLAMP_TO_ZERO,
	};

	enum class FilterMode
	{
		FILTER_NEAREST,
		FILTER_LINEAR,
		FILTER_NEAREST_MIPMAP_NEAREST,
		FILTER_LINEAR_MIPMAP_NEAREST,
		FILTER_NEAREST_MIPMAP_LINEAR,
		FILTER_LINEAR_MIPMAP_LINEAR,
	};

    // Find the first greater number which equals to 2^n, from jdk1.7
    static int32_t roundUpToPowerOf2(int32_t n)
    {
        static const int32_t max = 1 << 30;
        n -= 1;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        return n < 0 ? 1 : (n >= max ? max : n + 1);
    }

	class Texture
	{
	public:

		Texture()
		{

		}

		Texture(int32_t width, int32_t height)
		{
			mBuffer = TextureBuffer<glm::vec4>::create(width, height);
		}

		Texture(const Texture& other)
		{
            if (this != &other)
            {
                mType = other.mType;
                mBuffer = other.mBuffer;
                mMipmaps = other.mMipmaps;
                mMipmapReadyFlag = (bool)other.mMipmapReadyFlag;
                mMipmapGeneratingFlag = (bool)other.mMipmapGeneratingFlag;
            }
		}

		Texture& operator=(const Texture& other) 
		{
			if (this != &other)
			{
				mType = other.mType;
				mBuffer = other.mBuffer;
				mMipmaps = other.mMipmaps;
				mMipmapReadyFlag = (bool)other.mMipmapReadyFlag;
				mMipmapGeneratingFlag = (bool)other.mMipmapGeneratingFlag;
			}
			return *this;
		}

		virtual ~Texture() 
		{
			if (mMipmapThread) 
			{
				mMipmapThread->join();
			}
		}

		void initFromImage(Image::Ptr image)
		{
			const int32_t imageWidth = image->getWidth();
			const int32_t imageHeight = image->getHeight();

			mBuffer = createTextureBuffer(imageWidth, imageHeight);

			std::vector<uint8_t> out;

            for (int32_t y = 0; y < imageHeight; ++y)
            {
                for (int32_t x = 0; x < imageWidth; ++x)
                {
                    glm::vec4 p = image->getPixel(x, y);
                    glm::ivec4 ip = glm::ivec4(p.r * 255, p.g * 255, p.b * 255, p.a * 255);

                    mBuffer->set(x, y, ip);
                }
            }

			generateMipmaps();

		}

		void clear() 
		{
			mType = TextureType_NONE;
			mBuffer = nullptr;
			mMipmaps.clear();
			mMipmapReadyFlag = false;
			mMipmapGeneratingFlag = false;
		}

		bool isMipmapsReady() const
		{
			return mMipmapReadyFlag;
		}

		void setMipmapsReady(bool ready)
		{
			mMipmapReadyFlag = ready;
		}

		inline uint32_t getWidth() const 
		{
			if (mBuffer)
			{
				return mBuffer->getWidth();
			}
			return 0;
		}

		inline uint32_t getHeight() const
		{
			if (mBuffer)
			{
				return mBuffer->getHeight();
			}
			return 0;
		}

		inline bool isEmpty() const
		{
			if (mBuffer)
			{
				return mBuffer->isEmpty();
			}

			return true;
		}

        void generateMipmaps();

	private:
		std::shared_ptr<TextureBuffer<glm::vec4>> createTextureBuffer(int32_t width ,int32_t height)
		{
			return std::make_shared<LinearTextureBuffer<glm::vec4>>(width, height);
		}

	public:
		TextureType mType = TextureType::TextureType_NONE;
		std::shared_ptr<TextureBuffer<glm::vec4>> mBuffer = nullptr;
		std::vector<std::shared_ptr<TextureBuffer<glm::vec4>>>  mMipmaps;

	private:
		std::atomic<bool> mMipmapReadyFlag = false;
		std::atomic<bool> mMipmapGeneratingFlag = false;
		std::shared_ptr<std::thread> mMipmapThread = nullptr;
	};

    class Texture2D : public Texture
    {
    public:
        Texture2D(int32_t width, int32_t height);



    };

	class Sampler
	{
	public:
		virtual bool isEmpty() const = 0;
		inline int32_t getWidth() const { return mWidth; }
		inline int32_t getHeight() const { return mHeight; }
		
		inline void setWrapMode(WrapMode wrapMode)
		{
			mWrapMode = wrapMode;
		}

		inline void setFilterMode(FilterMode filterMode)
		{
			mFilterMode = filterMode;
		}

		glm::vec4 sampleTexture(Texture* texture, const glm::vec2& uv, float lod, const glm::vec2& offset);

        static glm::vec4 sampleBufferWithWrapMode(TextureBuffer<glm::vec4>* buffer, int32_t x, int32_t y, WrapMode wrapMode);

		static glm::vec4 sampleBufferNearest(TextureBuffer<glm::vec4>* buffer, const glm::vec2& uv, WrapMode wrapMode, const glm::ivec2& offset);

		static glm::vec4 sampleBufferBilinear(TextureBuffer<glm::vec4>* buffer, const glm::vec2& uv, WrapMode wrapMode, const glm::ivec2& offset);

	public:
		static const glm::vec4 BORDER_COLOR;

	protected:
		//std::function<float(BaseSampler<T>&)>* lod_func_ = nullptr;
		
		bool mUsemipmaps = false;
		int32_t mWidth = 0;
		int32_t mHeight = 0;
		WrapMode mWrapMode = WrapMode::WRAP_REPEAT;
		FilterMode mFilterMode = FilterMode::FILTER_LINEAR;
	};

	class BaseSampler2D : public Sampler
	{
	public:
		inline void bindTexture(Texture* texture)
		{
			mTexture = texture;
			mWidth  = (texture == nullptr || texture->isEmpty()) ? 0 : texture->getWidth();
			mHeight = (texture == nullptr || texture->isEmpty()) ? 0 : texture->getHeight();
			mUsemipmaps = (mFilterMode != FilterMode::FILTER_NEAREST && mFilterMode != FilterMode::FILTER_LINEAR);
		}

		inline bool isEmpty() const override
		{
			return mTexture == nullptr;
		}

        virtual glm::vec4 texture2DImpl(glm::vec2& uv, float bias = 0.0f) 
		{
			if (mTexture == nullptr)
			{
				return { 0, 0, 0, 0 };
			}

            float lod = bias;
            //if (Sampler<T>::use_mipmaps && BaseSampler<T>::lod_func_) 
			//{
            //    lod += (*BaseSampler<T>::lod_func_)(*this);
            //}

            return texture2DLodImpl(uv, lod);
        }

        virtual glm::vec4 texture2DLodImpl(glm::vec2& uv, float lod = 0.0f, glm::ivec2 offset = glm::ivec2(0)) 
		{
            glm::vec4 color = sampleTexture(mTexture, uv, lod, offset);

            return { color[0], color[1], color[2], color[3] };
        }

	private:
		Texture* mTexture = nullptr;
	};

	
    class Sampler2D : public BaseSampler2D
	{
    public:

        glm::vec4 texture2D(glm::vec2 uv, float bias = 0.f) 
		{
            return BaseSampler2D::texture2DImpl(uv, bias) / 255.f;
        }

        glm::vec4 texture2DLod(glm::vec2 uv, float lod = 0.f) 
		{
            return BaseSampler2D::texture2DLodImpl(uv, lod) / 255.f;
        }

        glm::vec4 texture2DLodOffset(glm::vec2 uv, float lod, glm::ivec2 offset) 
		{
            return BaseSampler2D::texture2DLodImpl(uv, lod, offset) / 255.f;
        }
    };

}