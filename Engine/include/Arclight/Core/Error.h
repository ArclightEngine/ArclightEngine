#pragma once

namespace Arclight {

enum Error {
    Error_OK = 0,
    Error_Unknown,
    Error_Initialization,
    Error_InvalidArgument,
    Error_FileNotFound,
    Error_FilePermission,
    Error_FileNotOpen,
};

} // namespace Arclight
