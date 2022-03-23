#pragma once

#include <SDL_keycode.h>

namespace Arclight {
    // It is a given that alphanumeric keycodes map to their ASCII characters e.g. a key = 'a', 0 key = '0'
    enum KeyCode {
        KeyCode_Invalid = 0,

        KeyCode_0 = '0',
        KeyCode_1 = '1',
        KeyCode_2 = '2',
        KeyCode_3 = '3',
        KeyCode_4 = '4',
        KeyCode_5 = '5',
        KeyCode_6 = '6',
        KeyCode_7 = '7',
        KeyCode_8 = '8',
        KeyCode_9 = '9',

        KeyCode_A = 'a',
        KeyCode_B = 'b',
        KeyCode_C = 'c',
        KeyCode_D = 'd',
        KeyCode_E = 'e',
        KeyCode_F = 'f',
        KeyCode_G = 'g',
        KeyCode_H = 'h',
        KeyCode_I = 'i',
        KeyCode_J = 'j',
        KeyCode_K = 'k',
        KeyCode_L = 'l',
        KeyCode_M = 'm',
        KeyCode_N = 'n',
        KeyCode_O = 'o',
        KeyCode_P = 'p',
        KeyCode_Q = 'q',
        KeyCode_R = 'r',
        KeyCode_S = 's',
        KeyCode_T = 't',
        KeyCode_U = 'u',
        KeyCode_V = 'v',
        KeyCode_W = 'w',
        KeyCode_X = 'x',
        KeyCode_Y = 'y',
        KeyCode_Z = 'z',

        KeyCode_Up = SDLK_UP,
        KeyCode_Down = SDLK_DOWN,
        KeyCode_Left = SDLK_LEFT,
        KeyCode_Right = SDLK_RIGHT,

        KeyCode_Space = SDLK_SPACE,

        KeyCode_LShift = SDLK_LSHIFT,
        KeyCode_RShift = SDLK_RSHIFT,
    };
};