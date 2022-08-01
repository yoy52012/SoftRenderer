#pragma once

#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "Image.h"
#include "FrameBuffer.h"
#include "Buffer.h"

namespace SoftRenderer
{
	class Texture2D
	{
	public:
		using Ptr = std::shared_ptr<Texture2D>;

		static Texture2D::Ptr create(const std::string& filename);


		Texture2D();
		Texture2D(int width, int height);
		~Texture2D();

		void init(int width, int height);
		void initFromImage(Image::Ptr image);
		void initFromFile(const std::string& filename);
		void initFromColorBuffer(FrameBuffer::Ptr framebuffer);
		void initFromDepthBuffer(FrameBuffer::Ptr framebuffer);

		glm::vec4 sample(const glm::vec2& texcoord);

		glm::vec4 sampleRepeat(const glm::vec2& texcoord);
		glm::vec4 sampleClamp(const glm::vec2& texcoord);

	private:
		int mWidth = 0;
		int mHeight = 0;
		std::vector<glm::vec4> mBuffer;

	};

	enum class WrapMode
	{
		WRAP_REPEAT,
		WRAP_MIRRORED_REPEAT,
		WRAP_CLAMP_TO_EDGE,
		WRAP_CLAMP_TO_BORDER,
		WRAP_CLAMP_TO_ZERO,
	};

