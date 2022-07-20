#pragma once

#include <memory>

namespace SoftRenderer
{
    template<typename T>
    class Buffer
    {
    public:

        static std::shared_ptr<Buffer<T>> create()
        {
            return std::make_shared<Buffer<T>>();
        }

        Buffer()
            : mWidth(0)
            , mHeight(0)
            , mDataWidth(0)
            , mDataHeight(0)
            , mDataSize(0)
            , mData(nullptr)
        {

        }

        ~Buffer()
        {

        }

        void init(unsigned int width, unsigned int height)
        {
            if (width > 0 && height > 0)
            {
                if (mWidth == width && mHeight == height)
                {
                    return;
                }

                mWidth = width;
                mHeight = height;

                initSize();

                mData = std::shared_ptr<T>(new T[mDataSize], [](const T* ptr) {delete[] ptr; });
            }
        }


        inline bool isEmpty() const
        {
            return mData == nullptr;
        }

        inline unsigned int getWidth() const
        {
            return mWidth;
        }

        inline unsigned int getHeight() const
        {
            return mHeight;
        }

        inline T* get(unsigned int x, unsigned int y)
        {
            T* dataPtr = mData.get();
            if (dataPtr != nullptr)
            {
                if (x < mWidth && y < mHeight)
                {
                    return &dataPtr[convertIndex(x, y)];
                }
            }
            return nullptr;
        }

        inline void set(unsigned int x, unsigned int y, const T& value)
        {
            T* dataPtr = mData.get();
            if (dataPtr != nullptr)
            {
                if (x < mWidth && y < mHeight)
                {
                    dataPtr[convertIndex(x, y)] = value;
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

    private:
        virtual inline void initSize(unsigned int width, unsigned int height)
        {
            mDataWidth = width;
            mDataHeight = height;
            mDataSize = mDataWidth * mDataHeight;
        }

        virtual inline unsigned int convertIndex(unsigned int x, unsigned y) const
        {
            y* mWidth + x;
        }


    private:
        unsigned int mWidth;
        unsigned int mHeight;

        unsigned int mDataWidth;
        unsigned int mDataHeight;

        unsigned int mDataSize;
        std::shared_ptr<T> mData;
    };


    template<T>
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

