#include <Arclight/Core/Application.h>
#include <Arclight/Core/Logger.h>
#include <Arclight/Core/ResourceManager.h>
#include <Arclight/ECS/System.h>
#include <Arclight/Graphics/Image.h>
#include <Arclight/Graphics/Rect.h>
#include <Arclight/Graphics/Sprite.h>
#include <Arclight/Graphics/Texture.h>
#include <Arclight/State/State.h>
#include <Arclight/Systems/Renderer2D.h>

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

        transformDefault.SetScale(.5f, .5f);
    }

    static Arclight::Rectf SpriteBounds(const Arclight::Sprite& spr) {
        assert(spr.texture);
        return {spr.transform.GetPosition(), spr.transform.GetPosition() + spr.size};
    }

    void Tick() override {
        auto& app = Arclight::Application::Instance();

        if (Arclight::Input::GetKeyPress(Arclight::KeyCode_E)) {
            Arclight::Entity newEntity = app.World().CreateEntity();

            Arclight::Sprite spr;
            spr.texture = &tex;
            spr.transform.SetPosition(rand() % app.Window().GetWindowRenderSize().x,
                                      rand() % app.Window().GetWindowRenderSize().y);

            app.World().AddComponent<Arclight::Sprite, Arclight::Sprite>(newEntity, std::move(spr));
            app.World().AddComponent<BallData>(newEntity, BallData{{ballSpeed, ballSpeed}});

            ballCount++;
            Arclight::Logger::Debug(ballCount, " balls");
        } else if (Arclight::Input::GetKeyPress(Arclight::KeyCode_R)) {
            int i = 1000; // Add 1000 entities
            while (i--) {
                Arclight::Entity newEntity = app.World().CreateEntity();

                Arclight::Sprite spr;
                spr.texture = &tex;
                spr.transform.SetPosition(rand() % app.Window().GetWindowRenderSize().x,
                                        rand() % app.Window().GetWindowRenderSize().y);

                app.World().AddComponent<Arclight::Sprite, Arclight::Sprite>(newEntity,
                                                                             std::move(spr));
                app.World().AddComponent<BallData>(newEntity, BallData{{ballSpeed, ballSpeed}});
            }

            ballCount += 1000;
            Arclight::Logger::Debug(ballCount, " balls");
        } else if (Arclight::Input::GetKeyPress(Arclight::KeyCode_Q)) {
            auto view = app.World().Registry().view<BallData>();
            for (Arclight::Entity e : view) {
                app.World().Registry().destroy(e);
            }

            ballCount = 0;
            Arclight::Logger::Debug(ballCount, " balls");
        }

        auto view = app.World().Registry().view<Arclight::Sprite, BallData>();
        for (Arclight::Entity e : view) {
            auto& spr = view.get<Arclight::Sprite>(e);
            auto& data = view.get<BallData>(e);

            spr.transform.SetPosition(spr.transform.GetPosition() + data.velocity);

            Arclight::Rect bounds = SpriteBounds(spr);
            if (bounds.right >= app.Window().GetWindowRenderSize().x) {
                data.velocity.x = -ballSpeed;
                spr.transform.SetPosition(app.Window().GetWindowRenderSize().x - bounds.Width(),
                                          bounds.top);
            } else if (bounds.left <= 0) {
                data.velocity.x = ballSpeed;
                spr.transform.SetPosition(0, bounds.top);
            }

            if (bounds.bottom >= app.Window().GetWindowRenderSize().y) {
                data.velocity.y = -ballSpeed;
                spr.transform.SetPosition(bounds.left,
                                          app.Window().GetWindowRenderSize().y - bounds.Height());
            } else if (bounds.top <= 0) {
                data.velocity.y = ballSpeed;
                spr.transform.SetPosition(bounds.left, 0);
            }
        }
    }

    Arclight::Texture tex;
    Arclight::Transform transformDefault;

    const float ballSpeed = 12.f;
    int ballCount = 0;
};

enum { StateDefault };

extern "C" {
void GameInit() {
    Arclight::Logger::Debug("Starting Game!");

    auto& app = Arclight::Application::Instance();
    app.Window().backgroundColour = {0, 0, 0, 255};
    app.AddSystem(new Arclight::Systems::Renderer2D());
    app.AddSystem(new TestSystem());

    app.AddState<StateDefault>();
    app.commands.LoadState<StateDefault>();
}
}
