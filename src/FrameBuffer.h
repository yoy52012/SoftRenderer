#pragma once

#include <vector>
#include <memory>

#include <glm/glm.hpp>

namespace SoftRenderer
{
	class FrameBuffer
	{
	public:
		using Ptr = std::shared_ptr<FrameBuffer>;

		FrameBuffer(unsigned int width, unsigned int height);
		~FrameBuffer();

		unsigned int getWidth() { return mWidth; }
		unsigned int getHeight() { return mHeight; }

		unsigned char* getColorBuffer() { return mColorBuffer.data(); }
		float* getDepthBuffer() { return mDepthBuffer.data(); }

		void clearColor(const glm::vec4& color);
		void clearDepth(float depth);

		void writeColor(unsigned int x, unsigned int y, const glm::vec4& color);
		void writeDepth(unsigned int x, unsigned int y, float depth);


	private:
		std::vector<unsigned char> mColorBuffer;
		std::vector<float> mDepthBuffer;
		unsigned int mWidth;
		unsigned int mHeight;
	};

}
