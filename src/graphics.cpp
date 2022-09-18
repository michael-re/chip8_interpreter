#include "graphics.h"

#include <SDL.h>

interpreter_8::Graphics::Graphics() : 
    m_window(nullptr), 
    m_texture(nullptr),
    m_renderer(nullptr),
    m_keymap({
        { SDL_KeyCode::SDLK_x,  0u  }, { SDL_KeyCode::SDLK_1,  1u  },
        { SDL_KeyCode::SDLK_2,  2u  }, { SDL_KeyCode::SDLK_3,  3u  },
        { SDL_KeyCode::SDLK_q,  4u  }, { SDL_KeyCode::SDLK_w,  5u  },
        { SDL_KeyCode::SDLK_e,  6u  }, { SDL_KeyCode::SDLK_a,  7u  },
        { SDL_KeyCode::SDLK_s,  8u  }, { SDL_KeyCode::SDLK_d,  9u  },
        { SDL_KeyCode::SDLK_z,  10u }, { SDL_KeyCode::SDLK_c,  11u },
        { SDL_KeyCode::SDLK_4,  12u }, { SDL_KeyCode::SDLK_r,  13u },
        { SDL_KeyCode::SDLK_f,  14u }, { SDL_KeyCode::SDLK_v,  15u }
    })
{ }

interpreter_8::Graphics::~Graphics() {
    CleanResources();
}

void interpreter_8::Graphics::Init(const char* title, int width, int height, int scale) {
    int index = -1;
    auto format = SDL_PIXELFORMAT_ABGR8888;
    auto access = SDL_TEXTUREACCESS_STREAMING;
    unsigned int pos = SDL_WINDOWPOS_CENTERED;

    // Initialize components
    SDL_Init(SDL_INIT_VIDEO);
    m_window = SDL_CreateWindow(title, pos, pos, width * scale, height * scale, SDL_WINDOW_SHOWN);
    m_renderer = SDL_CreateRenderer(m_window, index, SDL_RENDERER_ACCELERATED);
    m_texture = SDL_CreateTexture(m_renderer, format, access, width, height);
}

void interpreter_8::Graphics::Update(const void* graphicsBuffer, int videoPitch) {
    SDL_UpdateTexture(m_texture, nullptr, graphicsBuffer, videoPitch);
    // Set main game color: TODO - allow user to change color???
    SDL_SetTextureColorMod(m_texture, 255, 150, 100);
    SDL_RenderClear(m_renderer);
    SDL_RenderCopy(m_renderer, m_texture, nullptr, nullptr);
    SDL_RenderPresent(m_renderer);
}

bool interpreter_8::Graphics::HandleInput(uint8_t* keys) {
    bool running = true;
    SDL_Event event;
    SDL_Keycode key;

    // Process events
    if (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_KEYDOWN:
                key = event.key.keysym.sym;
                running = (key != SDL_KeyCode::SDLK_ESCAPE);

                if (running && valid(key)) {
                    keys[index(key)] = 1;
                }
                break;
            case SDL_KEYUP:
                key = event.key.keysym.sym;

                if (valid(key)) {
                    keys[index(key)] = 0;
                }
                break;
        }
    }
    return running;
}

void interpreter_8::Graphics::CleanResources() {
    SDL_DestroyTexture(m_texture);
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}
