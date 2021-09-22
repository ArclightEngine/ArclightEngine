#include <Arclight/Core/File.h>

#include <Arclight/Platform/Filesystem.h>

namespace Arclight {

bool File::Access(const UnicodeString& path, int mode) {
    return Platform::_Access(path, mode);
}

File* File::Open(const UnicodeString& path, int flags, Error* error) {
    return Platform::_OpenFile(path, flags, error);
}

}
