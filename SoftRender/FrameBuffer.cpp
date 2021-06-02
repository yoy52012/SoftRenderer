#include "FrameBuffer.h"

#include<cassert>
#include<iostream>
namespace SoftRenderer
{
	FrameBuffer::FrameBuffer(unsigned int width, unsigned int height)
		:m_width(width), m_height(height)
	{
		assert(width > 0 && height > 0);

		m_color_buffer.resize(m_width * m_height * 4, 255);
		m_depth_buffer.resize(m_width * m_height, 1.0f);
	}

	FrameBuffer::~FrameBuffer()
	{
	}

	void FrameBuffer::clearColor(const glm::vec4& color)
	{
		int numPixel = m_width * m_height;

		for (int i = 0; i < numPixel; ++i) {
			m_color_buffer[i * 4 + 0] = static_cast<unsigned char>(255 * color.x);
			m_color_buffer[i * 4 + 1] = static_cast<unsigned char>(255 * color.y);
			m_color_buffer[i * 4 + 2] = static_cast<unsigned char>(255 * color.z);
			m_color_buffer[i * 4 + 3] = static_cast<unsigned char>(255 * color.w);
		}

	}

	void FrameBuffer::clearDepth(float depth)
	{
		int numPixel = m_width * m_height;

		for (int i = 0; i < numPixel; ++i) {
			m_depth_buffer[i] = depth;
		}
	}

	void FrameBuffer::writeColor(unsigned int x, unsigned int y, const glm::vec4& color)
	{
		if (x < 0 || y < 0 || x > m_width || y > m_height)
			return;

		unsigned int index = y * m_width + x;
		m_color_buffer[index * 4 + 0] = static_cast<unsigned char>(255 * color.x);
		m_color_buffer[index * 4+ 1] = static_cast<unsigned char>(255 * color.y);
		m_color_buffer[index * 4+ 2] = static_cast<unsigned char>(255 * color.z);
		m_color_buffer[index * 4+ 3] = static_cast<unsigned char>(255 * color.w);

	}

	void FrameBuffer::writeDepth(unsigned int x, unsigned int y, float depth)
	{
		if (x < 0 || y < 0 || x > m_width || y > m_height)
			return;

		unsigned int index = y * m_width + x;
		m_color_buffer[index] = depth;
	}
}