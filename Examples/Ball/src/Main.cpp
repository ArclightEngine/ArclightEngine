#include <Arclight/Components/Sprite.h>
#include <Arclight/Core/Application.h>
#include <Arclight/Core/Logger.h>
#include <Arclight/Core/ResourceManager.h>
#include <Arclight/ECS/System.h>
#include <Arclight/Graphics/Image.h>
#include <Arclight/Graphics/Rect.h>
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
            Arclight::Entity newEntity = world.create_entity();

            Arclight::Sprite spr = Arclight::CreateSprite({BALL_SIZE});
            spr.texture = &tex;
            spr.transform.SetPosition(rand() % app.window().GetWindowRenderSize().x,
                                      rand() % app.window().GetWindowRenderSize().y);

            world.add_component<Arclight::Sprite, Arclight::Sprite>(newEntity, std::move(spr));
            world.add_component<BallData>(newEntity, BallData{{ballSpeed * ((rand() & 2) - 1),
                                                              ballSpeed * ((rand() & 2) - 1)}});

            ballCount++;
            Arclight::Logger::Debug("{} balls", ballCount);
        } else if (Arclight::Input::GetKeyPress(Arclight::KeyCode_R)) {
            int i = 1000; // Add 1000 entities
            while (i--) {
                Arclight::Entity newEntity = world.create_entity();

                Arclight::Sprite spr = Arclight::CreateSprite({BALL_SIZE});
                spr.texture = &tex;
                spr.transform.SetPosition(rand() % app.window().GetWindowRenderSize().x,
                                          rand() % app.window().GetWindowRenderSize().y);

                world.add_component<Arclight::Sprite, Arclight::Sprite>(newEntity, std::move(spr));
                world.add_component<BallData>(newEntity, BallData{{ballSpeed * ((rand() & 2) - 1),
                                                                  ballSpeed * ((rand() & 2) - 1)}});
            }

            ballCount += 1000;
            Arclight::Logger::Debug("{} balls", ballCount);
        } else if (Arclight::Input::GetKeyPress(Arclight::KeyCode_Q)) {
            auto view = world.registry().view<BallData>();
            for (Arclight::Entity e : view) {
                world.destroy_entity(e);
            }

            ballCount = 0;
            Arclight::Logger::Debug("{} balls", ballCount);
        }

        auto view = world.registry().view<Arclight::Sprite, BallData>();
        for (Arclight::Entity e : view) {
            auto& spr = view.get<Arclight::Sprite>(e);
            auto& data = view.get<BallData>(e);

            spr.transform.SetPosition(spr.transform.GetPosition() + data.velocity);

            Arclight::Rect bounds = SpriteBounds(spr);
            if (bounds.right >= app.window().get_render_size().x) {
                data.velocity.x = -ballSpeed;
                spr.transform.SetPosition(app.window().get_render_size().x - bounds.width(),
                                          bounds.top);
            } else if (bounds.left <= 0) {
                data.velocity.x = ballSpeed;
                spr.transform.SetPosition(0, bounds.top);
            }

            if (bounds.bottom >= app.window().get_render_size().y) {
                data.velocity.y = -ballSpeed;
                spr.transform.SetPosition(bounds.left,
                                          app.window().get_render_size().y - bounds.height());
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
    app.add_system<Arclight::Systems::renderer_2d>();
    app.add_system<Arclight::StdoutFPSCounter, &Arclight::StdoutFPSCounter::Tick>(fpsCounter);
    app.add_system<TestSystem, &TestSystem::Tick>(testSystem);

    app.add_state<StateDefault>();
    app.commands.load_state<StateDefault>();

    app.run();
}
}
