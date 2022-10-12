#include "FrameBuffer.h"

#include<cassert>
#include<iostream>

namespace SoftRenderer
{
	FrameBuffer::FrameBuffer(unsigned int width, unsigned int height)
		:mWidth(width), mHeight(height)
	{
		assert(width > 0 && height > 0);

		mColorBuffer.resize(mWidth * mHeight * 4, 255);
		mDepthBuffer.resize(mWidth * mHeight, 1.0f);
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
        const int32_t numPixel = mWidth * mHeight;

        for (int32_t i = 0; i < numPixel; ++i) 
		{
            mColorBuffer[i * 4 + 0] = static_cast<unsigned char>(255 * r);
            mColorBuffer[i * 4 + 1] = static_cast<unsigned char>(255 * g);
            mColorBuffer[i * 4 + 2] = static_cast<unsigned char>(255 * b);
            mColorBuffer[i * 4 + 3] = static_cast<unsigned char>(255 * a);
        }
	}

	void FrameBuffer::clearDepth(float depth)
	{
		const int32_t numPixel = mWidth * mHeight;

		for (int32_t i = 0; i < numPixel; ++i) 
		{
			mDepthBuffer[i] = depth;
		}
	}

	void FrameBuffer::writeColor(unsigned int x, unsigned int y, const glm::vec4& color)
	{
		if (x < 0 || y < 0 || x > mWidth || y > mHeight)
			return;

		unsigned int index = y * mWidth + x;
		mColorBuffer[index * 4 + 0] = static_cast<unsigned char>(255 * color.x);
		mColorBuffer[index * 4+ 1] = static_cast<unsigned char>(255 * color.y);
		mColorBuffer[index * 4+ 2] = static_cast<unsigned char>(255 * color.z);
		mColorBuffer[index * 4+ 3] = static_cast<unsigned char>(255 * color.w);

	}

	void FrameBuffer::writeDepth(unsigned int x, unsigned int y, float depth)
	{
		if (x < 0 || y < 0 || x > mWidth || y > mHeight)
			return;

		unsigned int index = y * mWidth + x;
		mColorBuffer[index] = depth;
	}
}