#include "Texture.h"
//#include "Texture.h"
//#include "MathUtils.h"
//
//namespace SoftRenderer
//{
//	Texture2D::Ptr Texture2D::create(const std::string& filename)
//	{
//		return Texture2D::Ptr();
//	}
//
//	Texture2D::Texture2D()
//	{
//
//	}
//
//	Texture2D::Texture2D(int width, int height)
//	{
//		init(width, height);
//	}
//
//	Texture2D::~Texture2D()
//	{
//	}
//
//	void Texture2D::init(int width, int height)
//	{
//		mWidth = width;
//		mHeight = height;
//		
//		glm::vec4 texel(0.0f, 0.0f, 0.0f, 1.0f);
//		mBuffer.resize(width * height);
//		std::fill(mBuffer.begin(), mBuffer.end(), texel);
//	}
//
//	void Texture2D::initFromImage(Image::Ptr image)
//	{
//		const int width = image->getWidth();
//		const int height = image->getHeight();
//		const int channle = image->getChannle();
//		const auto format = image->getFormat();
//
//		const int size = width * height;
//
//		mWidth = width;
//		mHeight = height;
//		mBuffer.resize(width * height);
//
//		if (image->getFormat() == ImageFormat::FORMAT_HDR)
//		{
//			for (int i = 0; i < size; ++i)
//			{
//				float* pixel = &image->getHdrData()[i * channle];
//				glm::vec4 texel(0.0f, 0.0f, 0.0f, 1.0f);
//				if (channle == 1)
//				{
//					texel.x = texel.y = texel.z = pixel[0];
//				}
//				else if (channle == 2)
//				{
//					texel.x = texel.y = texel.z = pixel[0];
//					texel.w = pixel[1];
//				}
//				else if (channle == 3)
//				{
//					texel.x = pixel[0];
//					texel.y = pixel[1];
//					texel.z = pixel[2];
//				}
//				else
//				{
//					texel.x = pixel[0];
//					texel.y = pixel[1];
//					texel.z = pixel[2];
//					texel.w = pixel[3];
//				}
//				mBuffer[i] = texel;
//			}
//		}
//		else if (image->getFormat() == ImageFormat::FORMAT_LDR)
//		{
//			for (int i = 0; i < size; ++i)
//			{
//				unsigned char* pixel = &image->getLdrData()[i * channle];
//				glm::vec4 texel(0.0f, 0.0f, 0.0f, 1.0f);
//				if (channle == 1)
//				{
//					texel.x = texel.y = texel.z = MathUtils::uchar2float(pixel[0]);
//				}
//				else if (channle == 2)
//				{
//					texel.x = texel.y = texel.z = MathUtils::uchar2float(pixel[0]);
//					texel.w = MathUtils::uchar2float(pixel[1]);
//				}
//				else if (channle == 3)
//				{
//					texel.x = MathUtils::uchar2float(pixel[0]);
//					texel.y = MathUtils::uchar2float(pixel[1]);
//					texel.z = MathUtils::uchar2float(pixel[2]);
//				}
//				else
//				{
//					texel.x = MathUtils::uchar2float(pixel[0]);
//					texel.y = MathUtils::uchar2float(pixel[1]);
//					texel.z = MathUtils::uchar2float(pixel[2]);
//					texel.w = MathUtils::uchar2float(pixel[3]);
//				}
//				mBuffer[i] = texel;
//			}
//		}
//	}
//
//	void Texture2D::initFromFile(const std::string& filename)
//	{
//		auto image = Image::create(filename);
//		if (image != nullptr)
//		{
//			initFromImage(image);
//		}
//	}
//
//	void Texture2D::initFromColorBuffer(FrameBuffer::Ptr framebuffer)
//	{
//		mWidth = framebuffer->getWidth();
//		mHeight = framebuffer->getHeight();
//
//		const int size = mWidth * mHeight;
//		mBuffer.resize(size);
//		for (int i = 0; i < size; i++)
//		{
//			unsigned char* color = &framebuffer->getColorBuffer()[i * 4];
//			float r = MathUtils::uchar2float(color[0]);
//			float g = MathUtils::uchar2float(color[1]);
//			float b = MathUtils::uchar2float(color[2]);
//			float a = MathUtils::uchar2float(color[3]);
//			mBuffer[i] = glm::vec4(r, g, b, a);
//		}
//	}
//
//	void Texture2D::initFromDepthBuffer(FrameBuffer::Ptr framebuffer)
//	{
//		mWidth = framebuffer->getWidth();
//		mHeight = framebuffer->getHeight();
//
//		const int size = mWidth * mHeight;
//		mBuffer.resize(size);
//		for (int i = 0; i < size; i++)
//		{
//			float depth = framebuffer->getDepthBuffer()[i];
//			mBuffer[i] = glm::vec4(depth, depth, depth, 1.0f);
//		}
//	}
//
//	glm::vec4 Texture2D::sample(const glm::vec2& texcoord)
//	{
//		return glm::vec4();
//	}
//	
//	glm::vec4 Texture2D::sampleRepeat(const glm::vec2& texcoord)
//	{
//		float u = texcoord.x - std::floor(texcoord.x);
//		float v = texcoord.y - std::floor(texcoord.y);
//		int c = static_cast<int>((mWidth - 1) * u);
//		int r = static_cast<int>((mHeight - 1) * v);
//		int index = r * mWidth + c;
//		return mBuffer[index];
//	}
//
//	glm::vec4 Texture2D::sampleClamp(const glm::vec2& texcoord)
//	{
//		auto clamp = [](float f) -> float {
//			return f < 0 ? 0 : (f > 1 ? 1 : f);
//		};
//
//		float u = clamp(texcoord.x);
//		float v = clamp(texcoord.y);
//		int c = static_cast<int>((mWidth - 1) * u);
//		int r = static_cast<int>((mHeight - 1) * v);
//		int index = r * mWidth + c;
//		return mBuffer[index];
//	}
//}


