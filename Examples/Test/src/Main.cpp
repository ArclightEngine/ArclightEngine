#include <Arclight/Core/Application.h>
#include <Arclight/Core/Logger.h>
#include <Arclight/Core/ResourceManager.h>
#include <Arclight/ECS/System.h>
#include <Arclight/Graphics/Image.h>
#include <Arclight/Graphics/Sprite.h>
#include <Arclight/Graphics/Texture.h>
#include <Arclight/State/State.h>

class TestSystem : public Arclight::System {
public:
    struct BallData {
        Arclight::Vector2f velocity;
    };

    TestSystem() : System("TestSystem") {
        std::shared_ptr<Arclight::Image> res =
            Arclight::ResourceManager::Instance().GetResource<Arclight::Image>("ball.png");
        assert(res.get());

        tex.Load(*res);
        spr.SetTexture(tex);
        spr.SetScale(0.5f, 0.5f);
    }

    void Tick() override {
        auto& app = Arclight::Application::Instance();

        if(Arclight::Input::GetKeyPress(Arclight::KeyCode_E)){
            Arclight::Entity newEntity = app.World().CreateEntity();

            app.World().AddComponent<Arclight::Sprite, Arclight::Sprite>(newEntity, Arclight::Sprite(spr));
            app.World().AddComponent<BallData>(newEntity, BallData{{ballSpeed, ballSpeed}});
        }

        auto view = app.World().Registry().view<Arclight::Sprite, BallData>();
        for(Arclight::Entity e : view){
            auto& spr = view.get<Arclight::Sprite>(e);
            auto& data = view.get<BallData>(e);

            spr.Move(data.velocity);

            if(spr.Bounds().right >= app.Window().GetWindowRenderSize().x){
                data.velocity.x = -ballSpeed;
                spr.SetPosition(app.Window().GetWindowRenderSize().x - spr.Bounds().Width(), spr.Bounds().top);
            } else if(spr.Bounds().left <= 0){
                data.velocity.x = ballSpeed;
                spr.SetPosition(0, spr.Bounds().top);
            }

            if(spr.Bounds().bottom >= app.Window().GetWindowRenderSize().y){
                data.velocity.y = -ballSpeed;
                spr.SetPosition(spr.Bounds().left, app.Window().GetWindowRenderSize().y - spr.Bounds().Height());
            } else if(spr.Bounds().top <= 0){
                data.velocity.y = ballSpeed;
                spr.SetPosition(spr.Bounds().left, 0);
            }
        }
    }

    Arclight::Texture tex;
    Arclight::Sprite spr;

    const float ballSpeed = 12.f;
};

enum {
    StateDefault
};

extern "C" {
void GameInit() {
    Arclight::Logger::Debug("Starting Game!");

    auto& app = Arclight::Application::Instance();
    app.Window().backgroundColour = {0, 0, 0, 255};

    app.AddSystem(new TestSystem());

    app.AddState<StateDefault>();
    app.commands.LoadState<StateDefault>();
}
}
