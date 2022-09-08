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

#if 0
    template <int C, class T>
    static void _scale_nearest(const uint8_t* __restrict src_data, uint8_t* __restrict dst_data, uint32_t src_width, uint32_t src_height, uint32_t dst_width, uint32_t dst_height)
    {
        for (uint32_t i = 0; i < dst_height; i++)
        {
            uint32_t src_yofs = i * src_height / dst_height;
            uint32_t y_ofs = src_yofs * src_width * C;

            for (uint32_t j = 0; j < dst_width; j++)
            {
                uint32_t src_xofs = j * src_width / dst_width;
                src_xofs *= C;

                for (uint32_t l = 0; l < C; l++)
                {
                    const T* src = ((const T*)src_data);
                    T* dst = ((T*)dst_data);

                    T p = src[y_ofs + src_xofs + l];
                    dst[i * dst_width * C + j * C + l] = p;
                }
            }
        }
    }

    template <int C, class T>
    static void _scale_bilinear(const uint8_t* __restrict src_data, uint8_t* __restrict dst_data, uint32_t src_width, uint32_t src_height, uint32_t dst_width, uint32_t dst_height)
    {
        const uint32_t FRAC_BITS = 8;
        const uint32_t FRAC_LEN = (1 << FRAC_BITS);
        const uint32_t FRAC_HALF = (FRAC_LEN >> 1);
        const uint32_t FRAC_MASK = FRAC_LEN - 1;

        for (uint32_t i = 0; i < dst_height; i++)
        {
            // Add 0.5 in order to interpolate based on pixel center
            uint32_t src_yofs_up_fp = (i + 0.5) * src_height * FRAC_LEN / dst_height;
            // Calculate nearest src pixel center above current, and truncate to get y index
            uint32_t src_yofs_up = src_yofs_up_fp >= FRAC_HALF ? (src_yofs_up_fp - FRAC_HALF) >> FRAC_BITS : 0;
            uint32_t src_yofs_down = (src_yofs_up_fp + FRAC_HALF) >> FRAC_BITS;
            if (src_yofs_down >= src_height)
            {
                src_yofs_down = src_height - 1;
            }
            // Calculate distance to pixel center of src_yofs_up
            uint32_t src_yofs_frac = src_yofs_up_fp & FRAC_MASK;
            src_yofs_frac = src_yofs_frac >= FRAC_HALF ? src_yofs_frac - FRAC_HALF : src_yofs_frac + FRAC_HALF;

            uint32_t y_ofs_up = src_yofs_up * src_width * C;
            uint32_t y_ofs_down = src_yofs_down * src_width * C;

            for (uint32_t j = 0; j < dst_width; j++)
            {
                uint32_t src_xofs_left_fp = (j + 0.5) * src_width * FRAC_LEN / dst_width;
                uint32_t src_xofs_left = src_xofs_left_fp >= FRAC_HALF ? (src_xofs_left_fp - FRAC_HALF) >> FRAC_BITS : 0;
                uint32_t src_xofs_right = (src_xofs_left_fp + FRAC_HALF) >> FRAC_BITS;
                if (src_xofs_right >= src_width)
                {
                    src_xofs_right = src_width - 1;
                }
                uint32_t src_xofs_frac = src_xofs_left_fp & FRAC_MASK;
                src_xofs_frac = src_xofs_frac >= FRAC_HALF ? src_xofs_frac - FRAC_HALF : src_xofs_frac + FRAC_HALF;

                src_xofs_left *= C;
                src_xofs_right *= C;

                for (uint32_t l = 0; l < C; l++)
                {
                    if (sizeof(T) == 1)
                    {   //uint8
                        uint32_t p00 = src_data[y_ofs_up + src_xofs_left + l] << FRAC_BITS;
                        uint32_t p10 = src_data[y_ofs_up + src_xofs_right + l] << FRAC_BITS;
                        uint32_t p01 = src_data[y_ofs_down + src_xofs_left + l] << FRAC_BITS;
                        uint32_t p11 = src_data[y_ofs_down + src_xofs_right + l] << FRAC_BITS;

                        uint32_t interp_up = p00 + (((p10 - p00) * src_xofs_frac) >> FRAC_BITS);
                        uint32_t interp_down = p01 + (((p11 - p01) * src_xofs_frac) >> FRAC_BITS);
                        uint32_t interp = interp_up + (((interp_down - interp_up) * src_yofs_frac) >> FRAC_BITS);
                        interp >>= FRAC_BITS;
                        dst_data[i * dst_width * C + j * C + l] = uint8_t(interp);
                    }
                    else if (sizeof(T) == 2)
                    {   //half float

                        //float xofs_frac = float(src_xofs_frac) / (1 << FRAC_BITS);
                        //float yofs_frac = float(src_yofs_frac) / (1 << FRAC_BITS);
                        //const T* src = ((const T*)src_data);
                        //T* dst = ((T*)dst_data);

                        //float p00 = Math::half_to_float(src[y_ofs_up + src_xofs_left + l]);
                        //float p10 = Math::half_to_float(src[y_ofs_up + src_xofs_right + l]);
                        //float p01 = Math::half_to_float(src[y_ofs_down + src_xofs_left + l]);
                        //float p11 = Math::half_to_float(src[y_ofs_down + src_xofs_right + l]);

                        //float interp_up = p00 + (p10 - p00) * xofs_frac;
                        //float interp_down = p01 + (p11 - p01) * xofs_frac;
                        //float interp = interp_up + ((interp_down - interp_up) * yofs_frac);

                        //dst[i * dst_width * C + j * C + l] = Math::make_half_float(interp);
                    }
                    else if (sizeof(T) == 4)
                    { //float

                        float xofs_frac = float(src_xofs_frac) / (1 << FRAC_BITS);
                        float yofs_frac = float(src_yofs_frac) / (1 << FRAC_BITS);
                        const T* src = ((const T*)src_data);
                        T* dst = ((T*)dst_data);

                        float p00 = src[y_ofs_up + src_xofs_left + l];
                        float p10 = src[y_ofs_up + src_xofs_right + l];
                        float p01 = src[y_ofs_down + src_xofs_left + l];
                        float p11 = src[y_ofs_down + src_xofs_right + l];

                        float interp_up = p00 + (p10 - p00) * xofs_frac;
                        float interp_down = p01 + (p11 - p01) * xofs_frac;
                        float interp = interp_up + ((interp_down - interp_up) * yofs_frac);

                        dst[i * dst_width * C + j * C + l] = interp;
                    }
                }
            }
        }
    }

    static double _bicubic_interp_kernel(double x)
    {
        x = std::abs(x);
        double bc = 0;

        if (x <= 1)
        {
            bc = (1.5 * x - 2.5) * x * x + 1;
        }
        else if (x < 2)
        {
            bc = ((-0.5 * x + 2.5) * x - 4) * x + 2;
        }

        return bc;
    }

    template <int C, class T>
    static void _scale_cubic(const uint8_t* __restrict src_data, uint8_t* __restrict dst_data, uint32_t src_width, uint32_t src_height, uint32_t dst_width, uint32_t dst_height)
    {
        // get source image size
        int width = src_width;
        int height = src_height;
        double xfac = (double)width / dst_width;
        double yfac = (double)height / dst_height;

        // coordinates of source points and coefficients
        double ox, oy, dx, dy, k1, k2;
        int ox1, oy1, ox2, oy2;

        // destination pixel values
        // width and height decreased by 1
        int ymax = height - 1;
        int xmax = width - 1;

        // temporary pointer

        for (uint32_t y = 0; y < dst_height; y++)
        {
            // Y coordinates
            oy = (double)y * yfac - 0.5f;
            oy1 = (int)oy;
            dy = oy - (double)oy1;

            for (uint32_t x = 0; x < dst_width; x++)
            {
                // X coordinates
                ox = (double)x * xfac - 0.5f;
                ox1 = (int)ox;
                dx = ox - (double)ox1;

                // initial pixel value
                T* __restrict dst = ((T*)dst_data) + (y * dst_width + x) * C;

                double color[C];
                for (int i = 0; i < C; i++)
                {
                    color[i] = 0;
                }

                for (int n = -1; n < 3; n++)
                {
                    // get Y coefficient
                    k1 = _bicubic_interp_kernel(dy - (double)n);

                    oy2 = oy1 + n;
                    if (oy2 < 0)
                    {
                        oy2 = 0;
                    }
                    if (oy2 > ymax)
                    {
                        oy2 = ymax;
                    }

                    for (int m = -1; m < 3; m++)
                    {
                        // get X coefficient
                        k2 = k1 * _bicubic_interp_kernel((double)m - dx);

                        ox2 = ox1 + m;
                        if (ox2 < 0)
                        {
                            ox2 = 0;
                        }
                        if (ox2 > xmax)
                        {
                            ox2 = xmax;
                        }

                        // get pixel of original image
                        const T* __restrict p = ((T*)src_data) + (oy2 * src_width + ox2) * C;

                        for (int i = 0; i < C; i++)
                        {
                            if (sizeof(T) == 2) { //half float
                                //color[i] = Math::half_to_float(p[i]);
                            }
                            else {
                                color[i] += p[i] * k2;
                            }
                        }
                    }
                }

                for (int i = 0; i < C; i++) {
                    if (sizeof(T) == 1) { //byte
                        dst[i] = clamp(std::round(color[i]), 0, 255);
                    }
                    else if (sizeof(T) == 2) { //half float
                        //dst[i] = Math::make_half_float(color[i]);
                    }
                    else {
                        dst[i] = color[i];
                    }
                }
            }
        }
    }

