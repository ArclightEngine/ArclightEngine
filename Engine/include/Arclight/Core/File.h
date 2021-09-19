#pragma once

#include <Arclight/Core/Error.h>
#include <Arclight/Core/UnicodeString.h>

#include <memory>

namespace Arclight {

class File {
public:
    enum {
        OpenReadOnly = 0,
        OpenWrite = 1,
        OpenReadWrite = 2,
        OpenReadWriteCreate = 4,
        OpenAccess = 3,
    };

    // It's important we use UnicodeString here,
    // as some platforms like Windows use UTF-16 for filenames.
    static bool Access(const UnicodeString& path, int mode = OpenReadOnly);
    static File* Open(const UnicodeString& path, int flags = OpenReadOnly, Error* error = nullptr);

    virtual bool IsOpen() const = 0;
    virtual bool IsEOF() const = 0;

    virtual int Seek(size_t offset) = 0;
    virtual ssize_t Tell() const = 0;
    virtual ssize_t GetSize() const = 0;

    virtual ssize_t Read(void* buffer, size_t size, Error* e = nullptr) = 0;
    template <typename T> inline size_t ReadObjects(T* buffer, size_t count) {
        return Read(buffer, sizeof(T) * count);
    }

    virtual ssize_t Write(const void* buffer, size_t size, Error* e = nullptr) = 0;
    template <typename T> inline ssize_t WriteObjects(T* buffer, size_t count) {
        return Write(buffer, sizeof(T) * count);
    }

    virtual ~File() = default;

protected:
    File() = default;
};

} // namespace Arclight