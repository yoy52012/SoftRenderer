#include "Graphics.h"

#include <iostream>
#include <algorithm>
#include <array>
#include "Utils.h"
namespace SoftRenderer
{
	void Graphics::init(int width, int height)
	{
		mWidth = width;
		mHeight = height;

		mFrontBuffer = std::make_shared<FrameBuffer>(width, height);
		mBackBuffer = std::make_shared<FrameBuffer>(width, height);

		mShader = std::make_shared<Shader>();

		std::string a = "container2.png";
		Image::Ptr image = Image::create(IMAGE_DIR + a);
		Texture2D::Ptr albedo = std::make_shared<Texture2D>();
		albedo->initFromImage(image);
		mShader->mUniforms.albedo = albedo;
	}

	void Graphics::drawTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2)
	{
		Shader::VertexData vd0, vd1, vd2;
		vd0 = mShader->vertexShader(v0);
		vd1 = mShader->vertexShader(v1);
		vd2 = mShader->vertexShader(v2);

		glm::vec3 ndcPosition0 = perspectiveDivede(vd0.clipPostion);
		glm::vec3 ndcPosition1 = perspectiveDivede(vd1.clipPostion);
		glm::vec3 ndcPosition2 = perspectiveDivede(vd2.clipPostion);

		vd0.clipW = 1.0f / vd0.clipPostion.w;
		vd1.clipW = 1.0f / vd1.clipPostion.w;
		vd2.clipW = 1.0f / vd2.clipPostion.w;

		vd0.texcoord *= vd0.clipW;
		vd1.texcoord *= vd1.clipW;
		vd2.texcoord *= vd2.clipW;

		glm::vec3 tmpPos0 = viewportTransform(ndcPosition0);
		glm::vec3 tmpPos1 = viewportTransform(ndcPosition1);
		glm::vec3 tmpPos2 = viewportTransform(ndcPosition2);

		vd0.screenPosition = glm::vec2(tmpPos0.x, tmpPos0.y);
		vd1.screenPosition = glm::vec2(tmpPos1.x, tmpPos1.y);
		vd2.screenPosition = glm::vec2(tmpPos2.x, tmpPos2.y);

		rasterizeTriangle(vd0, vd1, vd2);
	}

	void Graphics::drawMesh(const Mesh& mesh)
	{
		if (mesh.indices.empty() || mesh.vertices.empty())
			return;

		for (int i = 0; i < mesh.indices.size(); i+=3)
		{
			Vertex v0, v1, v2;
			v0 = mesh.vertices[mesh.indices[i]];
			v1 = mesh.vertices[mesh.indices[i + 1]];
			v2 = mesh.vertices[mesh.indices[i + 2]];

			drawTriangle(v0, v1, v2);
		}
	}

	void Graphics::clearBuffer(const glm::vec4& color)
	{
		mBackBuffer->clearColor(color);
	}

	void Graphics::swapBuffer()
	{
		auto tmp = std::move(mFrontBuffer);
		mFrontBuffer = std::move(mBackBuffer);
		mBackBuffer = std::move(tmp);
	}

	FrameBuffer::Ptr& Graphics::getOutput()
	{
		return mFrontBuffer;
	}

	void Graphics::setModelMatrix(const glm::mat4& mat)
	{
		mShader->setModelMatrix(mat);
	}

	void Graphics::setViewMatrix(const glm::mat4& mat)
	{
		mShader->setViewMatrix(mat);
	}

	void Graphics::setProjMatrix(const glm::mat4& mat)
	{
		mShader->setProjMatrix(mat);
	}

	bool Graphics::edgeFunction(const glm::ivec2& a, const glm::ivec2& b, const glm::ivec2& c)
	{
		return ((c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x) >= 0 ) ;	
	}

	glm::vec3 Graphics::perspectiveDivede(const glm::vec4& clipCoord)
	{
		float x = clipCoord.x / clipCoord.w;
		float y = clipCoord.y / clipCoord.w;
		float z = clipCoord.z / clipCoord.w;

		return glm::vec3(x, y, z);
	}

	/*
	 * for viewport transformation, see subsection 2.12.1 of
	 * https://www.khronos.org/registry/OpenGL/specs/es/2.0/es_full_spec_2.0.pdf
     */
	glm::vec3 Graphics::viewportTransform(const glm::vec3& ndcCoord)
	{
		float x = (ndcCoord.x + 1.0f) * 0.5f * static_cast<float>(mWidth);
		float y = (ndcCoord.y + 1.0f) * 0.5f * static_cast<float>(mHeight);
		float z = (ndcCoord.z + 1.0f) * 0.5f;
		return glm::vec3(x, y, z);
	}

	float Graphics::interpolateDepth(const std::array<float, 3>& screenDepth, const glm::vec3& weight)
	{
		float d0 = screenDepth[0] * weight.x;
		float d1 = screenDepth[1] * weight.y;
		float d2 = screenDepth[2] * weight.z;

		return d0 + d1 + d2;
	}

	glm::vec2 interpolateTexcoord(const std::array<glm::vec2, 3>& texcoord, const glm::vec3& weight, const std::array<float, 3>& recipW)
	{
		float weight0 = recipW[0] * weight.x;
		float weight1 = recipW[1] * weight.y;
		float weight2 = recipW[2] * weight.z;
		float normlWeight = 1.0f / (weight0 + weight1 + weight2);

		float x = (texcoord[0].x * weight0 + texcoord[1].x * weight1 + texcoord[2].x * weight2) * normlWeight;
		float y = (texcoord[0].y * weight0 + texcoord[1].y * weight1 + texcoord[2].y * weight2) * normlWeight;

		return { x, y };
	}

	void Graphics::rasterizeTriangle(const Shader::VertexData& vertex0, const Shader::VertexData& vertex1, const Shader::VertexData& vertex2)
	{
		auto v0 = glm::ivec2(vertex0.screenPosition.x, vertex0.screenPosition.y);
		auto v1 = glm::ivec2(vertex1.screenPosition.x, vertex1.screenPosition.y);
		auto v2 = glm::ivec2(vertex2.screenPosition.x, vertex2.screenPosition.y);

		int min_x = std::max(std::min(v0.x, std::min(v1.x, v2.x)), 0);
		int min_y = std::max(std::min(v0.y, std::min(v1.y, v2.y)), 0);
		int max_x = std::min(std::max(v0.x, std::max(v1.x, v2.x)), mWidth-1);
		int max_y = std::min(std::max(v0.y, std::max(v1.y, v2.y)), mHeight-1);

		//Strict barycenteric weights calculation
		auto barycentericWeight = [&](const float& x, const float& y) -> glm::vec3
		{
			glm::vec3 s[2];
			s[0] = glm::vec3(vertex2.screenPosition.x - vertex0.screenPosition.x, vertex1.screenPosition.x - vertex0.screenPosition.x, vertex0.screenPosition.x - x);
			s[1] = glm::vec3(vertex2.screenPosition.y - vertex0.screenPosition.y, vertex1.screenPosition.y - vertex0.screenPosition.y, vertex0.screenPosition.y - y);
			auto uf = glm::cross(s[0], s[1]);
			return glm::vec3(1.f - (uf.x + uf.y) / uf.z, uf.y / uf.z, uf.x / uf.z);
		};

		auto barycenterWeight1 = [&](const float& x, const float& y) -> glm::vec3
		{
			float x0 = vertex0.screenPosition.x;
			float x1 = vertex1.screenPosition.x;
			float x2 = vertex2.screenPosition.x;
			float y0 = vertex0.screenPosition.y;
			float y1 = vertex1.screenPosition.y;
			float y2 = vertex2.screenPosition.y;

			
			float a = (-(x - x1) * (y2 - y1) + (y - y1) * (x2 - x1)) / (-(x0 - x1) * (y2 - y1) + (y0 - y1) * (x2 - x1));
			float b = (-(x - x2) * (y0 - y2) + (y - y2) * (x0 - x2)) / (-(x1 - x2) * (y0 - y2) + (y1 - y2) * (x0 - x2));
			float c = 1 - a - b;

			return glm::vec3(a, b, c);
		};

		for (int x = min_x; x <= max_x; ++x)
		{
			for (int y = min_y; y <= max_y; ++y)
			{
				glm::vec2 v = { x, y };
				bool insied = true;
				insied &= edgeFunction(v0, v1, v);
				insied &= edgeFunction(v1, v2, v);
				insied &= edgeFunction(v2, v0, v);
				if (insied)
				{
					auto weight = barycentericWeight(x, y);
					auto weight1 = barycenterWeight1(x, y);
					auto v = Shader::VertexData::barycentricLerp(vertex0, vertex1, vertex2, weight);

					std::array<glm::vec2, 3> texcoords = {vertex0.texcoord, vertex1.texcoord, vertex2.texcoord};
					std::array<float, 3> reclipW = { vertex0.clipW, vertex1.clipW, vertex2.clipW };

					//v.texcoord = interpolateTexcoord(texcoords, weight, reclipW);
					v.texcoord /= v.clipW;

					//mBackBuffer->writeColor(x, y, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
					mBackBuffer->writeColor(x, y, mShader->fragmentShader(v));
				}
			}
		}
	}
}


