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

#define BALL_SIZE 32

class TestSystem {
public:
    struct BallData {
        Arclight::Vector2f velocity;
    };

    TestSystem() {
        std::shared_ptr<Arclight::Image> res =
            Arclight::ResourceManager::Instance().GetResource<Arclight::Image>("ball.png");
        assert(res.get());

        tex.Load(*res);
    }

    static Arclight::Rectf SpriteBounds(const Arclight::Sprite& spr) {
        assert(spr.texture);
        return {spr.transform.GetPosition(), spr.transform.GetPosition() + spr.PixelSize()};
    }

    void Tick(float elapsed, Arclight::World& world) {
        auto& app = Arclight::Application::Instance();

        if (Arclight::Input::GetKeyPress(Arclight::KeyCode_E)) {
            Arclight::Entity newEntity = world.CreateEntity();

            Arclight::Sprite spr = Arclight::CreateSprite({BALL_SIZE});
            spr.texture = &tex;
            spr.transform.SetPosition(rand() % app.Window().GetWindowRenderSize().x,
                                        rand() % app.Window().GetWindowRenderSize().y);

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
                spr.transform.SetPosition(rand() % app.Window().GetWindowRenderSize().x,
                                            rand() % app.Window().GetWindowRenderSize().y);

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

struct StdoutFPSCounter {
    void Tick(float elasped, Arclight::World&) {
        accum += elasped;
        fCount++;

        // Print the framerate every 2 seconds
        if(accum > 2.f && fCount){
            Arclight::Logger::Debug(fCount / accum, " fps");

            fCount = 0;
            accum = 0;
        }
    }

    float accum = 0.f;
    int fCount = 0;
};

enum { StateDefault };

extern "C" {
void GameInit() {
    Arclight::Logger::Debug("Starting Game!");

    TestSystem testSystem;
    StdoutFPSCounter fpsCounter;

    auto& app = Arclight::Application::Instance();
    app.Window().backgroundColour = {0, 0, 0, 255};
    app.AddSystem<Arclight::Systems::Renderer2D>();
    app.AddSystem<StdoutFPSCounter, &StdoutFPSCounter::Tick>(fpsCounter);
    app.AddSystem<TestSystem, &TestSystem::Tick>(testSystem);

    app.AddState<StateDefault>();
    app.commands.LoadState<StateDefault>();

    app.Run();
}
}
