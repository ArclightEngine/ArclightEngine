#pragma once

#include <Arclight/ECS/System.h>

namespace Arclight::Systems {

class Renderer2D
    : public System {
public:
    Renderer2D()
        : System("Renderer2D"){}

    void Tick() override;
};

}