namespace SoftRenderer
{
    static int roundupPowerOf2(int n)
    {
        static const int max = 1 << 30;
        n -= 1;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        return n < 0 ? 1 : (n >= max ? max : n + 1);
    }

    void Texture::generateMipmaps()
    {
        if (mMipmapReadyFlag || mMipmapGeneratingFlag)
        {
            return;
        }

        auto generatePo2Mipmap = [](TextureBuffer<glm::vec4>* inBuffer, TextureBuffer<glm::vec4>* outBuffer) -> void
        {
            const float ratioX = (float)inBuffer->getWidth() / (float)outBuffer->getWidth();
            const float ratioY = (float)inBuffer->getHeight() / (float)outBuffer->getHeight();

            for (uint32_t y = 0; y < outBuffer->getHeight(); ++y)
            {
                for (uint32_t x = 0; x < outBuffer->getWidth(); ++x)
                {
                    glm::vec2 uv = glm::vec2((float)x * ratioX, (float)y * ratioY);

                    const int32_t ix = (int32_t)uv.x;
                    const int32_t iy = (int32_t)uv.y;

                    glm::vec2 f = glm::fract(glm::vec2(x, y));

                    auto p0 = Sampler::sampleBufferWithWrapMode(inBuffer, ix, iy, WrapMode::WRAP_CLAMP_TO_EDGE);
                    auto p1 = Sampler::sampleBufferWithWrapMode(inBuffer, ix + 1, iy, WrapMode::WRAP_CLAMP_TO_EDGE);
                    auto p2 = Sampler::sampleBufferWithWrapMode(inBuffer, ix, iy + 1, WrapMode::WRAP_CLAMP_TO_EDGE);
                    auto p3 = Sampler::sampleBufferWithWrapMode(inBuffer, ix + 1, iy + 1, WrapMode::WRAP_CLAMP_TO_EDGE);


                    auto pixel = glm::mix(glm::mix(p0, p1, f.x), glm::mix(p2, p3, f.x), f.y);

                    outBuffer->set(x, y, pixel);
                }
            }
        };

        //mMipmapThread = std::make_shared<std::thread>([&]() 

        {
            mMipmapGeneratingFlag = true;

            mMipmaps.clear();

            const uint32_t originWidth = mBuffer->getWidth();
            const uint32_t originHeight = mBuffer->getHeight();

            // alloc memory for mip 0
            uint32_t mipSize = std::max(roundupPowerOf2(originWidth), roundupPowerOf2(originHeight));

            mMipmaps.emplace_back(createTextureBuffer(mipSize, mipSize));

            auto currentBufferPtr = mMipmaps.back().get();
            auto originBufferPtr  = mBuffer.get();

            if (mipSize == originBufferPtr->getWidth() && mipSize == originBufferPtr->getHeight())
            {
                originBufferPtr->copyTo(currentBufferPtr->getData());
            }
            else
            {
                generatePo2Mipmap(originBufferPtr, currentBufferPtr);
            }

            // other levels
            while (mipSize > 1)
            {
                mipSize /= 2;

                mMipmaps.emplace_back(createTextureBuffer(mipSize, mipSize));

                currentBufferPtr = mMipmaps.back().get();

                originBufferPtr = mMipmaps[mMipmaps.size() - 2].get();

                generatePo2Mipmap(originBufferPtr, currentBufferPtr);


                std::vector<uint8_t> out;
                for (size_t y = 0; y < mipSize; y++)
                {
                    for (size_t x = 0; x < mipSize; x++)
                    {
                        glm::vec4* pixel = currentBufferPtr->get(x, y);

                        out.push_back((*pixel).r);
                        out.push_back((*pixel).g);
                        out.push_back((*pixel).b);
                        out.push_back((*pixel).a);
                    }
                }

                auto outImage = Image::create(mipSize, mipSize, Image::PixelFormat::PF_RGBA8888, out);

                outImage->save("E:\\test.png", Image::PNG);

            }

            mMipmapGeneratingFlag = false;
            mMipmapReadyFlag = true;
        }
        //});
    }


