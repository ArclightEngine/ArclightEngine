#include <Arclight/Core/Logger.h>

extern "C"{
void GameInit(){
    Arclight::Logger::Debug("Starting Game!");

    app.Run();
}
}
