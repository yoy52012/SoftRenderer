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

		FrameBuffer(uint32_t width, uint32_t height);
		~FrameBuffer();

		uint32_t getWidth() { return mWidth; }
		uint32_t getHeight() { return mHeight; }

		unsigned char* getColorBuffer() { return mColorBuffer.data(); }
		float* getDepthBuffer() { return mDepthBuffer.data(); }

		void clearColor(const glm::vec4& color);
		void clearColor(float r, float g, float b, float a);
		void clearDepth(float depth);

		void writeColor(uint32_t x, uint32_t y, const glm::vec4& color);
		void writeDepth(uint32_t x, uint32_t y, float depth);

		float getDepth(uint32_t x, uint32_t y);
		glm::vec4 getColor(uint32_t x, uint32_t y);


	private:
		std::vector<uint8_t> mColorBuffer;
		std::vector<float> mDepthBuffer;
		unsigned int mWidth;
		unsigned int mHeight;
	};

}
