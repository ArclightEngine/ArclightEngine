#pragma once

#include <stdexcept>

namespace Arclight {

class Object {
public:
    enum Type {
        ObjectInvalid,
        ObjectPrimitive,
    };

    virtual ~Object() = default;

private:
    Type m_type = ObjectInvalid;
};

} // namespace Arclight