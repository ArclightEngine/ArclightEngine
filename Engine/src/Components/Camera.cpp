#include <Arclight/Components/Camera.h>

#include <Arclight/Core/Logger.h>

#include <assert.h>

#include <entt/core/hashed_string.hpp>

namespace Arclight {

struct Camera2DContext {
    Entity entity;
};

void camera2d_set_as_current(World& world, Entity entity) {
    if(!world.has_all_of<Transform2D, Camera2D>(entity)){
        Logger::Error("camera2d_set_as_current: entity requires Transform2D, Camera2D");
        return;
    }
    
    world.ctx_set<Camera2DContext>(entity);
}

Entity camera2d_get_current(World& world) {
    auto* ctx = world.try_ctx<Camera2DContext>();
    if(ctx) {
        return ctx->entity;
    }

    return NullEntity;
}

Transform2D camera2d_get_transformation(Entity ent) {
    Transform2D transform = World::current().get_component<Transform2D>(ent);
    transform.set_position(transform.get_position() * -1.f);

    return transform;
}

Vector2f camera2d_top_left(Entity ent) {
    //auto c = World::current().get_component<Camera2D>(ent);
    //auto t = World::current().get_component<Transform2D>(ent);
}

void camera_system(float elapsed, World& world) {
    
}

} // namespace Arclight
