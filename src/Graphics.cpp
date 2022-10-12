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

		// create back buffer
		mFrontBuffer = std::make_shared<FrameBuffer>(width, height);
		mBackBuffer = std::make_shared<FrameBuffer>(width, height);

		// set viewport
		mViewport.x = 0;
		mViewport.y = 0;
		mViewport.width  = (float)width;
		mViewport.height = (float)height;

		mShader = std::make_shared<Shader>();

		std::string a = "Cube_BaseColor.png";
		Image::Ptr image = Image::create(IMAGE_DIR + a);

		


		Texture* texture = new Texture();
		texture->initFromImage(image);
		mShader->mUniforms.albedoMap.bindTexture(texture);

		//Texture2D::Ptr albedo = std::make_shared<Texture2D>();
		//albedo->initFromImage(image);
		//mShader->mUniforms.albedo = albedo;
	}

	void Graphics::clear(float r, float g, float b, float a)
	{
		mFrontBuffer->clearColor(r, g, b, a);
		mBackBuffer->clearColor(r, g, b, a);
	}

	void Graphics::drawTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2)
	{
		Shader::VertexData vd0, vd1, vd2;
		vd0 = mShader->vertexShader(v0);
		vd1 = mShader->vertexShader(v1);
		vd2 = mShader->vertexShader(v2);

		glm::vec3 ndcPosition0 = perspectiveDivide(vd0.clipPostion);
		glm::vec3 ndcPosition1 = perspectiveDivide(vd1.clipPostion);
		glm::vec3 ndcPosition2 = perspectiveDivide(vd2.clipPostion);

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

		rasterizeTriangle3(vd0, vd1, vd2);
	}

	void Graphics::drawMesh(const Mesh* mesh)
	{
		for (const auto subMesh : mesh->subMeshs)
		{
            if (subMesh->indices.empty() || subMesh->vertices.empty())
                return;

            for (int i = 0; i < subMesh->indices.size(); i += 3)
            {
                Vertex v0, v1, v2;
                v0 = subMesh->vertices[subMesh->indices[i]];
                v1 = subMesh->vertices[subMesh->indices[i + 1]];
                v2 = subMesh->vertices[subMesh->indices[i + 2]];

                drawTriangle(v0, v1, v2);
            }
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

	glm::vec3 Graphics::perspectiveDivide(const glm::vec4& pos)
	{
		float invW = 1.0f / pos.w;
        float x = pos.x * invW;
        float y = pos.y * invW;
        float z = pos.z * invW;

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

	void Graphics::scanLine(const Shader::VertexData& left, const Shader::VertexData& right)
	{
		int length = right.screenPosition.x - left.screenPosition.x + 1;
		for (int i = 0; i <= length; i++)
		{
			Shader::VertexData v = Shader::VertexData::lerp(left, right, static_cast<float>(i / length));
			v.screenPosition.x = left.screenPosition.x + i;
			v.screenPosition.y = left.screenPosition.y;
			mBackBuffer->writeColor(v.screenPosition.x, v.screenPosition.y, glm::vec4(1.0, 1.0, 1.0, 1.0));
		}
		
	}

	void Graphics::rasterizeTopTriangle(const Shader::VertexData& v0, const Shader::VertexData& v1, const Shader::VertexData& v2)
	{
		Shader::VertexData top, left, right;
		Shader::VertexData newLeft, newRight;
		left  = v1.screenPosition.x > v2.screenPosition.x ? v2 : v1;
		right = v1.screenPosition.x > v2.screenPosition.x ? v1 : v2;
		top   = v0;

		int dy = top.screenPosition.y - left.screenPosition.y + 1;
		int nowY = top.screenPosition.y;

		for (int i = dy; i >= 0; --i)
		{
			float weight = 0;
			if(dy != 0)
				weight =  (float) i/ dy;
			newLeft  = Shader::VertexData::lerp(left, top, weight);
			newRight = Shader::VertexData::lerp(right, top, weight);
			newLeft.screenPosition.y = newRight.screenPosition.y = nowY;
			scanLine(newLeft, newRight);
			nowY--;
		}
	}

	void Graphics::rasterizeBottomTriangle(const Shader::VertexData& v0, const Shader::VertexData& v1, const Shader::VertexData& v2)
	{
		Shader::VertexData left, right, bottom;
		Shader::VertexData newLeft, newRight;
		left  = v1.screenPosition.x > v2.screenPosition.x ? v2 : v1;
		right = v1.screenPosition.x > v2.screenPosition.x ? v1 : v2;
		bottom = v0;

		int dy = left.screenPosition.y - bottom.screenPosition.y + 1;
		int nowY = left.screenPosition.y;

		for (int i = 0; i < dy; ++i)
		{
			float weight = 0;
			if(dy != 0)
				weight = (float) i / dy;
			newLeft  = Shader::VertexData::lerp(left, bottom, weight);
			newRight = Shader::VertexData::lerp(right, bottom, weight);
			newLeft.screenPosition.y = newRight.screenPosition.y = nowY;
			scanLine(newLeft, newRight);
			nowY--;
		}
	} 

	// Test whether two float or double numbers are equal.
	// ulp: units in the last place.
	template <typename T>
	typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
	isEqual(T x, T y, int ulp = 2) {
	// the machine epsilon has to be scaled to the magnitude of the values used
	// and multiplied by the desired precision in ULPs (units in the last place)
	return std::fabs(x - y) <
				std::numeric_limits<T>::epsilon() * std::fabs(x + y) * ulp
			// unless the result is subnormal
			|| std::fabs(x - y) < std::numeric_limits<T>::min();
	}


	void Graphics::rasterizeTriangle2(const Shader::VertexData& vertex0, const Shader::VertexData& vertex1, const Shader::VertexData& vertex2)
	{
		Shader::VertexData verts[3] = {vertex0, vertex1, vertex2};
		Shader::VertexData tmp;

		// sort verts's in axisY v0 < v1 < v2
		if(verts[0].screenPosition.y > verts[1].screenPosition.y)
		{
			tmp= verts[0];
			verts[0] = verts[1];
			verts[1] = tmp; 
		}
		if(verts[1].screenPosition.y > verts[2].screenPosition.y)
		{
			tmp = verts[1];
			verts[1] = verts[2];
			verts[2] = tmp;
		}
		if(verts[0].screenPosition.y > verts[1].screenPosition.y)
		{
			tmp = verts[0];
			verts[0] = verts[1];
			verts[1] = tmp;
		}

		if(isEqual(verts[1].screenPosition.y, verts[2].screenPosition.y))
		{
			rasterizeBottomTriangle(verts[0], verts[1], verts[2]);
		}
		else if(isEqual(verts[1].screenPosition.y, verts[0].screenPosition.y))
		{
			rasterizeTopTriangle(verts[2], verts[1], verts[0]);
		}
		else
		{
			float weight = (verts[2].screenPosition.y - verts[1].screenPosition.y) / (verts[2].screenPosition.y - verts[0].screenPosition.y);
			Shader::VertexData newVert = Shader::VertexData::lerp(verts[2], verts[0], weight);
			rasterizeTopTriangle(verts[2], verts[1], newVert);
			rasterizeBottomTriangle(verts[0], verts[1], newVert);
		}
	}

	void Graphics::rasterizeTriangle3(const Shader::VertexData& v0, const Shader::VertexData& v1, const Shader::VertexData& v2)
	{
			const glm::ivec2& A = glm::ivec2((int)(v0.screenPosition.x + 0.5f), (int)(v0.screenPosition.y + 0.5f));
			const glm::ivec2& B = glm::ivec2((int)(v1.screenPosition.x + 0.5f), (int)(v1.screenPosition.y + 0.5f));
			const glm::ivec2& C = glm::ivec2((int)(v2.screenPosition.x + 0.5f), (int)(v2.screenPosition.y + 0.5f));

			//float Z[3] = { v0.screenPosition.z, v1.screenPosition.z, v2.screenPosition.z };

			int minX = std::max(std::min(A.x, std::min(B.x, C.x)), 0);
			int minY = std::max(std::min(A.y, std::min(B.y, C.y)), 0);
			int maxX = std::min(std::max(A.x, std::max(B.x, C.x)), mWidth-1);
			int maxY = std::min(std::max(A.y, std::max(B.y, C.y)), mHeight-1);

			//I1 = Ay - By, I2 = By - Cy, I3 = Cy - Ay
			int I01 = A.y - B.y;
			int I02 = B.y - C.y;
			int I03 = C.y - A.y;
	
			//J1 = Bx - Ax, J2 = Cx - Bx, J3 = Ax - Cx 
			int J01 = B.x - A.x; 
			int J02 = C.x - B.x; 
			int J03 = A.x - C.x; 

            //K1 = AxBy - AyBx, K2 = BxCy - ByCx, K3 = CxAy - CyAx
            int K01 = A.x * B.y - A.y * B.x;
            int K02 = B.x * C.y - B.y * C.x;
            int K03 = C.x * A.y - C.y * A.x;

            //F1 = I1 * Px + J1 * Py + K1
            //F2 = I2 * Px + J2 * Py + K2
			//F3 = I3 * Px + J3 * Py + k3
            int F01 = (I01 * minX) + (J01 * minY) + K01;
            int F02 = (I02 * minX) + (J02 * minY) + K02;
            int F03 = (I03 * minX) + (J03 * minY) + K03;

			// Area = 1/2 * |AB| * |AC| * sin(|AB|, |AC|) = 1/2 * (AxBy - AyBx + BxCy - ByCx + CxAy - CyAx) = F1 + F2 + F3
			int delta = F01 + F02 + F03;

            //Degenerated to a line or a point
            if (delta <= 0)
                return;
           
            float oneDivideDelta = 1 / (float)delta;

            //Z[1] = (Z[1] - Z[0]) * OneDivideDelta;
	        //Z[2] = (Z[2] - Z[0]) * OneDivideDelta;


            int Cy1 = F01, Cy2 = F02, Cy3 = F03;

            for(int y = minY; y < maxY; ++y)
            {
                int Cx1 = Cy1;
                int Cx2 = Cy2;
                int Cx3 = Cy3;
                
                //float depth = Z[0] + Cx3 * Z[1] + Cx1 * Z[2];

                for(int x = minX; x < maxX; ++x)
                {
					const int mask = Cx1 | Cx2 | Cx3;
                    if(mask >= 0)
                    {
						glm::vec3 weights = glm::vec3(Cx2 * oneDivideDelta, Cx3 * oneDivideDelta, Cx1 * oneDivideDelta);

						auto v = Shader::VertexData::barycentricLerp(v0, v1, v2, weights);

						std::array<glm::vec2, 3> texcoords = { v0.texcoord, v1.texcoord, v2.texcoord };
						std::array<float, 3> reclipW = { v0.clipW, v1.clipW, v2.clipW };

						v.texcoord = interpolateTexcoord(texcoords, weights, reclipW);
						v.texcoord /= v.clipW;

						//mBackBuffer->writeColor(x, y, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
						mBackBuffer->writeColor(x, y, mShader->fragmentShader(v));

						//mBackBuffer->WriteDepth(x, y, depth);
                    }

                    Cx1 += I01;
                    Cx2 += I02;
                    Cx3 += I03;
                }

                Cy1 += J01;
                Cy2 += J02;
                Cy3 += J03;
            }
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

	void Graphics::drawLine(const glm::vec2& v0, const glm::vec2& v1)
	{
        int32_t x0 = (int32_t)v0.x;
        int32_t y0 = (int32_t)v0.y;
        int32_t x1 = (int32_t)v1.x;
        int32_t y1 = (int32_t)v1.y;

		bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);

		//check line's slop
		if (steep)
		{
			std::swap(x0, y0);
			std::swap(x1, y1);
		}

		//check line's direction
		if (x0 > x1)
		{
			std::swap(x0, x1);
			std::swap(y0, y1);
		}

		int32_t dx = x1 - x0;
		int32_t dy = y1 - y0;

		int32_t d2x = dx << 1;
		int32_t d2y = dy << 1; 
		int32_t d2xd2y = d2x - d2y;

		int32_t x = x0;
		int32_t y = y0;

		int32_t p = dx - d2y;
		for (int32_t i = 0; i <= dx; ++i)
		{
			if (steep)
			{
				mBackBuffer->writeColor(y, x, glm::vec4(1.0, 1.0, 1.0, 1.0));
			}
			else {
				mBackBuffer->writeColor(x, y, glm::vec4(1.0, 1.0, 1.0, 1.0));
			}

			if (p <= 0)
			{
				p += d2xd2y;
				y += 1;
			}
			else
			{
				p -= d2y; 
			}
			x += 1;
		}
	}
}


