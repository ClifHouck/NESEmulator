#ifndef _NESAPP_H_
#define _NESAPP_H_

#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"

#include "NES.hpp"
#include "utility/Console.hpp"

class NESApp {
    public:
        NESApp();
        int  onExecute();
        bool onInit();
        void onEvent(SDL_Event* Event);
        void onLoop();
        void onCleanup();

        void onRender();
        void renderConsole();
        void renderNESDisplay();

    private:
        void checkSDLError(bool condition, std::string errmsg);        
        void checkTTFError(bool condition, std::string errmsg);        

        bool            m_running;
        bool            m_console_text_dirty;
        SDL_Window*     m_console_window;
        SDL_Renderer*   m_console_renderer;
        SDL_Window*     m_nes_window;
        TTF_Font*       m_font;
        NES             m_nes;
        Console         m_console;
        std::string     m_current_input;
};

#endif
