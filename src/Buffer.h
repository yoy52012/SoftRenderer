#pragma once

#include <memory>

namespace SoftRenderer
{
    template<typename T>
    class Buffer
    {
    public:

        static std::shared_ptr<Buffer<T>> create(uint32_t width, uint32_t height)
        {
            auto buffer = std::make_shared<Buffer<T>>();
            buffer->init(width, height);
            return buffer;
        }

        Buffer() = default;
        virtual ~Buffer() = default;

        Buffer(const Buffer& other)
        {
            destroy();

            init(other.mWidth, other.mHeight);

            std::memcpy(mData->get(), other.mData->get(), mDataSize * sizeof(T));
        }

        Buffer& operator=(const Buffer& other)
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

                initDataSize();

                mData = std::shared_ptr<T>(new T[mDataSize], [](const T* ptr) {delete[] ptr; });
            }
        }

        virtual void destroy() 
        {
            mWidth = 0;
            mHeight = 0;
            mDataWidth = 0;
            mDataHeight = 0;
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

        inline T* get(uint32_t x, uint32_t y)
        {
            T* dataPtr = mData.get();
            if (dataPtr != nullptr)
            {
                if (x < mWidth && y < mHeight)
                {
                    const uint32_t index = getDataIndex(x, y);
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
                    const uint32_t index = getDataIndex(x, y);
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
                    for (int i = 0; i < mDataHeight; i++)
                    {
                        std::memcpy(out + mDataWidth * i, dataPtr + mDataWidth * (mDataHeight - 1 - i), mDataWidth * sizeof(T));
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

    private:
        virtual inline void initDataSize()
        {
            mDataWidth = mWidth;
            mDataHeight = mHeight;
            mDataSize = mDataWidth * mDataHeight;
        }

        virtual inline uint32_t getDataIndex(uint32_t x, uint32_t y) const
        {
            y * mWidth + x;
        }


    private:
        uint32_t mWidth = 0;
        uint32_t mHeight = 0;

        uint32_t mDataWidth = 0;
        uint32_t mDataHeight = 0;

        uint32_t mDataSize = 0;

        std::shared_ptr<T> mData = nullptr;
    };


    template<typename T>
    class TiledBuffer : public Buffer<T>
    {
    private:
        virtual inline void initSize(unsigned int width, unsigned int height) override
        {
            mTileWidth = (this->mWidth + tileSize - 1) / tileSize;
            mTileHeight = (this->mHeight + tileSize - 1) / tileSize;
            this->mDataWidth = mTileWidth * tileSize;
            this->mDataHeight = mTileHeight * tileSize;
        }

        inline unsigned int convertIndex(unsigned int x, unsigned int y) const override 
        {
            uint16_t tileX = x >> bits;              // x / tile_size_
            uint16_t tileY = y >> bits;              // y / tile_size_
            uint16_t inTileX = x & (tileSize - 1);   // x % tile_size_
            uint16_t inTileY = y & (tileSize - 1);   // y % tile_size_

            return ((tileY * mTileWidth + tileX) << bits << bits)
                 + (inTileY << bits)
                 + inTileX;
        }

    private:
        unsigned int mTileWidth;
        unsigned int mTileHeight;

        const static int tileSize = 4;
        const static int bits = 2;

    };

}

