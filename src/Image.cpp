#include "Image.h"

#include <cstring>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>

namespace SoftRenderer
{
	Image::Image()
	{
	}

	Image::Image(int width, int height, int channle, ImageFormat format)
	{
		init(width, height, channle, format);
	}

	Image::Image(const Image& image)
		:mWidth(image.mWidth)
		,mHeight(image.mHeight)
		,mChannle(image.mChannle)
		,mFormat(image.mFormat)
	{
		int size = mWidth * mHeight * mChannle;
		init(mWidth, mHeight, mChannle, mFormat);
		if (mFormat == ImageFormat::FORMAT_HDR)
		{
			std::copy(image.mHdrData, image.mHdrData + size, mHdrData);
		}
		else if (mFormat == ImageFormat::FORMAT_LDR)
		{
			std::copy(image.mLdrData, image.mLdrData + size, mLdrData);
		}
	}

	Image& Image::operator=(const Image& image)
	{
		mWidth = image.mWidth;
		mHeight = image.mHeight;
		mChannle = image.mChannle;
		mFormat = image.mFormat;

		int size = mWidth * mHeight * mChannle;
		init(mWidth, mHeight, mChannle, mFormat);
		if (mFormat == ImageFormat::FORMAT_HDR)
		{
			std::copy(image.mHdrData, image.mHdrData + size, mHdrData);
		}
		else if (mFormat == ImageFormat::FORMAT_LDR)
		{
			std::copy(image.mLdrData, image.mLdrData + size, mLdrData);
		}

		return *this;
	}

	Image::~Image()
	{
		release();
	}

	Image::Ptr Image::create(int width, int height, int channle, ImageFormat format)
	{
		Image::Ptr image = std::make_shared<Image>(width, height, channle, format);
		return image;
	}

	Image::Ptr Image::create(const std::string& filename)
	{
		Image::Ptr image = std::make_shared<Image>();
		if (!image->initFromFile(filename))
		{
			return nullptr;
		}
		return image;
	}

	void Image::init(int width, int height, int channle, ImageFormat format)
	{
		int size = width * height * channle;
		mWidth = width;
		mHeight = height;
		mChannle = channle;
		if (format == ImageFormat::FORMAT_HDR)
		{
			mHdrData = (float*)std::malloc(size * sizeof(float));
			std::memset(mHdrData, size * sizeof(float), 0);
			mFormat = format;
		}
		else if (format == ImageFormat::FORMAT_LDR)
		{
			mLdrData = (unsigned char*)std::malloc(size * sizeof(unsigned char));
			std::memset(mLdrData, size * sizeof(unsigned char), 0);
			mFormat = format;
		}
	}


	bool Image::initFromFile(const std::string& filename)
	{
		if (filename.empty())
		{
			std::cout << "[ERROR] image filename =" << filename << " is empty!" << std::endl;
			return false;
		}

		const char* extension = std::strrchr(filename.c_str(), '.') + 1;
		if (std::strcmp(extension, "hdr") == 0)
		{
			mHdrData = stbi_loadf(filename.c_str(), &mWidth, &mHeight, &mChannle, 0);
			if (mHdrData == nullptr) {
				std::cout << "[ERROR] can not load image = " << filename << std::endl;
				return false;
			}
			mFormat = ImageFormat::FORMAT_HDR;

		}
		else {
			mLdrData = stbi_load(filename.c_str(), &mWidth, &mHeight, &mChannle, 0);
			if (mLdrData == nullptr) {
				std::cout << "[ERROR] can not load image = " << filename << std::endl;
				return false;
			}
			mFormat = ImageFormat::FORMAT_LDR;
		}

		return true;
	}

	void Image::release()
	{
		if (mHdrData) free(mHdrData); mHdrData = nullptr;
		if (mLdrData) free(mLdrData); mLdrData = nullptr;
		mWidth = 0;
		mHeight = 0;
		mChannle = 0;
		mFormat = ImageFormat::FORMAT_UNKNOWN;
	}

}