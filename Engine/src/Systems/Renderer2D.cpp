#include <Arclight/Systems/Renderer2D.h>

#include <Arclight/Components/Sprite.h>
#include <Arclight/Core/Application.h>
#include <Arclight/Core/Logger.h>
#include <Arclight/Graphics/Rendering/Renderer.h>
#include <Arclight/Graphics/Text.h>

namespace Arclight::Systems {

void renderer_2d(float, World& world) {
    Rendering::Renderer& renderer = *Rendering::Renderer::instance();

    auto spriteView = world.registry().view<Sprite>();
    auto textView = world.registry().view<Text>();

    unsigned vertexCount = (spriteView.size() + textView.size()) * 4;
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

    Vertex* vertices = vbuf.get_mapping();

    auto pipeline = renderer.default_pipeline().handle();
    unsigned nextVertex = 0;
    for (Entity ent : spriteView) {
        Sprite& sprite = spriteView.get<Sprite>(ent);

        Texture::TextureHandle tex = nullptr;
        if (sprite.texture)
            tex = sprite.texture->handle();

        memcpy(vertices + nextVertex, sprite.vertices, sizeof(Vertex) * 4);

        renderer.draw(vbuf.handle(), nextVertex, 4, sprite.transform.Matrix(), tex, pipeline);
        nextVertex += 4;
    }

    for (Entity ent : textView) {
        Text& text = textView.get<Text>(ent);

        memcpy(vertices + nextVertex, text.vertices(), sizeof(Vertex) * 4);
        renderer.draw(vbuf.handle(), nextVertex, 4, text.transform.Matrix(), text.tex().handle(),
                      pipeline);
        nextVertex += 4;
    }

    renderer.render();
}

} // namespace Arclight::Systems
