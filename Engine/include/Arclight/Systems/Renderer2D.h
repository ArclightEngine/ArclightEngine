#pragma once

#include <Arclight/ECS/System.h>
#include <Arclight/ECS/World.h>

#include <Arclight/Graphics/VertexBuffer.h>

namespace Arclight::Systems {

struct Renderer2DContext {
    VertexBuffer spriteVertexBuffer;
};

void renderer_2d(float elapsed, World& world);

}
