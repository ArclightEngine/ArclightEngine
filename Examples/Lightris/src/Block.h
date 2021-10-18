#pragma once

#include <Arclight/Colour.h>

#define BLOCK_SIZE 32

enum {
    BlockEmpty,  // Nothing
    BlockRed,    // Z piece
    BlockGreen,  // S piece
    BlockYellow, // Square piece
    BlockOrange, // L piece
    BlockBlue,   // J piece
    BlockCyan,   // I piece
    BlockPurple, // T piece
};

constexpr Arclight::Colour blockColours[8] = {
    {0, 0, 0, 0},       {255, 0, 0, 255}, {0, 255, 0, 255},   {255, 255, 0, 255},
    {255, 128, 0, 255}, {0, 0, 255, 255}, {0, 255, 255, 255}, {255, 0, 255, 255},
};
