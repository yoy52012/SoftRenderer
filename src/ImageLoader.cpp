#include "ImageLoader.h"

#include <iostream>

#include <stb_image.h>

namespace SoftRenderer
{
    ImageLoaderManager* ImageLoaderManager::sInstance = NULL;

    std::string getPathExtension(const std::string& path)
    {
        if (path.find_last_of(".") != std::string::npos)
        {
            return path.substr(path.find_last_of(".") + 1);
        }
        return "";
    }

    bool ImageLoader::recognize(const std::string& extension) const
    {
        std::vector<std::string> extensions;
        getRecognizedExtensions(extensions);
        for (auto& ext : extensions)
        {
            if (ext.compare(extension) == 0)
            {
                return true;
            }
        }

        return false;
    }

    ImageLoaderManager* ImageLoaderManager::getInstance()
    {
        if (!sInstance)
        {
            sInstance = new ImageLoaderManager();
            sInstance->initialize();
        }

        return sInstance;
    }

    ImageLoaderManager::~ImageLoaderManager()
    {
        uninitialize();
    }

    void ImageLoaderManager::initialize()
    {
        addImageFormatLoader(std::make_shared<ImageLoaderJPG>());
        addImageFormatLoader(std::make_shared<ImageLoaderPNG>());
    }

    void ImageLoaderManager::uninitialize()
    {
        clean();
    }

    bool ImageLoaderManager::loadImage(const std::string& path, std::shared_ptr<Image> image, float scale)
    {

        std::string extension = getPathExtension(path);

        for (size_t i = 0; i < loaders.size(); ++i)
        {
            if(!loaders[i]->recognize(extension))
                continue;

            bool result = loaders[i]->loadImage(path, image);
            if (!result)
            {
                std::cerr << " Failed to load image: " << path << std::endl;
                return false;
            }
        }

        return true;
    }

    void ImageLoaderManager::addImageFormatLoader(std::shared_ptr<ImageLoader> loader)
    {
        loaders.emplace_back(loader);
    }

    void ImageLoaderManager::removeImageFormatLoader(std::shared_ptr<ImageLoader> loader)
    {
        for (auto it = loaders.begin(); it != loaders.end(); ) 
        {
            if (*it == loader) 
            {
                it = loaders.erase(it);
                break;
            }
            ++it;
        }
    }

    const std::vector<std::shared_ptr<ImageLoader>>& ImageLoaderManager::getImageFormatLoaders() const
    {
       return loaders;
    }

    void ImageLoaderManager::clean()
    {
        for (size_t i = 0; i < loaders.size(); ++i)
        {
            removeImageFormatLoader(loaders[i]);
        }
    }

    bool ImageLoaderJPG::loadImage(const std::string& path, std::shared_ptr<Image> image)
    {
        int32_t width = 0, height = 0, component = 0;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &component, STBI_default);
        if (data == nullptr)
        {
            std::cerr << "failed to load texture, path: " << path << std::endl;
            return false;
        }

        std::cout << "load image, path: " << path << std::endl;

        std::vector<uint8_t> buffer;
        Image::PixelFormat format = Image::PixelFormat::PF_RGBA8888;

        switch (component)
        {
        case STBI_grey:
            format = Image::PixelFormat::PF_L8;
            buffer.resize(Image::calculateByteSize(width, height, Image::PixelFormat::PF_L8));
            break;
        case STBI_grey_alpha:
            format = Image::PixelFormat::PF_LA8;
            buffer.resize(Image::calculateByteSize(width, height, Image::PixelFormat::PF_LA8));
            break;
        case STBI_rgb:
            format = Image::PixelFormat::PF_RGB888;
            buffer.resize(Image::calculateByteSize(width, height, Image::PixelFormat::PF_RGB888));
            break;
        case STBI_rgb_alpha:
            format = Image::PixelFormat::PF_RGBA8888;
            buffer.resize(Image::calculateByteSize(width, height, Image::PixelFormat::PF_RGBA8888));
            break;
        default:
            break;
        }

        //TODO cast all the image to PF_RGBA8888 format temporary
        format = Image::PixelFormat::PF_RGBA8888;
        buffer.resize(Image::calculateByteSize(width, height, Image::PixelFormat::PF_RGBA8888));

