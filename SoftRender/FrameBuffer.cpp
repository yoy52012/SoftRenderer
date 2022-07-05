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
		int numPixel = mWidth * mHeight;

		for (int i = 0; i < numPixel; ++i) {
			mColorBuffer[i * 4 + 0] = static_cast<unsigned char>(255 * color.x);
			mColorBuffer[i * 4 + 1] = static_cast<unsigned char>(255 * color.y);
			mColorBuffer[i * 4 + 2] = static_cast<unsigned char>(255 * color.z);
			mColorBuffer[i * 4 + 3] = static_cast<unsigned char>(255 * color.w);
		}

	}

	void FrameBuffer::clearDepth(float depth)
	{
		int numPixel = mWidth * mHeight;

		for (int i = 0; i < numPixel; ++i) {
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