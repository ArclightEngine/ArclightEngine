#pragma once

#include <Arclight/Core/UnicodeString.h>

namespace Arclight {

class Resource {
    friend class ResourceManager;

public:
    Resource(std::string name);
    virtual ~Resource() = default;

    // Load if not loaded
    virtual int Load() = 0;

    inline void SetFilesystemPath(UnicodeString path) { m_filesystemPath = std::move(path); }

protected:
    std::string m_name;
    UnicodeString m_filesystemPath;
};

} // namespace Arclight