	enum class FilterMode
	{
		FILTER_NEAREST,
		FILTER_LINEAR,
		FILTER_NEAREST_MIPMAP_NEAREST,
		FILTER_LINEAR_MIPMAP_NEAREST,
		FILTER_NEAREST_MIPMAP_LINEAR,
		FILTER_LINEAR_MIPMAP_LINEAR,
	};

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
					{ //uint8
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
					{ //half float

						float xofs_frac = float(src_xofs_frac) / (1 << FRAC_BITS);
						float yofs_frac = float(src_yofs_frac) / (1 << FRAC_BITS);
						const T* src = ((const T*)src_data);
						T* dst = ((T*)dst_data);

						float p00 = Math::half_to_float(src[y_ofs_up + src_xofs_left + l]);
						float p10 = Math::half_to_float(src[y_ofs_up + src_xofs_right + l]);
						float p01 = Math::half_to_float(src[y_ofs_down + src_xofs_left + l]);
						float p11 = Math::half_to_float(src[y_ofs_down + src_xofs_right + l]);

						float interp_up = p00 + (p10 - p00) * xofs_frac;
						float interp_down = p01 + (p11 - p01) * xofs_frac;
						float interp = interp_up + ((interp_down - interp_up) * yofs_frac);

						dst[i * dst_width * C + j * C + l] = Math::make_half_float(interp);
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

	#define LANCZOS_TYPE 3

	template <int C, class T>
	static void _scale_lanczos(const uint8_t* __restrict p_src, uint8_t* __restrict p_dst, uint32_t p_src_width, uint32_t p_src_height, uint32_t p_dst_width, uint32_t p_dst_height) 
	{
		auto lanczos = [](float p_x) 
		{
			return Math::abs(p_x) >= LANCZOS_TYPE ? 0 : Math::sincn(p_x) * Math::sincn(p_x / LANCZOS_TYPE);
		};

		int32_t src_width  = p_src_width;
		int32_t src_height = p_src_height;
		int32_t dst_height = p_dst_height;
		int32_t dst_width  = p_dst_width;

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

						const T* __restrict src_data = ((const T*)p_src) + (buffer_y * src_width + target_x) * C;

						for (uint32_t i = 0; i < C; i++) 
						{
							if (sizeof(T) == 2) { //half float
								pixel[i] += Math::half_to_float(src_data[i]) * lanczos_val;
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

			float* kernel =  new float[half_kernel * 2];

			for (int32_t dst_y = 0; dst_y < dst_height; dst_y++) 
			{
				float buffer_y = (dst_y + 0.5f) * y_scale;
				int32_t start_y = std::max(0, int32_t(buffer_y) - half_kernel + 1);
				int32_t end_y = std::min(src_height - 1, int32_t(buffer_y) + half_kernel);

				for (int32_t target_y = start_y; target_y <= end_y; target_y++) 
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

					T* dst_data = ((T*)p_dst) + (dst_y * dst_width + dst_x) * C;

					for (uint32_t i = 0; i < C; i++) 
					{
						pixel[i] /= weight;

						if (sizeof(T) == 1) //byte
						{   
							dst_data[i] = CLAMP(Math::fast_ftoi(pixel[i]), 0, 255);
						}
						else if (sizeof(T) == 2) //half float
						{ 
							dst_data[i] = Math::make_half_float(pixel[i]);
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



	template<typename T>
	class Texture
	{
	public:

	private:
		void generateMipmaps();


	private:
		

		std::shared_ptr<Buffer<glm::tvec4<T>>> mDataBuffer;
		std::vector<std::shared_ptr<Buffer<glm::tvec4<T>>>> mMipmaps;
	};

	template<typename T>
	class Sampler
	{
	public:
		
		static glm::tvec4<T> sampleNearest(const Buffer<glm::tvec4<T>>& buffer, const glm::vec2& uv, WrapMode wrapMode, const glm::ivec2& offset);

		static glm::tvec4<T> sampleBilinear(const Buffer<glm::tvec4<T>>& buffer, const glm::vec2& uv, WrapMode wrapMode, const glm::ivec2& offset);

		static glm::tvec4<T> sampleWithWrapMode(const Buffer<glm::tvec4<T>>& buffer, int32_t x, int32_t y, WrapMode wrapMode);

	};

	// Find the first greater number which equals to 2^n, from jdk1.7
	static int roundUpToPowerOf2(int n) 
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

	template<typename T>
	void Texture<T>::generateMipmaps()
	{
		if (mipmaps_ready_ || mipmaps_generating_) 
		{
			return;
		}

		mMipmaps.clear();

		int width = (int)buffer->GetWidth();
		int height = (int)buffer->GetHeight();

		
		int levelSize = std::max(roundUpToPowerOf2(width), roundUpToPowerOf2(height));

		mMipmaps.emplace_back(Buffer<glm::tvec4<T>>::create());

		Buffer<glm::tvec4<T>>* levelBuffer = mMipmaps.back().get();
		levelBuffer->init(levelSize, levelSize);

	}

	template<typename T>
	inline glm::tvec4<T> Sampler<T>::sampleNearest(const Buffer<glm::tvec4<T>>& texture, const glm::vec2& uv, WrapMode wrapMode, const glm::ivec2& offset)
	{
		const int32_t x = (int32_t)(uv.x * (texture.getWidth()  - 1) + 0.5f) + offset.x;
		const int32_t y = (int32_t)(uv.y * (texture.getHeight() - 1) + 0.5f) + offset.y;

		return sampleWithWrapMode(texture, x, y, wrapMode);
	}

	template<typename T>
	inline glm::tvec4<T> Sampler<T>::sampleBilinear(const Buffer<glm::tvec4<T>>& texture, const glm::vec2& uv, WrapMode wrapMode, const glm::ivec2& offset)
	{
		const float x = (uv.x * texture.getWidth()  - 0.5f) + offset.x;
		const float y = (uv.y * texture.getHeight() - 0.5f) + offset.y;
		const int32_t ix = (int32_t)x;
		const int32_t iy = (int32_t)y;

		/*********************
		 *   p2--p3
		 *   |   |
		 *   p0--p1
		 * Note: p0 is (ix,iy)
		 ********************/

		auto p0 = sampleWithWrapMode(texture, ix, iy, wrapMode);
		auto p1 = sampleWithWrapMode(texture, ix + 1, iy, wrapMode);
		auto p2 = sampleWithWrapMode(texture, ix, iy + 1, wrapMode);
		auto p3 = sampleWithWrapMode(texture, ix + 1, iy + 1, wrapMode);

		glm::vec2 f = glm::fract(glm::vec2(x, y));

		return glm::mix(glm::mix(s0, s1, f.x), glm::mix(s2, s3, f.x), f.y);
	}

	template<typename T>
	inline glm::tvec4<T> Sampler<T>::sampleWithWrapMode(const Buffer<glm::tvec4<T>>& buffer, int32_t x, int32_t y, WrapMode wrapMode)
	{
		const int32_t width  = buffer.getWidth();
		const int32_t height = buffer.getHeight();

		auto uvRepeat = [](int i, int n)
		{
			return (i & (n-1) + n) & (n-1); // (i % n + n) % n
		};

		auto uvMirror = [](int i)
		{
			return i >= 0 ? i : (-1 - i)
		};
		
		int32_t sampleX = x;
		int32_t sampleY = y; 
		switch (wrapMode)
		{
		case WrapMode::WRAP_REPEAT:
			sampleX = uvRepeat(x, width);
			sampleY = uvRepeat(y, height);
			break;
		case WrapMode::WRAP_MIRRORED_REPEAT:
			break;
		case WrapMode::WRAP_CLAMP_TO_EDGE:
			if(x < 0) sampleX = 0;
			if(y < 0) sampleY = 0;
			if(x >= width) sampleX = width - 1;
			if(y >= height) sampleY = height - 1;
			break;
		case WrapMode::WRAP_CLAMP_TO_BORDER:
			if (x < 0 || x >= w) return BORDER_COLOR;
			if (y < 0 || y >= h) return BORDER_COLOR;
			break;
		case WrapMode::WRAP_CLAMP_TO_ZERO:
			if (x < 0 || x >= w) return glm::tvec4<T>(0);
			if (y < 0 || y >= h) return glm::tvec4<T>(0);
			break;
		default:
			break;
		}

		glm::tvec4<T>* pixel = buffer.get(sampleX, sampleY);

		if (pixel)
		{
			return *pixel;
		}

		return glm::tvec4(0);
	}
}