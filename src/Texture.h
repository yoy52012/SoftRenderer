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

	enum class WarpMode
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
}