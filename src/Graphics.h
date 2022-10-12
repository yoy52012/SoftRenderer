#pragma once

#include "Mesh.h"
#include "Shader.h"
#include "FrameBuffer.h"

namespace SoftRenderer
{
	class Graphics
	{
	public:
		struct Viewport
		{
			float x;
			float y;
			float width;
			float height;
		};

	public:
		void init(int width, int height);

		void clear(float r, float g, float b, float a);

		void drawTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2);

		void drawMesh(const Mesh* mesh);

		void clearBuffer(const glm::vec4& color);

		void swapBuffer();

		FrameBuffer::Ptr& getOutput();

		void setModelMatrix(const glm::mat4& mat);

		void setViewMatrix(const glm::mat4& mat);

		void setProjMatrix(const glm::mat4& mat);

		void drawLine(const glm::vec2& v0, const glm::vec2& v1);

	private:
		void processVertexShader();

	private:
		glm::vec3 perspectiveDivide(const glm::vec4& pos);

		glm::vec3 viewportTransform(const glm::vec3& ndcCoord);

		float interpolateDepth(const std::array<float, 3>& screenDepth, const glm::vec3& weight);

		bool edgeFunction(const glm::ivec2& a, const glm::ivec2& b, const glm::ivec2& c);

		void rasterizeTriangle(const Shader::VertexData& vertex0, const Shader::VertexData& vertex1, const Shader::VertexData& vertex2);

		void rasterizeTriangle2(const Shader::VertexData& vertex0, const Shader::VertexData& vertex1, const Shader::VertexData& vertex2);

		void rasterizeTriangle3(const Shader::VertexData& vertex0, const Shader::VertexData& vertex1, const Shader::VertexData& vertex2);

		void rasterizeTopTriangle(const Shader::VertexData& v0, const Shader::VertexData& v1, const Shader::VertexData& v2);

		void rasterizeBottomTriangle(const Shader::VertexData& v0, const Shader::VertexData& v1, const Shader::VertexData& v2);

		void scanLine(const Shader::VertexData& left, const Shader::VertexData& right);


	private:
		Shader::Ptr mShader;

		FrameBuffer::Ptr mBackBuffer;
		FrameBuffer::Ptr mFrontBuffer;

		int mWidth;
		int mHeight;

		Viewport mViewport;
	};
}