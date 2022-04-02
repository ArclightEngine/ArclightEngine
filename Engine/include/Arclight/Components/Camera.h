#pragma once

#include <Arclight/Components/Transform.h>
#include <Arclight/ECS/World.h>

namespace Arclight {

struct Camera2D {
    Transform2D transform;
};

void camera2d_set_as_current(World& world, Entity entity);
Camera2D* camera2d_get_current(World& world);

void camera_system(float elapsed, World& world);

}