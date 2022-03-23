#include <Arclight/Core/Application.h>
#include <Arclight/Core/Logger.h>

ARCLIGHT_GAME_INIT() {
    auto& app = Arclight::Application::instance();
    Arclight::Logger::Debug("Starting Game!");

    app.run();
}
