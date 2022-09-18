#pragma once

#include <cstdint>
#include <unordered_map>

// #include <SDL.h>
// #undef main // SDL main 
class SDL_Window;
class SDL_Texture;
class SDL_Renderer;

namespace interpreter_8 {
    class Graphics final {
    private:
        SDL_Window* m_window;
        SDL_Texture* m_texture;
        SDL_Renderer* m_renderer;

        // keymap for chip8 - TODO: implement feature to allow user to remap keys from default
        std::unordered_map<int32_t, int> m_keymap;

        // @brief returns if the given key is part of control
        inline bool valid(const int32_t& key) {
            return m_keymap.find(key) != m_keymap.end();
        }

        // @brief returns index of given key
        inline int index(const int32_t& key) {
            return valid(key) ? m_keymap.at(key) : -1;
        }

    public:
        Graphics();
        ~Graphics();

        /**
         * @brief initialize SDL components and create window to displayer to user.
         *
         * @param title  - window title.
         * @param width  - horizontal pixel count.
         * @param height - vertical pixel count.
         * @param scale  - how much to scale window. If given 64x32 pixel display size and scale
         *                 value of 10, the final window size will be 640x320p.
         */
        void Init(const char* title, int width, int height, int scale = 10);

        /**
         * @brief Updates screen with new pixel data
         *
         * @param graphicsBuffer - Pointer containing texture to update texture with. For this interpreter, the
         *                         texture is an array that contains the pixel data to show to user.
         * @param videoPitch     - the number of bytes in a row of pixel data, including padding between lines
         */
        void Update(const void* graphicsBuffer, int videoPitch);

        /**
         * @brief Processes user input and updates the value of given keys to current user input.
         *
         * @param keys  - key buffer to update user input with
         * @return true - if user wants to keep running the program, false otherwise
         */
        bool HandleInput(uint8_t* keys);

        // @brief Free window, renderer, and texture
        void CleanResources();
    };
}
