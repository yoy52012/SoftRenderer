#pragma once

#include <vector>
#include <string>

#include "Image.h"

namespace SoftRenderer
{
    class ImageLoaderManager;
    class ImageLoader
    {
    public:
        ImageLoader() = default;
        virtual ~ImageLoader() = default;

    protected:
        virtual bool loadImage(const std::string& path, std::shared_ptr<Image> image) = 0;
        virtual void getRecognizedExtensions(std::vector<std::string>& extensions) const = 0;

        bool recognize(const std::string& extension) const;

        friend class ImageLoaderManager;
    };


    class ImageLoaderManager
    {
    public:
        static ImageLoaderManager* getInstance();

        virtual ~ImageLoaderManager();

        void initialize();

        void uninitialize();

        bool loadImage(const std::string& path, std::shared_ptr<Image> image, float scale = 1.0);
        
        //static void getRecognizedExtensions(std::vector<std::string>* extensions);
        //static std::shared_ptr<ImageLoader> recognize(const std::string& extension);

        void addImageFormatLoader(std::shared_ptr<ImageLoader> loader);

        void removeImageFormatLoader(std::shared_ptr<ImageLoader> loader);

        const std::vector<std::shared_ptr<ImageLoader>>& getImageFormatLoaders() const;

        void clean();

    private:
        ImageLoaderManager() = default;

    private:
        static ImageLoaderManager* sInstance;

        std::vector<std::shared_ptr<ImageLoader>> loaders;
    };


    class ImageLoaderJPG : public ImageLoader
    {
    public:
        virtual bool loadImage(const std::string& path, std::shared_ptr<Image> image) override;
        virtual void getRecognizedExtensions(std::vector<std::string>& extensions) const override;
    };

    class ImageLoaderPNG : public ImageLoader
    {
    public:
        virtual bool loadImage(const std::string& path, std::shared_ptr<Image> image) override;
        virtual void getRecognizedExtensions(std::vector<std::string>& extensions) const override;
    };
}

