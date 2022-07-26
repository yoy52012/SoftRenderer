#pragma once

#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "Image.h"
#include "FrameBuffer.h"
#include "Buffer.h"

namespace SoftRenderer
{
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


	template<typename T>
	class Texture
	{
	public:

	private:
		void generateMipmaps();


	private:
		

		std::shared_ptr<Buffer<glm::tvec4<T>>> mDataBuffer;
		std::vector<std::shared_ptr<Buffer<glm::tvec4<T>>>> mMipmaps;
	};

	template<typename T>
	class Sampler
	{
	public:
		
		static glm::tvec4<T> sampleNearest(const Buffer<glm::tvec4<T>>& buffer, const glm::vec2& uv, WrapMode wrapMode, const glm::ivec2& offset);

		static glm::tvec4<T> sampleBilinear(const Buffer<glm::tvec4<T>>& buffer, const glm::vec2& uv, WrapMode wrapMode, const glm::ivec2& offset);

		static glm::tvec4<T> sampleWithWrapMode(const Buffer<glm::tvec4<T>>& buffer, int32_t x, int32_t y, WrapMode wrapMode);

	};

	// Find the first greater number which equals to 2^n, from jdk1.7
	static int roundUpToPowerOf2(int n) 
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

	template<typename T>
	void Texture<T>::generateMipmaps()
	{
		if (mipmaps_ready_ || mipmaps_generating_) 
		{
			return;
		}

		mMipmaps.clear();

		int width = (int)buffer->GetWidth();
		int height = (int)buffer->GetHeight();

		
		int levelSize = std::max(roundUpToPowerOf2(width), roundUpToPowerOf2(height));

		mMipmaps.emplace_back(Buffer<glm::tvec4<T>>::create());

		Buffer<glm::tvec4<T>>* levelBuffer = mMipmaps.back().get();
		levelBuffer->init(levelSize, levelSize);

	}

	template<typename T>
	inline glm::tvec4<T> Sampler<T>::sampleNearest(const Buffer<glm::tvec4<T>>& texture, const glm::vec2& uv, WrapMode wrapMode, const glm::ivec2& offset)
	{
		const int32_t x = (int32_t)(uv.x * (texture.getWidth()  - 1) + 0.5f) + offset.x;
		const int32_t y = (int32_t)(uv.y * (texture.getHeight() - 1) + 0.5f) + offset.y;

		return sampleWithWrapMode(texture, x, y, wrapMode);
	}

	template<typename T>
	inline glm::tvec4<T> Sampler<T>::sampleBilinear(const Buffer<glm::tvec4<T>>& texture, const glm::vec2& uv, WrapMode wrapMode, const glm::ivec2& offset)
	{
		const float x = (uv.x * texture.getWidth()  - 0.5f) + offset.x;
		const float y = (uv.y * texture.getHeight() - 0.5f) + offset.y;
		const int32_t ix = (int32_t)x;
		const int32_t iy = (int32_t)y;

		/*********************
		 *   p2--p3
		 *   |   |
		 *   p0--p1
		 * Note: p0 is (ix,iy)
		 ********************/

		auto p0 = sampleWithWrapMode(texture, ix, iy, wrapMode);
		auto p1 = sampleWithWrapMode(texture, ix + 1, iy, wrapMode);
		auto p2 = sampleWithWrapMode(texture, ix, iy + 1, wrapMode);
		auto p3 = sampleWithWrapMode(texture, ix + 1, iy + 1, wrapMode);

		glm::vec2 f = glm::fract(glm::vec2(x, y));

		return glm::mix(glm::mix(s0, s1, f.x), glm::mix(s2, s3, f.x), f.y);
	}

	template<typename T>
	inline glm::tvec4<T> Sampler<T>::sampleWithWrapMode(const Buffer<glm::tvec4<T>>& buffer, int32_t x, int32_t y, WrapMode wrapMode)
	{
		const int32_t width  = buffer.getWidth();
		const int32_t height = buffer.getHeight();

		auto uvRepeat = [](int i, int n)
		{
			return (i & (n-1) + n) & (n-1); // (i % n + n) % n
		};

		auto uvMirror = [](int i)
		{
			return i >= 0 ? i : (-1 - i)
		};
		
		int32_t sampleX = x;
		int32_t sampleY = y; 
		switch (wrapMode)
		{
		case WrapMode::WRAP_REPEAT:
			sampleX = uvRepeat(x, width);
			sampleY = uvRepeat(y, height);
			break;
		case WrapMode::WRAP_MIRRORED_REPEAT:
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

		glm::tvec4<T> pixel = buffer.get(sampleX, sampleY);

		return pixel;
	}
}