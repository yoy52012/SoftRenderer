#pragma once
#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace SoftRenderer
{
	class Image : public std::enable_shared_from_this<Image>
	{
	public:
		using Ptr = std::shared_ptr<Image>;

        enum SaveFormat
        {
            PNG,
            JPG,
            BMP,
            TGA,
        };

        enum PixelFormat {
            PF_L8, //luminance
            PF_LA8, //luminance-alpha
            PF_R8,
            PF_RG88,
            PF_RGB888,
            PF_RGBA8888,
            PF_RGBA4444,
            PF_RGB565,
            PF_R32F, //float
            PF_RG32F,
            PF_RGB32F,
            PF_RGBA32F,
			PF_R16H,
            PF_RG16H,
            PF_RGB16H,
            PF_RGBA16H,
        };

		static Image::Ptr create(const std::string& filename);

		static Image::Ptr create(int32_t width, int32_t height, PixelFormat format);

		static Image::Ptr create(int32_t width, int32_t height, PixelFormat format, const std::vector<uint8_t>& buffer);

		/**
         * create an empty image
         */
		Image();

		/**
		 * create an empty image of a specific size and format
		 */
		Image(int32_t width, int32_t height,  PixelFormat format);

		/**
		 * create an image of a specific size and format from a pointer
		 */
		Image(int32_t width, int32_t height,  PixelFormat format, const std::vector<uint8_t>& buffer);

        Image(const Image& image);

        Image& operator=(const Image& image);

        ~Image();

		std::shared_ptr<Image> getSharedFromThis() { return shared_from_this(); }

        void init(int32_t width, int32_t height, PixelFormat format);

        void init(int32_t width, int32_t height, PixelFormat format, const std::vector<uint8_t>& buffer);

		bool load(const std::string& filename);

		bool save(const std::string& filename, SaveFormat saveformat);

		void release();

		/**
		 * Get image width
		 */
		int32_t getWidth() const { return mWidth; }

		/**
		 * Get image height
		 */
		int32_t getHeight() const { return mHeight; }

		/**
		 * Get image component
		 */
		int32_t getComponent() const { return getPixelFormatChannle(mPixelFormat); }

		/**
		 * Get the current image format.
		 */
		PixelFormat getFormat() const { return mPixelFormat; }

		/**
         * Convert the image to another format, conversion only to raw byte format
         */
		void convert(PixelFormat newFormat);

		glm::vec4 getPixel(int32_t x, int32_t y) const;

		void setPixel(int32_t x, int32_t y, const glm::vec4& pixel);

		const uint8_t* getData() const { return mData; };

		int32_t getDataSize() const { return mByteSize;};


	private:
        void copyFrom(const Image& image);


	public:
		static int32_t getPixelFormatByteSize(PixelFormat format);

		static int32_t getPixelFormatChannle(PixelFormat format);

		static int32_t calculateByteSize(int32_t width, int32_t height, PixelFormat format);

	private:
		int32_t mWidth = 0;
		int32_t mHeight = 0;
		int32_t mByteSize = 0;

		PixelFormat mPixelFormat = PixelFormat::PF_L8;
		uint8_t* mData = nullptr;

	};

}