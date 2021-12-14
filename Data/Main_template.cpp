#include <Arclight/Core/Application.h>
#include <Arclight/Core/Logger.h>

extern "C" {
void GameInit() {
    auto& app = Arclight::Application::instance();
    Arclight::Logger::Debug("Starting Game!");

    app.Run();
}
}
