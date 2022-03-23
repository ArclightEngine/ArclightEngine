#include <Arclight/Core/Time.h>

#include <windows.h>

namespace Arclight {

void sleep_for_useconds(long us) { Sleep(us / 1000); }

} // namespace Arclight
