#pragma once

namespace Arclight {

class InputBinding {
public:
    enum BindingFlags {
        BindingKeyboard = 0x1,
        BindingGamepad = 0x2,  
    };

    float Value() const;
    int RawValue() const;
private:
    BindingFlags m_bindingFlags;
};

} // namespace Arclight