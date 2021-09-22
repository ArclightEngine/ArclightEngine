#include <Arclight/Core/Logger.h>
#include <Arclight/Core/Node.h>
#include <Arclight/Core/ResourceManager.h>
#include <Arclight/Graphics/Image.h>
#include <Arclight/Graphics/Sprite.h>
#include <Arclight/Graphics/Texture.h>

Arclight::Sprite* test;
Arclight::Texture* tex;

extern "C" {
void GameInit() {
    Arclight::Logger::Debug("Starting Game!");

    std::shared_ptr<Arclight::Image> res =
        Arclight::ResourceManager::Instance().GetResource<Arclight::Image>("art.png");
    assert(res.get());

    tex = new Arclight::Texture(*res);
    test = new Arclight::Sprite(*tex);
}

void GameRun() {

}
}
