#pragma once

// See Documentation/Engine/Core/Objects.md

#include <memory>
#include <string_view>

#include <Arclight/Core/Util.h>

// We need static functions so child classes can check the types of parents
// This attempts to require the least amount of boilerplate code by those creating
// a new object class

#define ARCLIGHT_OBJECT(_class, parent)                                                            \
public:                                                                                            \
    static ALWAYS_INLINE constexpr std::string_view ObjectTypeStatic() { return #_class; }         \
    static ALWAYS_INLINE bool IsObjectStatic(const std::string_view& sv) {                         \
        if (sv.compare(#_class) == 0) {                                                            \
            return true;                                                                           \
        }                                                                                          \
                                                                                                   \
        return parent::IsObjectStatic(sv);                                                         \
    }                                                                                              \
                                                                                                   \
    virtual bool IsObject(std::string_view sv) const override { return IsObjectStatic(sv); }       \
    virtual std::string_view ObjectType() const override { return ObjectTypeStatic(); }            \
                                                                                                   \
protected: // Default Protection

namespace Arclight {

class Object {
public:
    static ALWAYS_INLINE constexpr std::string_view ObjectTypeStatic() { return "Object"; }
    static ALWAYS_INLINE constexpr bool IsObjectStatic(const std::string_view& sv) {
        return sv.compare("Object");
    }

    virtual std::string_view ObjectType() const { return ObjectTypeStatic(); }
    virtual bool IsObject(std::string_view sv) const { return IsObjectStatic(sv); }

    virtual ~Object() = default;

private:
};

template <typename T, typename U> std::shared_ptr<T> ObjectCast(const std::shared_ptr<U>& ptr) {
    if (ptr->IsObject(T::ObjectTypeStatic())) {
        return static_pointer_cast<T>(ptr);
    }

    return nullptr;
}

} // namespace Arclight
