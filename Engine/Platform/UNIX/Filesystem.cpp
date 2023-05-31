#include <Arclight/Core/Fatal.h>
#include <Arclight/Core/File.h>
#include <Arclight/Core/Logger.h>
#include <Arclight/Platform/Platform.h>

#include <cassert>
#include <cstdio>
#include <cstring>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

namespace Arclight::Platform {

class OSFile : public File {
public:
    OSFile(FILE* f) : m_file(f) {}
    ~OSFile() override {
        if (m_file) {
            fclose(m_file);
        }
    }

    inline static int ToUNIXAccessMode(int mode) {
        int uMode = 0;

        int rwMode = mode & OpenAccess;
        switch (rwMode) {
        case OpenReadOnly:
            uMode = R_OK;
            break;
        case OpenWrite:
            uMode = W_OK;
            break;
        case OpenReadWrite:
            uMode = R_OK | W_OK;
            break;
        default:
            FatalRuntimeError("Invalid file access mode!");
            break;
        }

        return uMode;
    }

    bool IsOpen() const override { return m_file; }
    bool IsEOF() const override { return feof(m_file) != 0; }

    int Seek(size_t off) override { return fseeko(m_file, off, SEEK_SET); }

    ssize_t Tell() const override { return ftello(m_file); }

    ssize_t get_size() const override {
        assert(IsOpen());

        ssize_t pos = Tell();

        fseeko(m_file, 0, SEEK_END);
        ssize_t size = Tell();

        fseeko(m_file, pos, SEEK_SET);

        return size;
    }

    ssize_t Read(void* buffer, size_t size, Error* e) override {
        if (!IsOpen()) {
            if (e) {
                *e = Error_FileNotOpen;
            }
            return -1;
        }

        return fread(buffer, 1, size, m_file);
    }

    ssize_t Write(const void* buffer, size_t size, Error* e) override {
        if (!IsOpen()) {
            if (e) {
                *e = Error_FileNotOpen;
            }
            return -1;
        }

        return fwrite(buffer, 1, size, m_file);
    }

private:
    FILE* m_file = nullptr;
};

bool _Access(const UnicodeString& upath, int mode) {
    std::string path;
    upath.toUTF8String<std::string>(path);

    if (access(path.c_str(), OSFile::ToUNIXAccessMode(mode)) != 0) {
        Logger::Error("Failed to access {}: {}", path, strerror(errno));
        return false;
    }

    return true;
}

File* _OpenFile(const UnicodeString& upath, int mode, Error* e) {
    std::string path;
    upath.toUTF8String<std::string>(path);

    // Most UTF-8 strings are valid on UNIX
    // The only reserved character is the null terminator and '/' separator

    const char* cMode = "r";
    switch (mode & File::OpenAccess) {
    case File::OpenWrite:
        cMode = "w";
        break;
    case File::OpenReadWrite:
        cMode = "r+";
        break;
    case File::OpenReadWriteCreate:
        cMode = "w+";
        break;
    }

    FILE* filep = nullptr;
    if (!(filep = fopen(path.c_str(), cMode))) {
        Logger::Error("Failed to access {}: {}", path, strerror(errno));
        if (e) {
            switch (errno) {
            case EACCES:
                *e = Error::Error_FilePermission;
                break;
            case ENOENT:
                *e = Error::Error_FileNotFound;
                break;
            default:
                *e = Error::Error_Unknown;
                break;
            }
        }

        return nullptr;
    }

    assert(filep);
    return new OSFile(filep);
}

} // namespace Arclight::Platform