    Texture2D::Texture2D(int32_t width, int32_t height)
        :Texture(width, height)
    {
    }

	const glm::vec4 Sampler::BORDER_COLOR(0);

	glm::vec4 Sampler::sampleBufferWithWrapMode(TextureBuffer<glm::vec4>* buffer, int32_t x, int32_t y, WrapMode wrapMode)
	{
		const int32_t width = buffer->getWidth();
		const int32_t height = buffer->getHeight();

		auto uvMod = [](int32_t i, int32_t n)
		{
			return (i & (n - 1) + n) & (n - 1); // (i % n + n) % n
		};

		auto uvMirror = [](int32_t i)
		{
			return i >= 0 ? i : (-1 - i);
		};

		int32_t sampleX = x;
		int32_t sampleY = y;
		switch (wrapMode)
		{
		case WrapMode::WRAP_REPEAT:
			sampleX = uvMod(x, width);
			sampleY = uvMod(y, height);
			break;
		case WrapMode::WRAP_MIRRORED_REPEAT:
			sampleX = uvMod(x, 2 * width);
			sampleY = uvMod(y, 2 * height);

			sampleX -= width;
			sampleY -= height;

			sampleX = uvMirror(sampleX);
			sampleY = uvMirror(sampleY);

			sampleX = width - 1 - sampleX;
			sampleY = height - 1 - sampleY;
			break;
		case WrapMode::WRAP_CLAMP_TO_EDGE:
			if (x < 0) sampleX = 0;
			if (y < 0) sampleY = 0;
			if (x >= width) sampleX = width - 1;
			if (y >= height) sampleY = height - 1;
			break;
		case WrapMode::WRAP_CLAMP_TO_BORDER:
			if (x < 0 || x >= width) return BORDER_COLOR;
			if (y < 0 || y >= height) return BORDER_COLOR;
			break;
		case WrapMode::WRAP_CLAMP_TO_ZERO:
			if (x < 0 || x >= width) return glm::vec4(0);
			if (y < 0 || y >= height) return glm::vec4(0);
			break;
		default:
			break;
		}

		glm::vec4* pixel = buffer->get(sampleX, sampleY);

		if (pixel)
		{
			return *pixel;
		}

		return glm::vec4(0);
	}

	glm::vec4 Sampler::sampleBufferNearest(TextureBuffer<glm::vec4>* buffer, const glm::vec2& uv, WrapMode wrapMode, const glm::ivec2& offset)
	{
		const int32_t x = (int32_t)(uv.x * (buffer->getWidth() - 1) + 0.5f) + offset.x;
		const int32_t y = (int32_t)(uv.y * (buffer->getHeight() - 1) + 0.5f) + offset.y;

		return sampleBufferWithWrapMode(buffer, x, y, wrapMode);
	}

