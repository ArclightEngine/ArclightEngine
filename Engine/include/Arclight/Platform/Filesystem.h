#pragma once

#include <Arclight/Core/Error.h>
#include <Arclight/Core/UnicodeString.h>

namespace Arclight::Platform {

bool _Access(const UnicodeString& path, int mode);
File* _OpenFile(const UnicodeString& path, int mode, Error* e = nullptr);

} // namespace Arclight::Platform
