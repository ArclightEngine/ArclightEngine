# Arclight Object Management

Much like other other game engines as well as GUI toolkits, Arclight Engine has an object management system. This is intended in the future to support optional use of alternate bindings for other programming languages as well as potential for a GUI editor.

## C++

See `Arclight/Core/Object.h`

Objects are classes inheriting from `Object` , using the `ARCLIGHT_OBJECT` macro.

```c++
#include <Arclight/Core/Object.h>

class UserObject : public Arclight::Object {
    ARCLIGHT_OBJECT(UserObject)
        
public:
    UserObject() = default;
    ~UserObject() override = default;
};
```



