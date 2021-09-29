#include <Arclight/Systems/Renderer2D.h>

#include <Arclight/Core/Application.h>
#include <Arclight/Graphics/Rendering/Renderer.h>
#include <Arclight/Graphics/Sprite.h>

namespace Arclight::Systems {

void Renderer2D(float elasped, World& world) {
    Rendering::Renderer& renderer = *Rendering::Renderer::Instance();
    renderer.Clear();

    auto view = world.Registry().view<Sprite>();
    for (Entity ent : view) {
        Sprite& sprite = view.get<Sprite>(ent);

        assert(sprite.texture);
        sprite.size = Vector2f::Scale(sprite.texture->FloatSize(), sprite.transform.GetScale());

        sprite.vertices[0].position = { 0, static_cast<float>(sprite.texture->Size().y) };
		sprite.vertices[1].position = { 0, 0 };
		sprite.vertices[2].position = { static_cast<float>(sprite.texture->Size().x), static_cast<float>(sprite.texture->Size().y) };
		sprite.vertices[3].position = { static_cast<float>(sprite.texture->Size().x), 0 };
        
        renderer.Draw(sprite.vertices, 4, sprite.transform.Matrix(), sprite.texture->Handle());
    }

    renderer.Render();
}

} // namespace Arclight::Systems