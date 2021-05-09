#pragma once

#include <vector>

#include <glm/glm.hpp>

namespace SoftRenderer
{
	class FrameBuffer
	{
	public:
		FrameBuffer(unsigned int width, unsigned int height);
		~FrameBuffer();

		unsigned int getWidth() { return m_width; }
		unsigned int getHeight() { return m_height; }

		unsigned char* getColorBuffer() { return m_color_buffer.data(); }
		float* getDepthBuffer() { return m_depth_buffer.data(); }

		void clearColor(const glm::vec4& color);
		void clearDepth(float depth);

		void writeColor(unsigned int x, unsigned int y, const glm::vec4& color);
		void writeDepth(unsigned int x, unsigned int y, float depth);


	private:
		std::vector<unsigned char> m_color_buffer;
		std::vector<float> m_depth_buffer;
		unsigned int m_width;
		unsigned int m_height;
	};

}
