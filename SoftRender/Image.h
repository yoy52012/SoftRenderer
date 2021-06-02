#pragma once
#include <memory>
#include <string>

namespace SoftRenderer
{
	enum class ImageFormat
	{
		FORMAT_UNKNOWN,
		FORMAT_HDR,
		FORMAT_LDR
	};

	class Image
	{
	public:
		using Ptr = std::shared_ptr<Image>;

		static Image::Ptr create(const std::string& filename);

		static Image::Ptr create(int width, int height, int channle, ImageFormat format);


		void init(int width, int height, int channle, ImageFormat format);

		bool initFromFile(const std::string& filename);

		void release();

		ImageFormat getFormat() const
		{
			return mFormat;
		}

		int getWidth() const
		{
			return mWidth;
		}

		int getHeight() const
		{
			return mHeight;
		}

		int getChannle() const
		{
			return mChannle;
		}

		float* getHdrData() const
		{
			return mHdrData;
		}

		unsigned char* getLdrData() const
		{
			return mLdrData;
		}

		Image();
		Image(int width, int height, int channle, ImageFormat format);
		Image(const Image& image);
		Image& operator=(const Image& image);

		~Image();

	private:
		int mWidth = 0;
		int mHeight = 0;
		int mChannle = 0;
		ImageFormat mFormat = ImageFormat::FORMAT_UNKNOWN;

		unsigned char* mLdrData = nullptr;
		float* mHdrData = nullptr;

	};

}