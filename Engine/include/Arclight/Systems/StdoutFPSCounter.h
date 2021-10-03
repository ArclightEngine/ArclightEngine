#pragma once

#include <Arclight/Core/Logger.h>
#include <Arclight/ECS/World.h>

namespace Arclight {
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
}