#define LANCZOS_TYPE 3
#define Math_PI 3.1415926535897932384626433833

    template<typename T>
    static T sinc(T p_x) { return p_x == 0 ? 1 : std::sin(p_x) / p_x; }

    template<typename T>
    static T sincn(T p_x) { return sinc((T)Math_PI * p_x); }

    template <typename T>
    T clamp(const T& n, const T& lower, const T& upper) {
        return std::max(lower, std::min(n, upper));
    }

    template <int C, class T>
    static void _scale_lanczos(const uint8_t* __restrict src_data, uint8_t* __restrict dst_data, uint32_t src_width, uint32_t src_height, uint32_t dst_width, uint32_t dst_height)
    {
        auto lanczos = [](float p_x)
        {
            return Math::abs(p_x) >= LANCZOS_TYPE ? 0 : sincn(p_x) * sincn(p_x / LANCZOS_TYPE);
        };

        uint32_t buffer_size = src_height * dst_width * C;
        float* buffer = new float[buffer_size]; // Store the first pass in a buffer 

        // FIRST PASS (horizontal)
        {
            float x_scale = float(src_width) / float(dst_width);

            float scale_factor = std::max(x_scale, 1.0); // A larger kernel is required only when downscaling
            int32_t half_kernel = LANCZOS_TYPE * scale_factor;

            float* kernel = new float[half_kernel * 2];

            for (int32_t buffer_x = 0; buffer_x < dst_width; buffer_x++)
            {
                // The corresponding point on the source image
                float src_x = (buffer_x + 0.5f) * x_scale; // Offset by 0.5 so it uses the pixel's center
                int32_t start_x = std::max(0, int32_t(src_x) - half_kernel + 1);
                int32_t end_x = std::min(src_width - 1, int32_t(src_x) + half_kernel);

                // Create the kernel used by all the pixels of the column
                for (int32_t target_x = start_x; target_x <= end_x; target_x++)
                {
                    kernel[target_x - start_x] = lanczos((target_x + 0.5f - src_x) / scale_factor);
                }

                for (int32_t buffer_y = 0; buffer_y < src_height; buffer_y++)
                {
                    float pixel[C] = { 0 };
                    float weight = 0;

                    for (int32_t target_x = start_x; target_x <= end_x; target_x++)
                    {
                        float lanczos_val = kernel[target_x - start_x];
                        weight += lanczos_val;

                        const T* __restrict src_data = ((const T*)src_data) + (buffer_y * src_width + target_x) * C;

                        for (uint32_t i = 0; i < C; i++)
                        {
                            if (sizeof(T) == 2) { //half float
                                //pixel[i] += Math::half_to_float(src_data[i]) * lanczos_val;
                            }
                            else
                            {
                                pixel[i] += src_data[i] * lanczos_val;
                            }
                        }
                    }

                    float* dst_data = ((float*)buffer) + (buffer_y * dst_width + buffer_x) * C;

                    for (uint32_t i = 0; i < C; i++)
                    {
                        dst_data[i] = pixel[i] / weight; // Normalize the sum of all the samples
                    }
                }
            }

            delete[] kernel;
        } // End of first pass

        // SECOND PASS (vertical + result)
        {
            float y_scale = float(src_height) / float(dst_height);

            float scale_factor = std::max(y_scale, 1);
            int32_t half_kernel = LANCZOS_TYPE * scale_factor;

            float* kernel = new float[half_kernel * 2];

            for (int32_t dst_y = 0; dst_y < dst_height; dst_y++)
            {
                float buffer_y = (dst_y + 0.5f) * y_scale;
                int32_t start_y = std::max(0, int32_t(buffer_y) - half_kernel + 1);
                int32_t end_y = std::min(src_height - 1, int32_t(buffer_y) + half_kernel);

                for (int32_t target_y = start_y; target_y <= end_y; target_y++) src_data
                {
                    kernel[target_y - start_y] = lanczos((target_y + 0.5f - buffer_y) / scale_factor);
                }

                for (int32_t dst_x = 0; dst_x < dst_width; dst_x++)
                {
                    float pixel[C] = { 0 };
                    float weight = 0;

                    for (int32_t target_y = start_y; target_y <= end_y; target_y++)
                    {
                        float lanczos_val = kernel[target_y - start_y];
                        weight += lanczos_val;

                        float* buffer_data = ((float*)buffer) + (target_y * dst_width + dst_x) * C;

                        for (uint32_t i = 0; i < C; i++)
                        {
                            pixel[i] += buffer_data[i] * lanczos_val;
                        }
                    }

                    T* dst_data = ((T*)dst_data) + (dst_y * dst_width + dst_x) * C;

                    for (uint32_t i = 0; i < C; i++)
                    {
                        pixel[i] /= weight;

                        if (sizeof(T) == 1) //byte
                        {
                            dst_data[i] = clamp(std::round(pixel[i]), 0, 255);
                        }
                        else if (sizeof(T) == 2) //half float
                        {
                            //dst_data[i] = Math::make_half_float(pixel[i]);
                        }
                        else // float
                        {
                            dst_data[i] = pixel[i];
                        }
                    }
                }
            }

            delete[] kernel;
        } // End of second pass

        delete[] buffer;
    }

    template <class Component, int CC, bool renormalize,
        void (*average_func)(Component&, const Component&, const Component&, const Component&, const Component&),
        void (*renormalize_func)(Component*)>
        static void _generate_po2_mipmap(const Component* p_src, Component* p_dst, uint32_t p_width, uint32_t p_height) {
        //fast power of 2 mipmap generation
        uint32_t dst_w = MAX(p_width >> 1, 1u);
        uint32_t dst_h = MAX(p_height >> 1, 1u);

        int right_step = (p_width == 1) ? 0 : CC;
        int down_step = (p_height == 1) ? 0 : (p_width * CC);

        for (uint32_t i = 0; i < dst_h; i++) {
            const Component* rup_ptr = &p_src[i * 2 * down_step];
            const Component* rdown_ptr = rup_ptr + down_step;
            Component* dst_ptr = &p_dst[i * dst_w * CC];
            uint32_t count = dst_w;

            while (count) {
                count--;
                for (int j = 0; j < CC; j++) {
                    average_func(dst_ptr[j], rup_ptr[j], rup_ptr[j + right_step], rdown_ptr[j], rdown_ptr[j + right_step]);
                }

                if (renormalize) {
                    renormalize_func(dst_ptr);
                }

                dst_ptr += CC;
                rup_ptr += right_step * 2;
                rdown_ptr += right_step * 2;
            }
        }
    }

    void Image::generateMipmaps()
    {
        const int32_t width = mWidth;
        const int32_t height = mHeight;
        const PixelFormat format = mFormat;

        int32_t mmcount;

        int32_t size = getByteSize(width, height, format, mmcount);

        data.resize(size);

        uint8_t* wp = data.ptrw();

        int prev_ofs = 0;
        int prev_h = height;
        int prev_w = width;

        for (int i = 1; i <= mmcount; i++) {
            int32_t offset, w, h;
            getMipmapOffsetAndSize(i, offset, w, h);

            switch (format)
            {
            case FORMAT_L8:
            case FORMAT_R8:
                _generate_po2_mipmap<uint8_t, 1, false, Image::average_4_uint8, Image::renormalize_uint8>(&wp[prev_ofs], &wp[ofs], prev_w, prev_h);
                break;
            case FORMAT_LA8:
            case FORMAT_RG8:
                _generate_po2_mipmap<uint8_t, 2, false, Image::average_4_uint8, Image::renormalize_uint8>(&wp[prev_ofs], &wp[ofs], prev_w, prev_h);
                break;
            case FORMAT_RGB8:
                if (p_renormalize) {
                    _generate_po2_mipmap<uint8_t, 3, true, Image::average_4_uint8, Image::renormalize_uint8>(&wp[prev_ofs], &wp[ofs], prev_w, prev_h);
                }
                else {
                    _generate_po2_mipmap<uint8_t, 3, false, Image::average_4_uint8, Image::renormalize_uint8>(&wp[prev_ofs], &wp[ofs], prev_w, prev_h);
                }

                break;
            case FORMAT_RGBA8:
                if (p_renormalize) {
                    _generate_po2_mipmap<uint8_t, 4, true, Image::average_4_uint8, Image::renormalize_uint8>(&wp[prev_ofs], &wp[ofs], prev_w, prev_h);
                }
                else {
                    _generate_po2_mipmap<uint8_t, 4, false, Image::average_4_uint8, Image::renormalize_uint8>(&wp[prev_ofs], &wp[ofs], prev_w, prev_h);
                }
                break;
            case FORMAT_RF:
                _generate_po2_mipmap<float, 1, false, Image::average_4_float, Image::renormalize_float>(reinterpret_cast<const float*>(&wp[prev_ofs]), reinterpret_cast<float*>(&wp[ofs]), prev_w, prev_h);
                break;
            case FORMAT_RGF:
                _generate_po2_mipmap<float, 2, false, Image::average_4_float, Image::renormalize_float>(reinterpret_cast<const float*>(&wp[prev_ofs]), reinterpret_cast<float*>(&wp[ofs]), prev_w, prev_h);
                break;
            case FORMAT_RGBF:
                if (p_renormalize) {
                    _generate_po2_mipmap<float, 3, true, Image::average_4_float, Image::renormalize_float>(reinterpret_cast<const float*>(&wp[prev_ofs]), reinterpret_cast<float*>(&wp[ofs]), prev_w, prev_h);
                }
                else {
                    _generate_po2_mipmap<float, 3, false, Image::average_4_float, Image::renormalize_float>(reinterpret_cast<const float*>(&wp[prev_ofs]), reinterpret_cast<float*>(&wp[ofs]), prev_w, prev_h);
                }

                break;
            case FORMAT_RGBAF:
                if (p_renormalize) {
                    _generate_po2_mipmap<float, 4, true, Image::average_4_float, Image::renormalize_float>(reinterpret_cast<const float*>(&wp[prev_ofs]), reinterpret_cast<float*>(&wp[ofs]), prev_w, prev_h);
                }
                else {
                    _generate_po2_mipmap<float, 4, false, Image::average_4_float, Image::renormalize_float>(reinterpret_cast<const float*>(&wp[prev_ofs]), reinterpret_cast<float*>(&wp[ofs]), prev_w, prev_h);
                }

                break;
            case FORMAT_RH:
                _generate_po2_mipmap<uint16_t, 1, false, Image::average_4_half, Image::renormalize_half>(reinterpret_cast<const uint16_t*>(&wp[prev_ofs]), reinterpret_cast<uint16_t*>(&wp[ofs]), prev_w, prev_h);
                break;
            case FORMAT_RGH:
                _generate_po2_mipmap<uint16_t, 2, false, Image::average_4_half, Image::renormalize_half>(reinterpret_cast<const uint16_t*>(&wp[prev_ofs]), reinterpret_cast<uint16_t*>(&wp[ofs]), prev_w, prev_h);
                break;
            case FORMAT_RGBH:
                if (p_renormalize) {
                    _generate_po2_mipmap<uint16_t, 3, true, Image::average_4_half, Image::renormalize_half>(reinterpret_cast<const uint16_t*>(&wp[prev_ofs]), reinterpret_cast<uint16_t*>(&wp[ofs]), prev_w, prev_h);
                }
                else {
                    _generate_po2_mipmap<uint16_t, 3, false, Image::average_4_half, Image::renormalize_half>(reinterpret_cast<const uint16_t*>(&wp[prev_ofs]), reinterpret_cast<uint16_t*>(&wp[ofs]), prev_w, prev_h);
                }

                break;
            case FORMAT_RGBAH:
                if (p_renormalize) {
                    _generate_po2_mipmap<uint16_t, 4, true, Image::average_4_half, Image::renormalize_half>(reinterpret_cast<const uint16_t*>(&wp[prev_ofs]), reinterpret_cast<uint16_t*>(&wp[ofs]), prev_w, prev_h);
                }
                else {
                    _generate_po2_mipmap<uint16_t, 4, false, Image::average_4_half, Image::renormalize_half>(reinterpret_cast<const uint16_t*>(&wp[prev_ofs]), reinterpret_cast<uint16_t*>(&wp[ofs]), prev_w, prev_h);
                }

                break;
            case FORMAT_RGBE9995:
                if (p_renormalize) {
                    _generate_po2_mipmap<uint32_t, 1, true, Image::average_4_rgbe9995, Image::renormalize_rgbe9995>(reinterpret_cast<const uint32_t*>(&wp[prev_ofs]), reinterpret_cast<uint32_t*>(&wp[ofs]), prev_w, prev_h);
                }
                else {
                    _generate_po2_mipmap<uint32_t, 1, false, Image::average_4_rgbe9995, Image::renormalize_rgbe9995>(reinterpret_cast<const uint32_t*>(&wp[prev_ofs]), reinterpret_cast<uint32_t*>(&wp[ofs]), prev_w, prev_h);
                }

                break;
            default: {
            }
            }

            prev_ofs = ofs;
            prev_w = w;
            prev_h = h;
        }

        mipmaps = true;

        return OK;

    }

    int32_t Image::getPixelFormatByteSize(PixelFormat format) const
    {
        switch (format)
        {
        case PF_L8:
            return 1;
        case PF_R8:
            return 1;
        case PF_RG88:
            return 2;
        case PF_RGB888:
            return 3;
        case PF_RGBA8888:
            return 4;
        case PF_RGBA4444:
            return 2;
        case PF_RGB565:
            return 2;
        case PF_R32F:
            return 4;
        case PF_RG32F:
            return 8;
        case PF_RGB32F:
            return 12;
        case PF_RGBA32F:
            return 16;
        case PF_R16F:
            return 2;
        case PF_RG16F:
            return 4;
        case PF_RGB16F:
            return 8;
        case PF_RGBA16F:
            return 8;
        default:
            break;
        }
        return 0;
    }

    int32_t Image::getByteSize(int32_t width, int32_t height, PixelFormat format, int32_t& mipmapCount, int32_t targetMipmaps) const
    {
        int32_t size = 0;
        int32_t w = width;
        int32_t h = height;

        // Current mipmap index in the loop below. p_mipmaps is the target mipmap index.
        // In this function, mipmap 0 represents the first mipmap instead of the original texture.
        int32_t mm = 0;

        const int32_t pixelByteSize = getPixelFormatByteSize(format);

        const int32_t minWidth = 1;
        const int32_t minHeight = 1;

        while (true)
        {
            int32_t byteSize = w * h * pixelByteSize;

            size += byteSize;

            if (targetMipmaps >= 0)
            {
                w = std::max(minWidth, w >> 1);
                h = std::max(minHeight, h >> 1);
            }
            else
            {
                if (w == minWidth && h == minHeight)
                {
                    break;
                }
                w = std::max(minWidth, w >> 1);
                h = std::max(minHeight, h >> 1);
            }

            if (targetMipmaps >= 0 && mm == targetMipmaps)
            {
                break;
            }

            mm++;
        }

        mipmapCount = mm;
        return size;
    }

    void Image::getMipmapOffsetAndWidthHeight(int32_t mipmap, int32_t& offset, int32_t& width, int32_t& height) const
    {
        int32_t w = width;
        int32_t h = height;
        int32_t ofs = 0;

        const int32_t pixelByteSize = getPixelFormatByteSize(mFormat);

        const int32_t minWidth = 1,
            const int32_t minHeight = 1;;

        for (int32_t i = 0; i < mipmap; i++)
        {
            int32_t byteSize = w * h * pixelByteSize;

            ofs += byteSize;

            w = std::max(minWidth, w >> 1);
            h = std::max(minHeight, h >> 1);
        }

        offset = ofs;
        width = w;
        height = h;
    }

    int32_t Image::getMipmapOffset(int32_t mipmap) const
    {
        if (mipmap < 0 || mipmap >= getMipmapCount() + 1)
        {
            std::cerr << "Input mipmap's value is invalid" << std::endl;
            return -1;
        }

        int32_t offset, width, height;
        getMipmapOffsetAndWidthHeight(mipmap, offset, width, height);
        return offset;
    }

    int32_t Image::getMipmapByteSize(int32_t mipmap) const
    {
        if (mipmap < 0 || mipmap >= getMipmapCount() + 1)
        {
            std::cerr << "Input mipmap's value is invalid" << std::endl;
            return -1;
        }

        int32_t offset, width, height;
        getMipmapOffsetAndWidthHeight(mipmap, offset, width, height);
        int32_t offset2;
        getMipmapOffsetAndWidthHeight(mipmap + 1, offset2, width, height);
        return offset2 - offset;
    }

    std::pair<int32_t, int32_t> Image::getMipmapOffsetAndByteSize(int32_t mipmap) const
    {
        if (mipmap < 0 || mipmap >= getMipmapCount() + 1)
        {
            std::cerr << "Input mipmap's value is invalid" << std::endl;
            return;
        }

        int32_t offset, width, height;
        getMipmapOffsetAndWidthHeight(mipmap, offset, width, height);
        int32_t offset2;
        getMipmapOffsetAndWidthHeight(mipmap + 1, offset2, width, height);
        int32_t size = offset2 - offset;
        return std::make_pair(offset, size);
    }

    int32_t Image::getMipmapCount() const
    {
        if (mipmaps)
        {
            return get_image_required_mipmaps(width, height, format);
        }
        else
        {
            return 0;
        }
    }
#endif

    

}