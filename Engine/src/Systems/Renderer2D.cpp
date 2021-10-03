#include <Arclight/Systems/Renderer2D.h>

#include <Arclight/Core/Logger.h>
#include <Arclight/Core/Application.h>
#include <Arclight/Graphics/Rendering/Renderer.h>
#include <Arclight/Graphics/Sprite.h>

namespace Arclight::Systems {

void Renderer2D(float elasped, World& world) {
    Rendering::Renderer& renderer = *Rendering::Renderer::Instance();
    renderer.Clear();

    auto view = world.Registry().view<Sprite>();

    renderer.BindPipeline(renderer.DefaultPipeline().Handle());
    for (Entity ent : view) {
        Sprite& sprite = view.get<Sprite>(ent);

        Texture::TextureHandle tex = nullptr;
        if(sprite.texture)
            tex = sprite.texture->Handle();

        renderer.BindTexture(tex);
        renderer.Draw(sprite.vertices, 4, sprite.transform.Matrix());
    }

    renderer.Render();
}

} // namespace Arclight::Systems
