#include "Texture.h"
#include "MathUtils.h"

namespace SoftRenderer
{
	Texture2D::Ptr Texture2D::create(const std::string& filename)
	{
		return Texture2D::Ptr();
	}

	Texture2D::Texture2D()
	{

	}

	Texture2D::Texture2D(int width, int height)
	{
		init(width, height);
	}

	Texture2D::~Texture2D()
	{
	}

	void Texture2D::init(int width, int height)
	{
		mWidth = width;
		mHeight = height;
		
		glm::vec4 texel(0.0f, 0.0f, 0.0f, 1.0f);
		mBuffer.resize(width * height);
		std::fill(mBuffer.begin(), mBuffer.end(), texel);
	}

	void Texture2D::initFromImage(Image::Ptr image)
	{
		const int width = image->getWidth();
		const int height = image->getHeight();
		const int channle = image->getChannle();
		const auto format = image->getFormat();

		const int size = width * height;

		mWidth = width;
		mHeight = height;
		mBuffer.resize(width * height);

		if (image->getFormat() == ImageFormat::FORMAT_HDR)
		{
			for (int i = 0; i < size; ++i)
			{
				float* pixel = &image->getHdrData()[i * channle];
				glm::vec4 texel(0.0f, 0.0f, 0.0f, 1.0f);
				if (channle == 1)
				{
					texel.x = texel.y = texel.z = pixel[0];
				}
				else if (channle == 2)
				{
					texel.x = texel.y = texel.z = pixel[0];
					texel.w = pixel[1];
				}
				else if (channle == 3)
				{
					texel.x = pixel[0];
					texel.y = pixel[1];
					texel.z = pixel[2];
				}
				else
				{
					texel.x = pixel[0];
					texel.y = pixel[1];
					texel.z = pixel[2];
					texel.w = pixel[3];
				}
				mBuffer[i] = texel;
			}
		}
		else if (image->getFormat() == ImageFormat::FORMAT_LDR)
		{
			for (int i = 0; i < size; ++i)
			{
				unsigned char* pixel = &image->getLdrData()[i * channle];
				glm::vec4 texel(0.0f, 0.0f, 0.0f, 1.0f);
				if (channle == 1)
				{
					texel.x = texel.y = texel.z = MathUtils::uchar2float(pixel[0]);
				}
				else if (channle == 2)
				{
					texel.x = texel.y = texel.z = MathUtils::uchar2float(pixel[0]);
					texel.w = MathUtils::uchar2float(pixel[1]);
				}
				else if (channle == 3)
				{
					texel.x = MathUtils::uchar2float(pixel[0]);
					texel.y = MathUtils::uchar2float(pixel[1]);
					texel.z = MathUtils::uchar2float(pixel[2]);
				}
				else
				{
					texel.x = MathUtils::uchar2float(pixel[0]);
					texel.y = MathUtils::uchar2float(pixel[1]);
					texel.z = MathUtils::uchar2float(pixel[2]);
					texel.w = MathUtils::uchar2float(pixel[3]);
				}
				mBuffer[i] = texel;
			}
		}
	}

	void Texture2D::initFromFile(const std::string& filename)
	{
		auto image = Image::create(filename);
		if (image != nullptr)
		{
			initFromImage(image);
		}
	}

	void Texture2D::initFromColorBuffer(FrameBuffer::Ptr framebuffer)
	{
		mWidth = framebuffer->getWidth();
		mHeight = framebuffer->getHeight();

		const int size = mWidth * mHeight;
		mBuffer.resize(size);
		for (int i = 0; i < size; i++)
		{
			unsigned char* color = &framebuffer->getColorBuffer()[i * 4];
			float r = MathUtils::uchar2float(color[0]);
			float g = MathUtils::uchar2float(color[1]);
			float b = MathUtils::uchar2float(color[2]);
			float a = MathUtils::uchar2float(color[3]);
			mBuffer[i] = glm::vec4(r, g, b, a);
		}
	}

	void Texture2D::initFromDepthBuffer(FrameBuffer::Ptr framebuffer)
	{
		mWidth = framebuffer->getWidth();
		mHeight = framebuffer->getHeight();

		const int size = mWidth * mHeight;
		mBuffer.resize(size);
		for (int i = 0; i < size; i++)
		{
			float depth = framebuffer->getDepthBuffer()[i];
			mBuffer[i] = glm::vec4(depth, depth, depth, 1.0f);
		}
	}

	glm::vec4 Texture2D::sample(const glm::vec2& texcoord)
	{
		return glm::vec4();
	}
	
	glm::vec4 Texture2D::sampleRepeat(const glm::vec2& texcoord)
	{
		float u = texcoord.x - std::floor(texcoord.x);
		float v = texcoord.y - std::floor(texcoord.y);
		int c = static_cast<int>((mWidth - 1) * u);
		int r = static_cast<int>((mHeight - 1) * v);
		int index = r * mWidth + c;
		return mBuffer[index];
	}

	glm::vec4 Texture2D::sampleClamp(const glm::vec2& texcoord)
	{
		auto clamp = [](float f) -> float {
			return f < 0 ? 0 : (f > 1 ? 1 : f);
		};

		float u = clamp(texcoord.x);
		float v = clamp(texcoord.y);
		int c = static_cast<int>((mWidth - 1) * u);
		int r = static_cast<int>((mHeight - 1) * v);
		int index = r * mWidth + c;
		return mBuffer[index];
	}
}