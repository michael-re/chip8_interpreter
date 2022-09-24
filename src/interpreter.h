#pragma once

#include "chip8.h"
#include "graphics.h"

namespace interpreter_8 {
    class Interpreter {
    private:
        bool m_runFlag;
        int m_scale;
        int m_cycleDelay;
        int m_videoPitch;

        Chip8 m_chip8;
        Graphics m_window;

    public:
        Interpreter(const char* title, const char* rom, int delay, int scale);
        void Run();
    };
}
