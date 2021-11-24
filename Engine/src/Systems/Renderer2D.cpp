#include <Arclight/Systems/Renderer2D.h>

#include <Arclight/Core/Logger.h>
#include <Arclight/Core/Application.h>
#include <Arclight/Graphics/Rendering/Renderer.h>
#include <Arclight/Graphics/Sprite.h>
#include <Arclight/Graphics/Text.h>

namespace Arclight::Systems {

void Renderer2D(float, World& world) {
    Rendering::Renderer& renderer = *Rendering::Renderer::instance();
    renderer.Clear();

    auto spriteView = world.Registry().view<Sprite>();
    auto textView = world.Registry().view<Text>();

    renderer.BindPipeline(renderer.DefaultPipeline().Handle());
    for (Entity ent : spriteView) {
        Sprite& sprite = spriteView.get<Sprite>(ent);

        Texture::TextureHandle tex = nullptr;
        if(sprite.texture)
            tex = sprite.texture->Handle();

        renderer.BindTexture(tex);
        renderer.Draw(sprite.vertices, 4, sprite.transform.Matrix());
    }

    for(Entity ent : textView) {
        Text& text = textView.get<Text>(ent);

        renderer.BindTexture(text.Tex().Handle());
        renderer.Draw(text.Vertices(), 4, text.transform.Matrix());
    }

    renderer.Render();
}

} // namespace Arclight::Systems
