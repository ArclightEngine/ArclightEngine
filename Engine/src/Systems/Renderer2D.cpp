#include <Arclight/Systems/Renderer2D.h>

#include <Arclight/Core/Logger.h>
#include <Arclight/Core/Application.h>
#include <Arclight/Components/Drawable.h>
#include <Arclight/Graphics/Rendering/Renderer.h>
#include <Arclight/Components/Sprite.h>
#include <Arclight/Graphics/Text.h>

namespace Arclight::Systems {

void renderer_2d(float, World& world) {
    Rendering::Renderer& renderer = *Rendering::Renderer::instance();
    renderer.clear();

    auto spriteView = world.registry().view<Sprite>();
    auto textView = world.registry().view<Text>();
    auto drawableView = world.registry().view<Components::Drawable>();

    renderer.bind_pipeline(renderer.default_pipeline().handle());
    for (Entity ent : spriteView) {
        Sprite& sprite = spriteView.get<Sprite>(ent);

        Texture::TextureHandle tex = nullptr;
        if(sprite.texture)
            tex = sprite.texture->Handle();

        renderer.bind_texture(tex);
        renderer.draw(sprite.vertices, 4, sprite.transform.Matrix());
    }

    for(Entity ent : textView) {
        Text& text = textView.get<Text>(ent);

        renderer.bind_texture(text.Tex().Handle());
        renderer.draw(text.Vertices(), 4, text.transform.Matrix());
    }

    renderer.render();
}

} // namespace Arclight::Systems
