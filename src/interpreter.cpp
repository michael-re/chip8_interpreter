#include "interpreter.h"
#include "graphics.h"

#include <chrono>

interpreter_8::Interpreter::Interpreter(const char* title, const char* rom, int delay, int scale) :
    m_runFlag(false), m_scale(0), m_cycleDelay(0), m_videoPitch(0), m_chip8(nullptr), m_window(nullptr)
{
    m_chip8 = new Chip8();
    m_window = new Graphics();

    int width = Chip8::video::WIDTH;
    int height = Chip8::video::HEIGHT;

    // The number of bytes in a row of pixel data, including padding between lines
    m_videoPitch = sizeof(m_chip8->graphicsBuffer()[0]) * Chip8::video::WIDTH;
    m_scale = (scale < 40 ? scale : 40);
    m_cycleDelay = delay;

    // Create window
    m_window->Init(title, width, height, m_scale);

    // Initialize chip8 and try to load rom
    m_runFlag = m_chip8->Init() && m_chip8->LoadROM(rom);
}

interpreter_8::Interpreter::~Interpreter() {
    delete m_window;
    delete m_chip8;

    m_window = nullptr;
    m_chip8 = nullptr;
}

void interpreter_8::Interpreter::Run() {
    if (!m_runFlag) {
        std::cerr << "Error: failed to initialize chip8 interpreter." << std::endl;
        return;
    }

    // Easier time working with chrono
    using clock = std::chrono::high_resolution_clock;
    using period = std::chrono::milliseconds::period;
    using duration = std::chrono::duration<double, period>;

    bool running = true;
    auto currentTime = clock::now();
    auto previousTime = clock::now();

    while (running) {
        // Process user input
        running = m_window->HandleInput(m_chip8->keyBuffer());

        // Get elapsed time
        currentTime = clock::now();
        double deltaTime = duration(currentTime - previousTime).count();

        // Use delta time to control game framerate
        if (deltaTime > m_cycleDelay) {
            // Update previous time
            previousTime = currentTime;

            // Run new emulation cycle
            m_chip8->RunCycle();

            // Update screen
            m_window->Update(m_chip8->graphicsBuffer(), m_videoPitch);
        }
    }
}
