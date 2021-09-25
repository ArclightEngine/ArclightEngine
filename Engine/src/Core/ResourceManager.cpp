#include <Arclight/Core/ResourceManager.h>

#include <Arclight/Core/File.h>
#include <Arclight/Graphics/Image.h>
#include <Arclight/Graphics/Font.h>

#include <cassert>
#include <cstdio>
#include <map>

#include <nlohmann/json.hpp>

namespace Arclight {

class ImageFormat final : public ResourceFormat {
public:
    bool CanLoad(const UnicodeString& path) const override {
        static const std::vector<UnicodeString> supportedExtensions = {
            ".png",
            ".jpg",
            ".bmp",
            ".gif",
            ".psd",
            ".tga",
            ".ppm", 
        };

        for (const auto& extension : supportedExtensions) {
            if (path.endsWith(extension)) {
                return true;
            }
        }

        return false;
    }

    std::shared_ptr<Resource> CreateResource() const override {
        return std::make_shared<Image>();
    }
};

class FontFormat final : public ResourceFormat {
public:
    bool CanLoad(const UnicodeString& path) const override {
        static const std::vector<UnicodeString> supportedExtensions = {
            ".ttf",
            ".otf"
        };

        for (const auto& extension : supportedExtensions) {
            if (path.endsWith(extension)) {
                return true;
            }
        }

        return false;
    }

    std::shared_ptr<Resource> CreateResource() const override {
        return std::make_shared<Font>();
    }
};

ResourceManager* ResourceManager::s_instance = nullptr;

ResourceManager::ResourceManager() {
    assert(!s_instance);
    s_instance = this;

    RegisterFormat(new ImageFormat());
    RegisterFormat(new FontFormat());
}

ResourceManager::~ResourceManager() {
    for(auto* f : m_formats){
        delete f;
    }
}

void ResourceManager::RegisterFormat(ResourceFormat* format){
    m_formats.push_back(format);
}

const std::shared_ptr<Resource>& ResourceManager::GetResource(const std::string& id) {
    std::unique_lock lockRes(m_lock);

    if(m_resources.find(id) != m_resources.end()){
        auto& res = m_resources.at(id);
        res->Load();

        return res;
    }

    UnicodeString uID = UnicodeString("./").append(UnicodeString(id.c_str()));
    if(File::Access(uID)){
        for(auto* f : m_formats){
            if(f->CanLoad(uID)){
                auto r = f->CreateResource();
                r->SetFilesystemPath(std::move(uID));
                r->Load();

                m_resources[id] = r;
                break;
            }
        }
    }

    return m_resources.at(id);
}

} // namespace Arclight