	glm::vec4 Sampler::sampleBufferBilinear(TextureBuffer<glm::vec4>* buffer, const glm::vec2& uv, WrapMode wrapMode, const glm::ivec2& offset)
	{
		const float x = (uv.x * buffer->getWidth() - 0.5f) + offset.x;
		const float y = (uv.y * buffer->getHeight() - 0.5f) + offset.y;
		const int32_t ix = (int32_t)x;
		const int32_t iy = (int32_t)y;

		glm::vec2 f = glm::fract(glm::vec2(x, y));

		/*********************
		 *   p2--p3
		 *   |   |
		 *   p0--p1
		 * Note: p0 is (ix,iy)
		 ********************/

		auto p0 = sampleBufferWithWrapMode(buffer, ix, iy, wrapMode);
		auto p1 = sampleBufferWithWrapMode(buffer, ix + 1, iy, wrapMode);
		auto p2 = sampleBufferWithWrapMode(buffer, ix, iy + 1, wrapMode);
		auto p3 = sampleBufferWithWrapMode(buffer, ix + 1, iy + 1, wrapMode);


		return glm::mix(glm::mix(p0, p1, f.x), glm::mix(p2, p3, f.x), f.y);
	}


	glm::vec4 Sampler::sampleTexture(Texture* texture, const glm::vec2& uv, float lod, const glm::vec2& offset)
	{
		if (texture == nullptr || texture->isEmpty())
		{
			return glm::vec4(0);
		}

		const FilterMode filterMode = mFilterMode;
		const WrapMode   wrapMode = mWrapMode;

		if (filterMode == FilterMode::FILTER_NEAREST)
		{
			return sampleBufferNearest(texture->mBuffer.get(), uv, wrapMode, offset);
		}

		if (filterMode == FilterMode::FILTER_LINEAR)
		{
			return sampleBufferBilinear(texture->mBuffer.get(), uv, wrapMode, offset);
		}

		if (!texture->isMipmapsReady())
		{
			texture->generateMipmaps();

			if (filterMode == FilterMode::FILTER_NEAREST_MIPMAP_NEAREST || filterMode == FilterMode::FILTER_NEAREST_MIPMAP_LINEAR)
			{
				return sampleBufferNearest(texture->mBuffer.get(), uv, wrapMode, offset);
			}

			if (filterMode == FilterMode::FILTER_LINEAR_MIPMAP_NEAREST || filterMode == FilterMode::FILTER_LINEAR_MIPMAP_LINEAR)
			{
				return sampleBufferBilinear(texture->mBuffer.get(), uv, wrapMode, offset);
			}
		}

		const int32_t maxLevel = (int32_t)texture->mMipmaps.size() - 1;

		if (filterMode == FilterMode::FILTER_NEAREST_MIPMAP_NEAREST || filterMode == FilterMode::FILTER_LINEAR_MIPMAP_NEAREST)
		{
			int32_t level = glm::clamp((int32_t)std::round(lod), 0, maxLevel);

			if (filterMode == FilterMode::FILTER_NEAREST_MIPMAP_NEAREST)
			{
				return sampleBufferNearest(texture->mMipmaps[level].get(), uv, wrapMode, offset);
			}
			else
			{
				return sampleBufferBilinear(texture->mMipmaps[level].get(), uv, wrapMode, offset);
			}
		}

		if (filterMode == FilterMode::FILTER_NEAREST_MIPMAP_LINEAR || filterMode == FilterMode::FILTER_LINEAR_MIPMAP_LINEAR)
		{
			int32_t level1 = glm::clamp((int32_t)std::floor(lod), 0, maxLevel);
			int32_t level2 = glm::clamp((int32_t)std::ceil(lod), 0, maxLevel);

			glm::vec4 texel1, texel2;
			if (filterMode == FilterMode::FILTER_NEAREST_MIPMAP_LINEAR)
			{
				texel1 = sampleBufferNearest(texture->mMipmaps[level1].get(), uv, wrapMode, offset);
			}
			else
			{
				texel1 = sampleBufferBilinear(texture->mMipmaps[level1].get(), uv, wrapMode, offset);
			}

			if (level1 == level2)
			{
				return texel1;
			}
			else
			{
				if (filterMode == FilterMode::FILTER_NEAREST_MIPMAP_LINEAR)
				{
					texel2 = sampleBufferNearest(texture->mMipmaps[level2].get(), uv, wrapMode, offset);
				}
				else
				{
					texel2 = sampleBufferBilinear(texture->mMipmaps[level2].get(), uv, wrapMode, offset);
				}
			}

			float f = glm::fract(lod);

			return glm::mix(texel1, texel2, f);
		}

		return glm::vec4(0);
	}
}


