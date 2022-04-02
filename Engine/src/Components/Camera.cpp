#include <Arclight/Components/Camera.h>

#include <entt/core/hashed_string.hpp>

namespace Arclight {

struct Camera2DContext {
    Entity entity;
};

void camera2d_set_as_current(World& world, Entity entity) {
    world.ctx_set<Camera2DContext>(entity);
}

Camera2D* camera2d_get_current(World& world) {
    auto* ctx = world.try_ctx<Camera2DContext>();
    if(ctx) {
        return &world.get_component<Camera2D>(ctx->entity);
    }

    return nullptr;
}

Transform2D camera2d_get_transformation(Camera2D* camera) {
    Transform2D transform = camera->transform;
    transform.set_position(camera->transform.get_position() * -1.f);

    return transform;
}

void camera_system(float elapsed, World& world) {
    Camera2D* current = camera2d_get_current(world);    
}

} // namespace Arclight
