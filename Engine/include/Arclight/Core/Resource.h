#pragma once

#include <Arclight/Core/Object.h>
#include <Arclight/Core/UnicodeString.h>

namespace Arclight {

class Resource : public Object {
    friend class ResourceManager;

    ARCLIGHT_OBJECT(Resource, Object)
public:
    Resource();
    virtual ~Resource() = default;

    // Load if not loaded
    virtual int Load() = 0;

    inline void SetFilesystemPath(UnicodeString path) { m_filesystemPath = std::move(path); }

protected:
    UnicodeString m_filesystemPath;
};

} // namespace Arclight
