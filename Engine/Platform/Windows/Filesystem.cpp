#include <Arclight/Core/File.h>
#include <Arclight/Platform/Platform.h>

#include <fileapi.h>

namespace Arclight {

class OSFile : public File {
public:
private:
    HANDLE m_fileDescriptor;
};

} // namespace Arclight