#include "graphics.h"

#include <SDL.h>

interpreter_8::Graphics::Graphics() :
    m_window(nullptr), 
    m_texture(nullptr),
    m_renderer(nullptr),
    m_keymap({
        { SDLK_x,  0u  }, { SDLK_1,  1u  }, { SDLK_2,  2u  }, { SDLK_3,  3u  },
        { SDLK_q,  4u  }, { SDLK_w,  5u  }, { SDLK_e,  6u  }, { SDLK_a,  7u  },
        { SDLK_s,  8u  }, { SDLK_d,  9u  }, { SDLK_z,  10u }, { SDLK_c,  11u },
        { SDLK_4,  12u }, { SDLK_r,  13u }, { SDLK_f,  14u }, { SDLK_v,  15u }
    })
{ }

interpreter_8::Graphics::~Graphics() {
    SDL_DestroyTexture(m_texture);
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

void interpreter_8::Graphics::Init(const char* title, int width, int height, int scale) {
    auto format = SDL_PIXELFORMAT_ABGR8888;
    auto access = SDL_TEXTUREACCESS_STREAMING;
    unsigned int pos = SDL_WINDOWPOS_CENTERED;

    // Initialize components
    SDL_Init(SDL_INIT_VIDEO);
    m_window = SDL_CreateWindow(title, pos, pos, width * scale, height * scale, SDL_WINDOW_SHOWN);
    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
    m_texture = SDL_CreateTexture(m_renderer, format, access, width, height);
}

void interpreter_8::Graphics::Update(const void* graphicsBuffer, int videoPitch) {
    SDL_UpdateTexture(m_texture, nullptr, graphicsBuffer, videoPitch);
    SDL_SetTextureColorMod(m_texture, 0, 255, 0); // Set main game color: TODO - allow user to change color???
    SDL_RenderClear(m_renderer);
    SDL_RenderCopy(m_renderer, m_texture, nullptr, nullptr);
    SDL_RenderPresent(m_renderer);
}

bool interpreter_8::Graphics::HandleInput(uint8_t* keys) {
    bool running = true;
    SDL_Event event;
    int key;

    auto valid = [&](int key) -> bool { return m_keymap.find(key) != m_keymap.end(); };
    auto at = [&](int key) -> int { return valid(key) ? m_keymap[key] : -1; };

    // Process events
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_KEYDOWN:
                key = event.key.keysym.sym;
                running = (key != SDLK_ESCAPE);
                if (running && valid(key)) keys[at(key)] = 1;
                break;
            case SDL_KEYUP:
                key = event.key.keysym.sym;
                if (valid(key)) keys[at(key)] = 0;
                break;
        }
    }
    return running;
}
