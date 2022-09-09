#pragma once

#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <functional>
#include <atomic>
#include <thread>

#include <glm/glm.hpp>

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

	class Texture2D
	{
	public:
		using Ptr = std::shared_ptr<Texture2D>;

		static Texture2D::Ptr create(const std::string& filename);


		Texture2D();
		Texture2D(int width, int height);
		~Texture2D();

		void init(int width, int height);
		void initFromImage(Image::Ptr image);
		void initFromFile(const std::string& filename);
		void initFromColorBuffer(FrameBuffer::Ptr framebuffer);
		void initFromDepthBuffer(FrameBuffer::Ptr framebuffer);

		glm::vec4 sample(const glm::vec2& texcoord);

		glm::vec4 sampleRepeat(const glm::vec2& texcoord);
		glm::vec4 sampleClamp(const glm::vec2& texcoord);

	private:
		int mWidth = 0;
		int mHeight = 0;
		std::vector<glm::vec4> mBuffer;

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

	//enum PixelFormat
	//{
	//	PF_L8,
	//	PF_R8,
 //       PF_RG88,
 //       PF_RGB888,
 //       PF_RGBA8888,
 //       PF_RGBA4444,
 //       PF_RGB565,
 //       PF_R32F, //float
 //       PF_RG32F,
 //       PF_RGB32F,
 //       PF_RGBA32F,
 //       PF_R16F, //half float
 //       PF_RG16F,
 //       PF_RGB16F,
 //       PF_RGBA16F,
	//	PF_MAX
	//};

 //   enum Interpolation 
	//{
 //       INTERPOLATE_NEAREST,
 //       INTERPOLATE_BILINEAR,
 //       INTERPOLATE_CUBIC,
 //       INTERPOLATE_TRILINEAR,
 //       INTERPOLATE_LANCZOS,
 //   };

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

	template<typename T>
	class Texture
	{
	public:

		Texture(const Texture& other)
		{
            if (this != &other)
            {
                mType = other.mType;
                mBuffer = other.mBuffer;
                mMipmaps = other.mMipmaps;
                mMipmapReadyFlag = other.mMipmapReadyFlag;
                mMipmapGeneratingFlag = other.mMipmapGeneratingFlag;
            }
		}

		Texture& operator=(const Texture& other) 
		{
			if (this != &other)
			{
				mType = other.mType;
				mBuffer = other.mBuffer;
				mMipmaps = other.mMipmaps;
				mMipmapReadyFlag = other.mMipmapReadyFlag;
				mMipmapGeneratingFlag = other.mMipmapGeneratingFlag;
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

		void setMipmapsReady(bool ready) const
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

		static std::shared_ptr<Buffer<glm::tvec4<T>>> createBuffer()
		{
			return std::make_shared<Buffer<glm::tvec4<T>>>();
		}

		static std::shared_ptr<Buffer<glm::tvec4<T>>> createBuffer(uint32_t width, uint32_t height)
		{
			return Buffer<glm::tvec4<T>>::create(width, height);
		}

	public:
		TextureType mType = TextureType::TextureType_NONE;
		std::shared_ptr<Buffer<glm::tvec4<T>>> mBuffer = nullptr;
		std::vector<std::shared_ptr<Buffer<glm::tvec4<T>>>>  mMipmaps = nullptr;

	private:
		std::atomic<bool> mMipmapReadyFlag = false;
		std::atomic<bool> mMipmapGeneratingFlag = false;
		std::shared_ptr<std::thread> mMipmapThread = nullptr;
	};

	template<typename T>
	class Sampler
	{
	public:

		inline int32_t getWidth() const { return mWidth; }
		inline int32_t getHeight() const { return mHeight; }

		
		inline void setWrapMode(WrapMode wrapMode)
		{
			mWrapMode = wrapMode
		}

		inline void setFilterMode(FilterMode filterMode)
		{
			mFilterMode = filterMode;
		}

		glm::tvec4<T> sampleTexture(Texture<T>* texture, const glm::vec2& uv, float lod, const glm::vec2& offset);

		static glm::tvec4<T> sampleBufferNearest(const Buffer<glm::tvec4<T>>& buffer, const glm::vec2& uv, WrapMode wrapMode, const glm::ivec2& offset);

		static glm::tvec4<T> sampleBufferBilinear(const Buffer<glm::tvec4<T>>& buffer, const glm::vec2& uv, WrapMode wrapMode, const glm::ivec2& offset);

		static glm::tvec4<T> sampleBufferWithWrapMode(const Buffer<glm::tvec4<T>>& buffer, int32_t x, int32_t y, WrapMode wrapMode);

	public:
		static const glm::tvec4<T> BORDER_COLOR;

	protected:
		//std::function<float(BaseSampler<T>&)>* lod_func_ = nullptr;
		
		bool mUsemipmaps = false;
		int32_t mWidth = 0;
		int32_t mHeight = 0;
		WrapMode mWrapMode = WrapMode::WRAP_REPEAT;
		FilterMode mFilterMode = FilterMode::FILTER_LINEAR;
	};

	template<typename T>
	class BaseSampler2D : public Sampler<T>
	{
	public:
		inline void setTexture(Texture<T>* texture)
		{
			mTexture = texture;
			mWidth  = (texture == nullptr || texture->isEmpty()) ? 0 : texture->getWidth();
			mHeight = (texture == nullptr || texture->isEmpty()) ? 0 : texture->getHeight();
			mUsemipmaps = (mFilterMode != FILTER_NEAREST && mFilterMode != FILTER_LINEAR);
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
            glm::tvec4<T> color = sampleTexture(mTexture, uv, lod, offset);

            return { color[0], color[1], color[2], color[3] };
        }

	private:
		Texture<T>* mTexture = nullptr;
	};


	template<typename T>
	void Texture<T>::generateMipmaps() 
	{
		if (mMipmapReadyFlag || mMipmapGeneratingFlag) 
		{
			return;
		}

		auto generatePo2Mipmap = [](Buffer<glm::tvec4<T>>* inBuffer, Buffer<glm::tvec4<T>>* outBuffer)->
		{
			const float ratioW = (float)inBuffer->getWidth() / (float)outBuffer->getWidth();
			const float ratioH = (float)inBuffer->getHeight() / (float)outBuffer->getHeight();
			for (uint32_t y = 0; y < outBuffer->getHeight(); ++y)
			{
				for (uint32_t x = 0; x < outBuffer->getWidth(); ++x)
				{
					glm::vec2 uv = glm::vec2((float)x*ratioW, (float)y*ratioH);

					const int32_t ix = (int32_t)uv.x;
					const int32_t iy = (int32_t)uv.y;

                    glm::vec2 f = glm::fract(glm::vec2(x, y));

					auto p0 = sampleBufferWithWrapMode(inBuffer, ix, iy, WrapMode::WRAP_CLAMP_TO_EDGE);
					auto p1 = sampleBufferWithWrapMode(inBuffer, ix + 1, iy, WrapMode::WRAP_CLAMP_TO_EDGE);
					auto p2 = sampleBufferWithWrapMode(inBuffer, ix, iy + 1, WrapMode::WRAP_CLAMP_TO_EDGE);
					auto p3 = sampleBufferWithWrapMode(inBuffer, ix + 1, iy + 1, WrapMode::WRAP_CLAMP_TO_EDGE);


					auto color = glm::mix(glm::mix(p0, p1, f.x), glm::mix(p2, p3, f.x), f.y);

                    outBuffer->set(x, y, color);
				}
			}
		};

		//mMipmapThread = std::make_shared<std::thread>([&]() 
		
		{
			mMipmapGeneratingFlag = true;

			mMipmaps.clear();

			const uint32_t originWidth  = mBuffer->getWidth();
			const uint32_t originHeight = mBuffer->getHeight();

			// alloc memory for mip 0
			uint32_t mipSize = std::max(roundupPowerOf2(originWidth), roundupPowerOf2(originHeight));

			mMipmaps.push_back(Texture<T>::createBuffer(mipSize, mipSize));
			
			Buffer<glm::tvec4<T>>* currentBufferPtr = mMipmaps.back().get();

			Buffer<glm::tvec4<T>>* originBufferPtr = mBuffer.get();

			if (mipSize == originBufferPtr->getWidth() && mipSize == originBufferPtr->getHeight())
			{
				originBufferPtr->copyTo(currentBufferPtr->get());
			}
			else 
			{
				generatePo2Mipmap(currentBufferPtr, originBufferPtr);
			}

			// other levels
			while (mipSize > 1) 
			{
				mipSize /= 2;

				mMipmaps.push_back(Texture<T>::CreateBuffer(mipSize, mipSize));

				currentBufferPtr = mMipmaps.back().get();

				originBufferPtr = mMipmaps[mMipmaps.size() - 2].get();
				
				generatePo2Mipmap(currentBufferPtr, originBufferPtr);
			}

			mMipmapGeneratingFlag = false;
			mMipmapReadyFlag = true;
		}
			//});
	}

	template<typename T>
	inline glm::tvec4<T> Sampler<T>::sampleTexture(Texture<T>* texture, const glm::vec2& uv, float lod, const glm::vec2& offset)
	{
		if (texture == nullptr || texture->isEmpty())
		{
			return glm::tvec4<T>(0);
		}

		const FilterMode filterMode = mFilterMode;
		const WrapMode   wrapMode   = mWrapMode;

		if (filterMode == FilterMode::FILTER_NEAREST)
		{
			return sampleBufferNearest(texture->mBuffer->get(), uv, wrapMode, offset);
		}

		if (filterMode == FilterMode::FILTER_LINEAR)
		{
			return sampleBufferBilinear(texture->mBuffer->get(), uv, wrapMode, offset);
		}

		if (!texture->isMipmapsReady())
		{
			texture->generateMipmaps();

			if (filterMode == FilterMode::FILTER_NEAREST_MIPMAP_NEAREST || filterMode == FilterMode::FILTER_NEAREST_MIPMAP_LINEAR)
			{
				return sampleBufferNearest(texture->mBuffer->get(), uv, wrapMode, offset);
			}

			if (filterMode == FilterMode::FILTER_LINEAR_MIPMAP_NEAREST || filterMode == FilterMode::FILTER_LINEAR_MIPMAP_LINEAR)
			{
				return sampleBufferBilinear(texture->mBuffer->get(), uv, wrapMode, offset);
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
            int32_t level2 = glm::clamp((int32_t)std::ceil(lod), 0,  maxLevel);

            glm::tvec4<T> texel1, texel2;
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

		return glm::tvec4<T>(0);
	}

	template<typename T>
	inline glm::tvec4<T> Sampler<T>::sampleBufferNearest(const Buffer<glm::tvec4<T>>& buffer, const glm::vec2& uv, WrapMode wrapMode, const glm::ivec2& offset)
	{
		const int32_t x = (int32_t)(uv.x * (buffer.getWidth()  - 1) + 0.5f) + offset.x;
		const int32_t y = (int32_t)(uv.y * (buffer.getHeight() - 1) + 0.5f) + offset.y;

		return sampleBufferWithWrapMode(buffer, x, y, wrapMode);
	}

	template<typename T>
	inline glm::tvec4<T> Sampler<T>::sampleBufferBilinear(const Buffer<glm::tvec4<T>>& buffer, const glm::vec2& uv, WrapMode wrapMode, const glm::ivec2& offset)
	{
		const float x = (uv.x * buffer.getWidth()  - 0.5f) + offset.x;
		const float y = (uv.y * buffer.getHeight() - 0.5f) + offset.y;
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

	template<typename T>
	inline glm::tvec4<T> Sampler<T>::sampleBufferWithWrapMode(const Buffer<glm::tvec4<T>>& buffer, int32_t x, int32_t y, WrapMode wrapMode)
	{
		const int32_t width  = buffer.getWidth();
		const int32_t height = buffer.getHeight();

		auto uvMod = [](int32_t i, int32_t n)
		{
			return (i & (n-1) + n) & (n-1); // (i % n + n) % n
		};

		auto uvMirror = [](int32_t i)
		{
			return i >= 0 ? i : (-1 - i)
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
			sampleX = uvMod(x, 2 * w);
			sampleY = uvMod(y, 2 * h);

			sampleX -= width;
			sampleY -= height;

            sampleX = uvMirror(sampleX);
            sampleY = uvMirror(sampleY);

			sampleX = width - 1 - sampleX;
			sampleY = height - 1 - sampleY;
			break;
		case WrapMode::WRAP_CLAMP_TO_EDGE:
			if(x < 0) sampleX = 0;
			if(y < 0) sampleY = 0;
			if(x >= width) sampleX = width - 1;
			if(y >= height) sampleY = height - 1;
			break;
		case WrapMode::WRAP_CLAMP_TO_BORDER:
			if (x < 0 || x >= w) return BORDER_COLOR;
			if (y < 0 || y >= h) return BORDER_COLOR;
			break;
		case WrapMode::WRAP_CLAMP_TO_ZERO:
			if (x < 0 || x >= w) return glm::tvec4<T>(0);
			if (y < 0 || y >= h) return glm::tvec4<T>(0);
			break;
		default:
			break;
		}

		glm::tvec4<T>* pixel = buffer.get(sampleX, sampleY);

		if (pixel)
		{
			return *pixel;
		}

		return glm::tvec4(0);
	}
}