#pragma once

#include <Arclight/Components/Transform.h>
#include <Arclight/ECS/World.h>

namespace Arclight {

struct Camera2D {};

void camera2d_set_as_current(World& world, Entity entity);
Entity camera2d_get_current(World& world);
// Get the transformation the camera applies to objects,
// not the transform of the camera in world space
Transform2D camera2d_get_transformation(Entity camera);
Vector2f camera2d_top_left(Entity camera);

void camera_system(float elapsed, World& world);

}