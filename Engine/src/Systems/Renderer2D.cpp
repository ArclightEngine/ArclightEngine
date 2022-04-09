#include <Arclight/Systems/Renderer2D.h>

#include <Arclight/Components/Camera.h>
#include <Arclight/Components/Sprite.h>
#include <Arclight/Core/Application.h>
#include <Arclight/Core/Logger.h>
#include <Arclight/Graphics/Rendering/Renderer.h>
#include <Arclight/Graphics/Text.h>

namespace Arclight::Systems {

void renderer_2d(float, World& world) {
    Rendering::Renderer& renderer = *Rendering::Renderer::instance();

    auto sprites = world.registry().group<Sprite>(entt::get<Transform2D>);
    auto textObjects = world.registry().group<Text>(entt::get<Transform2D>);

    unsigned vertexCount = (sprites.size() + textObjects.size()) * 4;
    if (vertexCount <= 0) { // Nothing to draw
        return;
    }

    // Create context if does not exist
    Renderer2DContext* ctx = world.try_ctx<Renderer2DContext>();
    if (!ctx) {
        ctx = &world.ctx_set<Renderer2DContext>();
    }

    auto& vbuf = ctx->spriteVertexBuffer;
    if (vertexCount > vbuf.size()) {
        vbuf.reallocate(vertexCount + 16);
    }

    // Check if the vertices required take up less than 50% of the vertex buffer
    // If so reallocate to save memory
    if (vertexCount < vbuf.size() / 2) {
        vbuf.reallocate(vertexCount + 16);
    }

    Camera2D* camera = camera2d_get_current(world);
    Transform2D viewTransform;
    if (camera) {
        viewTransform = camera2d_get_transformation(camera);
    }

    auto pipeline = renderer.default_pipeline().handle();
    unsigned nextVertex = 0;
    for (Entity ent : sprites) {
        Sprite& sprite = sprites.get<Sprite>(ent);
        Transform2D& t = sprites.get<Transform2D>(ent);

        Texture::TextureHandle tex = nullptr;
        if (sprite.texture)
            tex = sprite.texture->handle();

        vbuf.update(sprite.vertices, nextVertex, 4);

        renderer.draw(vbuf.handle(), nextVertex, 4, t.matrix(),
                      viewTransform.matrix(), tex, pipeline);
        nextVertex += 4;
    }

    for (Entity ent : textObjects) {
        Text& text = textObjects.get<Text>(ent);
        Transform2D& t = textObjects.get<Transform2D>(ent);

        vbuf.update(text.vertices(), nextVertex, 4);
        renderer.draw(vbuf.handle(), nextVertex, 4, t.matrix(), viewTransform.matrix(),
                      text.tex().handle(), pipeline);
        nextVertex += 4;
    }
}

} // namespace Arclight::Systems
