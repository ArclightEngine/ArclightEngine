#include <Arclight/Core/Time.h>

#include <unistd.h>

namespace Arclight {

void sleep_for_useconds(long us) { usleep(us); }

} // namespace Arclight
