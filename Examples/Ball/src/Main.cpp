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
#include <Arclight/Systems/StdoutFPSCounter.h>

#define BALL_SIZE 32

class TestSystem {
public:
    struct BallData {
        Arclight::Vector2f velocity;
    };

    TestSystem() {
        std::shared_ptr<Arclight::Image> res =
            Arclight::ResourceManager::instance().GetResource<Arclight::Image>("ball.png");
        assert(res.get());

        tex.Load(*res);
    }

    static Arclight::Rectf SpriteBounds(const Arclight::Sprite& spr) {
        assert(spr.texture);
        return {spr.transform.GetPosition(), spr.transform.GetPosition() + spr.PixelSize()};
    }

    void Tick(float elapsed, Arclight::World& world) {
        auto& app = Arclight::Application::instance();

        if (Arclight::Input::GetKeyPress(Arclight::KeyCode_E)) {
            Arclight::Entity newEntity = world.CreateEntity();

            Arclight::Sprite spr = Arclight::CreateSprite({BALL_SIZE});
            spr.texture = &tex;
            spr.transform.SetPosition(rand() % app.window().GetWindowRenderSize().x,
                                        rand() % app.window().GetWindowRenderSize().y);

            world.AddComponent<Arclight::Sprite, Arclight::Sprite>(newEntity,
                                                                            std::move(spr));
            world.AddComponent<BallData>(
                newEntity,
                BallData{{ballSpeed * ((rand() & 2) - 1), ballSpeed * ((rand() & 2) - 1)}});

            ballCount++;
            Arclight::Logger::Debug(ballCount, " balls");
        } else if (Arclight::Input::GetKeyPress(Arclight::KeyCode_R)) {
            int i = 1000; // Add 1000 entities
            while (i--) {
                Arclight::Entity newEntity = world.CreateEntity();

                Arclight::Sprite spr = Arclight::CreateSprite({BALL_SIZE});
                spr.texture = &tex;
                spr.transform.SetPosition(rand() % app.window().GetWindowRenderSize().x,
                                            rand() % app.window().GetWindowRenderSize().y);

                world.AddComponent<Arclight::Sprite, Arclight::Sprite>(newEntity,
                                                                             std::move(spr));
                world.AddComponent<BallData>(
                    newEntity,
                    BallData{{ballSpeed * ((rand() & 2) - 1), ballSpeed * ((rand() & 2) - 1)}});
            }

            ballCount += 1000;
            Arclight::Logger::Debug(ballCount, " balls");
        } else if (Arclight::Input::GetKeyPress(Arclight::KeyCode_Q)) {
            auto view = world.Registry().view<BallData>();
            for (Arclight::Entity e : view) {
                world.DestroyEntity(e);
            }

            ballCount = 0;
            Arclight::Logger::Debug(ballCount, " balls");
        }

        auto view = world.Registry().view<Arclight::Sprite, BallData>();
        for (Arclight::Entity e : view) {
            auto& spr = view.get<Arclight::Sprite>(e);
            auto& data = view.get<BallData>(e);

            spr.transform.SetPosition(spr.transform.GetPosition() + data.velocity);

            Arclight::Rect bounds = SpriteBounds(spr);
            if (bounds.right >= app.window().GetWindowRenderSize().x) {
                data.velocity.x = -ballSpeed;
                spr.transform.SetPosition(app.window().GetWindowRenderSize().x - bounds.Width(),
                                          bounds.top);
            } else if (bounds.left <= 0) {
                data.velocity.x = ballSpeed;
                spr.transform.SetPosition(0, bounds.top);
            }

            if (bounds.bottom >= app.window().GetWindowRenderSize().y) {
                data.velocity.y = -ballSpeed;
                spr.transform.SetPosition(bounds.left,
                                          app.window().GetWindowRenderSize().y - bounds.Height());
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

    TestSystem testSystem;
    Arclight::StdoutFPSCounter fpsCounter;

    auto& app = Arclight::Application::instance();
    app.window().backgroundColour = {0, 0, 0, 255};
    app.window().SetSize({640, 480});
    app.add_system<Arclight::Systems::Renderer2D>();
    app.add_system<Arclight::StdoutFPSCounter, &Arclight::StdoutFPSCounter::Tick>(fpsCounter);
    app.add_system<TestSystem, &TestSystem::Tick>(testSystem);

    app.add_state<StateDefault>();
    app.commands.load_state<StateDefault>();

    app.run();
}
}
