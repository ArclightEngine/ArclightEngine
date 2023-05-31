#include <Arclight/Core/File.h>
#include <Arclight/Core/Logger.h>
#include <Arclight/Platform/Filesystem.h>
#include <Arclight/Platform/Platform.h>

#include <assert.h>
#include <windows.h>
#include <fileapi.h>

#include <string>

namespace Arclight::Platform {

class OSFile : public File {
public:
    OSFile(FILE* f) : m_file(f) { Seek(0); }
    ~OSFile() override {
        if (m_file) {
            fclose(m_file);
        }
    }

    bool IsOpen() const override { return m_file; }
    bool IsEOF() const override { return feof(m_file) != 0; }

    int Seek(size_t off) override { return fseek(m_file, off, SEEK_SET); }

    ssize_t Tell() const override { return ftell(m_file); }

    ssize_t get_size() const override {
        assert(IsOpen());

        ssize_t pos = Tell();

        fseek(m_file, 0, SEEK_END);
        ssize_t size = Tell();

        fseek(m_file, pos, SEEK_SET);

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
    if (GetFileAttributesW((wchar_t*)upath.getBuffer()) == INVALID_FILE_ATTRIBUTES) {
        return false;
    }

    return true;
}

File* _OpenFile(const UnicodeString& upath, int mode, Error* e) {
    static_assert(sizeof(wchar_t) == sizeof(char16_t));
    UnicodeString pathCopy = upath;
    const wchar_t* path = as_wide_string(pathCopy);

    // Windows (unlike unixes) actually gives a shit about the 'b' flag
    // (~>_<)~
    const wchar_t* cMode = L"rb";
    switch (mode & File::OpenAccess) {
    case File::OpenWrite:
        cMode = L"wb";
        break;
    case File::OpenReadWrite:
        cMode = L"rb+";
        break;
    case File::OpenReadWriteCreate:
        cMode = L"wb+";
        break;
    }

    FILE* filep = nullptr;
    if (int err = (int)_wfopen_s(&filep, path, cMode)) {
        Logger::Error("Failed to open {}: {}", pathCopy, strerror(err));
        if (e) {
            switch (err) {
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

} // namespace Arclight
