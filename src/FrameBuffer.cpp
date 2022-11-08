#include "FrameBuffer.h"

#include<cassert>
#include<iostream>

namespace SoftRenderer
{
	FrameBuffer::FrameBuffer(uint32_t width, uint32_t height)
		:mWidth(width), mHeight(height)
	{
		assert(width > 0 && height > 0);

		mColorBuffer.resize(mWidth * mHeight * 4, 255);
		mDepthBuffer.resize(mWidth * mHeight, 1.0f);

		std::fill(mColorBuffer.begin(), mColorBuffer.end(), 0);
		std::fill(mDepthBuffer.begin(), mDepthBuffer.end(), 0.0f);
	}

	FrameBuffer::~FrameBuffer()
	{
	}

	void FrameBuffer::clearColor(const glm::vec4& color)
	{
		clearColor(color.r, color.g, color.b, color.a);
	}

	void FrameBuffer::clearColor(float r, float g, float b, float a)
	{
        const uint32_t numPixel = mWidth * mHeight;

        for (uint32_t i = 0; i < numPixel; ++i) 
		{
            mColorBuffer[static_cast<uint64_t>(i) * 4 + 0] = static_cast<unsigned char>(255 * glm::clamp<float>(r, 0.0f, 1.0f));
            mColorBuffer[static_cast<uint64_t>(i) * 4 + 1] = static_cast<unsigned char>(255 * glm::clamp<float>(g, 0.0f, 1.0f));
            mColorBuffer[static_cast<uint64_t>(i) * 4 + 2] = static_cast<unsigned char>(255 * glm::clamp<float>(b, 0.0f, 1.0f));
            mColorBuffer[static_cast<uint64_t>(i) * 4 + 3] = static_cast<unsigned char>(255 * glm::clamp<float>(a, 0.0f, 1.0f));
        }
	}

	void FrameBuffer::clearDepth(float depth)
	{
		const uint32_t numPixel = mWidth * mHeight;

		for (uint32_t i = 0; i < numPixel; ++i)
		{
			mDepthBuffer[i] = depth;
		}
	}

	void FrameBuffer::writeColor(uint32_t x, uint32_t y, const glm::vec4& color)
	{
		if (x < 0 || y < 0 || x >= mWidth || y >= mHeight)
			return;

		const uint32_t index = y * mWidth + x;
        mColorBuffer[static_cast<uint64_t>(index) * 4 + 0] = static_cast<uint8_t>(255 * color.x);
        mColorBuffer[static_cast<uint64_t>(index) * 4 + 1] = static_cast<uint8_t>(255 * color.y);
        mColorBuffer[static_cast<uint64_t>(index) * 4 + 2] = static_cast<uint8_t>(255 * color.z);
        mColorBuffer[static_cast<uint64_t>(index) * 4 + 3] = static_cast<uint8_t>(255 * color.w);

	}

	void FrameBuffer::writeDepth(uint32_t x, uint32_t y, float depth)
	{
		if (x < 0 || y < 0 || x >= mWidth || y >= mHeight)
			return;

		const uint32_t index = y * mWidth + x;
		mDepthBuffer[index] = depth;
	}

	float FrameBuffer::getDepth(uint32_t x, uint32_t y)
	{
        if (x < 0 || y < 0 || x >= mWidth || y >= mHeight)
            return 0.0f;

		const uint32_t index = y * mWidth + x;
		return mDepthBuffer[index];
	}

	glm::vec4 FrameBuffer::getColor(uint32_t x, uint32_t y)
	{
        if (x < 0 || y < 0 || x >= mWidth || y >= mHeight)
            return glm::vec4(0.0f);

		const uint32_t index = y * mWidth + x;
		const glm::vec4 color = glm::vec4(mColorBuffer[static_cast<uint64_t>(index) * 4 + 0], mColorBuffer[static_cast<uint64_t>(index) * 4 + 1], mColorBuffer[static_cast<uint64_t>(index) * 4 + 2], mColorBuffer[static_cast<uint64_t>(index) * 4 + 3]);

		return color;
	}
}