        for (int32_t y = 0; y < height; y++)
        {
            for (int32_t x = 0; x < width; x++)
            {
                
                //int32_t idx = x + y * width;

                //switch (component)
                //{
                //case STBI_grey:
                //{
                //    buffer[idx * 4 + 0] = data[idx];
                //    buffer[idx * 4 + 1] = data[idx];
                //    buffer[idx * 4 + 2] = data[idx];
                //    buffer[idx * 4 + 3] = 255;
                //    break;
                //}

                //case STBI_grey_alpha:
                //{
                //    buffer[idx * 4 + 0] = data[idx * 2 + 0];
                //    buffer[idx * 4 + 1] = data[idx * 2 + 0];
                //    buffer[idx * 4 + 2] = data[idx * 2 + 0];
                //    buffer[idx * 4 + 3] = data[idx * 2 + 1];
                //    break;
                //}
                //case STBI_rgb:
                //{
                //    buffer[idx * 4 + 0] = data[idx * 3 + 0];
                //    buffer[idx * 4 + 1] = data[idx * 3 + 1];
                //    buffer[idx * 4 + 2] = data[idx * 3 + 2];
                //    buffer[idx * 4 + 3] = 255;
                //    break;
                //}
                //case STBI_rgb_alpha:
                //{
                //    buffer[idx * 4 + 0] = data[idx * 4 + 0];
                //    buffer[idx * 4 + 1] = data[idx * 4 + 1];
                //    buffer[idx * 4 + 2] = data[idx * 4 + 2];
                //    buffer[idx * 4 + 3] = data[idx * 4 + 3];
                //    break;
                //}
                //default:
                //    break;
                //}

                for (int32_t c = 0; c < component; c++)
                {
                    int32_t idx = y * width * component + x * component + c;
                    buffer[idx] = data[idx];
                }
            }
        }

        image->create(width, height, format, buffer);

        stbi_image_free(data);

        return true;
    }

    void ImageLoaderJPG::getRecognizedExtensions(std::vector<std::string>& extensions) const
    {
        extensions.emplace_back("jpg");
        extensions.emplace_back("jpeg");
    }

    bool ImageLoaderPNG::loadImage(const std::string& path, std::shared_ptr<Image> image)
    {
        int32_t width = 0, height = 0, component = 0;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &component, STBI_default);
        if (data == nullptr)
        {
            std::cerr << "failed to load texture, path: " << path << std::endl;
            return false;
        }

        std::cout << "load image, path: " << path << std::endl;

        std::vector<uint8_t> buffer;
        Image::PixelFormat format = Image::PixelFormat::PF_RGBA8888;

        switch (component)
        {
        case STBI_grey:
            format = Image::PixelFormat::PF_L8;
            buffer.resize(Image::calculateByteSize(width, height, Image::PixelFormat::PF_L8));
            break;
        case STBI_grey_alpha:
            format = Image::PixelFormat::PF_LA8;
            buffer.resize(Image::calculateByteSize(width, height, Image::PixelFormat::PF_LA8));
            break;
        case STBI_rgb:
            format = Image::PixelFormat::PF_RGB888;
            buffer.resize(Image::calculateByteSize(width, height, Image::PixelFormat::PF_RGB888));
            break;
        case STBI_rgb_alpha:
            format = Image::PixelFormat::PF_RGBA8888;
            buffer.resize(Image::calculateByteSize(width, height, Image::PixelFormat::PF_RGBA8888));
            break;
        default:
            break;
        }

        //TODO cast all the image to PF_RGBA8888 format temporary
        format = Image::PixelFormat::PF_RGBA8888;
        buffer.resize(Image::calculateByteSize(width, height, Image::PixelFormat::PF_RGBA8888));

        for (int32_t y = 0; y < height; y++)
        {
            for (int32_t x = 0; x < width; x++)
            {

                //int32_t idx = x + y * width;

                //switch (component)
                //{
                //case STBI_grey:
                //{
                //    buffer[idx * 4 + 0] = data[idx];
                //    buffer[idx * 4 + 1] = data[idx];
                //    buffer[idx * 4 + 2] = data[idx];
                //    buffer[idx * 4 + 3] = 255;
                //    break;
                //}

                //case STBI_grey_alpha:
                //{
                //    buffer[idx * 4 + 0] = data[idx * 2 + 0];
                //    buffer[idx * 4 + 1] = data[idx * 2 + 0];
                //    buffer[idx * 4 + 2] = data[idx * 2 + 0];
                //    buffer[idx * 4 + 3] = data[idx * 2 + 1];
                //    break;
                //}
                //case STBI_rgb:
                //{
                //    buffer[idx * 4 + 0] = data[idx * 3 + 0];
                //    buffer[idx * 4 + 1] = data[idx * 3 + 1];
                //    buffer[idx * 4 + 2] = data[idx * 3 + 2];
                //    buffer[idx * 4 + 3] = 255;
                //    break;
                //}
                //case STBI_rgb_alpha:
                //{
                //    buffer[idx * 4 + 0] = data[idx * 4 + 0];
                //    buffer[idx * 4 + 1] = data[idx * 4 + 1];
                //    buffer[idx * 4 + 2] = data[idx * 4 + 2];
                //    buffer[idx * 4 + 3] = data[idx * 4 + 3];
                //    break;
                //}
                //default:
                //    break;
                //}

                for (int32_t c = 0; c < component; c++)
                {
                    int32_t idx = y * width * component + x * component + c;
                    buffer[idx] = data[idx];
                }
            }
        }

        image->init(width, height, format, buffer);
        image->convert(Image::PF_RGBA8888);

        stbi_image_free(data);

        return true;
    }

    void ImageLoaderPNG::getRecognizedExtensions(std::vector<std::string>& extensions) const
    {
        extensions.emplace_back("png");
    }

}


