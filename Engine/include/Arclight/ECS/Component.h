#pragma once

#include <Arclight/Core/Object.h>

namespace Arclight {

class Component : public Object {
    ARCLIGHT_OBJECT(Component, Object);

    // Components should not contain any logic, only data
    ~Component() final override;
};

} // namespace Arclight
