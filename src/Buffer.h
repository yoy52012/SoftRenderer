#pragma once

#include <memory>

namespace SoftRenderer
{
    template<typename T>
    class TextureBuffer
    {
    public:

        static std::shared_ptr<TextureBuffer<T>> create(uint32_t width, uint32_t height)
        {
            auto buffer = std::make_shared<LinearTextureBuffer<T>>(width, height);
            return buffer;
        }

        /**
         * Create empty buffer
         */
        TextureBuffer() = default;

        TextureBuffer(uint32_t width, uint32_t height)
        {
            
        }

        virtual ~TextureBuffer() = default;

        TextureBuffer(const TextureBuffer& other)
        {
            destroy();

            init(other.mWidth, other.mHeight);

            std::memcpy(mData->get(), other.mData->get(), mDataSize * sizeof(T));
        }

        TextureBuffer& operator=(const TextureBuffer& other)
        {
            destroy();

            init(other.mWidth, other.mHeight);

            std::memcpy(mData->get(), other.mData->get(), mDataSize * sizeof(T));
        }

        void init(uint32_t width, uint32_t height)
        {
            if (width > 0 && height > 0)
            {
                if (mWidth == width && mHeight == height)
                {
                    return;
                }

                mWidth = width;
                mHeight = height;

                initLayout();

                mDataSize = mInnerWidth * mInnerHeight;
                mData = std::shared_ptr<T>(new T[mDataSize], [](const T* ptr) {delete[] ptr; });
            }
        }

        virtual void destroy() 
        {
            mWidth = 0;
            mHeight = 0;
            mInnerWidth = 0;
            mInnerHeight = 0;
            mDataSize = 0;
            mData = nullptr;
        }

        inline bool isEmpty() const
        {
            return mData == nullptr;
        }

        inline uint32_t getWidth() const
        {
            return mWidth;
        }

        inline uint32_t getHeight() const
        {
            return mHeight;
        }

        inline T* getData() const 
        {
            return mData.get();
        }

        inline T* get(uint32_t x, uint32_t y)
        {
            T* dataPtr = mData.get();
            if (dataPtr != nullptr)
            {
                if (x < mWidth && y < mHeight)
                {
                    const uint32_t index = convertIndex(x, y);
                    return &dataPtr[index];
                }
            }
            return nullptr;
        }

        inline void set(uint32_t x, uint32_t y, const T& value)
        {
            T* dataPtr = mData.get();
            if (dataPtr != nullptr)
            {
                if (x < mWidth && y < mHeight)
                {
                    const uint32_t index = convertIndex(x, y);
                    dataPtr[index] = value;
                }
            }
        }

        inline void clear() const
        {
            T* dataPtr = mData.get();
            if (dataPtr != nullptr)
            {
                std::memset(dataPtr, 0, mDataSize * sizeof(T));
            }
        }

        inline void copyTo(T* out, bool flipY = false) const
        {
            T* dataPtr = mData.get();
            if (dataPtr != nullptr)
            {
                if (!flipY)
                {
                    std::memcpy(out, dataPtr, mDataSize * sizeof(T));
                }
                else
                {
                    for (int i = 0; i < mInnerHeight; i++)
                    {
                        std::memcpy(out + mInnerWidth * i, dataPtr + mInnerWidth * (mInnerHeight - 1 - i), mInnerWidth * sizeof(T));
                    }
                }
            }
        }

        inline void setAll(T value) const 
        {
            T* dataPtr = mData.get();
            if (dataPtr != nullptr) 
            {
                for (uint32_t i = 0; i < mDataSize; i++) 
                {
                    dataPtr[i] = value;
                }
            }
        }

    protected:
        virtual void initLayout() = 0;

        virtual uint32_t convertIndex(uint32_t x, uint32_t y) const = 0;

    protected:
        uint32_t mWidth = 0;
        uint32_t mHeight = 0;

        uint32_t mInnerWidth = 0;
        uint32_t mInnerHeight = 0;

        uint32_t mDataSize = 0;

        std::shared_ptr<T> mData = nullptr;
    };

    template<typename T>
    class LinearTextureBuffer : public TextureBuffer<T>
    {
    public:
        LinearTextureBuffer(int32_t width, int32_t height)
        : TextureBuffer(width, height)
        {
            init(width, height);
        }

    private:
        inline void initLayout() override
        {
            mInnerWidth  = mWidth;
            mInnerHeight = mHeight;
        }

        inline uint32_t convertIndex(uint32_t x, uint32_t y) const override
        {
            return y * mInnerWidth + x;
        }
    };

    template<typename T>
    class TiledTextureBuffer : public TextureBuffer<T>
    {
    public:
        TiledTextureBuffer(int32_t width, int32_t height)
        : TextureBuffer(width, height)
        {}

    private:

        inline void initLayout() override
        {
            mTileWidth   = (mWidth + tileSize - 1) / tileSize;
            mTileHeight  = (mHeight + tileSize - 1) / tileSize;
            mInnerWidth  =  mTileWidth * tileSize;
            mInnerHeight =  mTileHeight * tileSize;
        }

        inline uint32_t convertIndex(uint32_t x, uint32_t y) const override
        {
            //Tiling address mapping
            //Note: this is naive version
            //return ((y / tileSize) * mTileWidth + (x / tileSize)) * tileSize * tileSize  + (y % tileSize) * tileSize + x % tileSize;
            //Note: this is optimized version
            uint16_t tileX = x >> bits_;              // x / tileSize
            uint16_t tileY = y >> bits_;              // y / tileSize
            uint16_t inTileX = x & (tileSize - 1);    // x % tileSize
            uint16_t inTileY = y & (tileSize - 1);    // y % tileSize

            return ((tileY * mTileWidth + tileX) << bits_ << bits_) + (inTileY << bits_) + inTileX;
        }

    private:
        const static int tileSize = 4;    // 4 x 4
        const static int bits = 2;        // tileSize = 2^bits
        size_t mTileWidth = 0;
        size_t mTileHeight = 0;
    };

    template<typename T>
    class MortonTextureBuffer : public TextureBuffer<T>
    {
    public:
        MortonTextureBuffer(int32_t width, int32_t height)
        :TextureBuffer(width, height)
        {}

    private:
        inline void initLayout() override
        {

        }


        inline uint32_t convertIndex(uint32_t x, uint32_t y) const override
        {
            return 0;
        }
    };
}

