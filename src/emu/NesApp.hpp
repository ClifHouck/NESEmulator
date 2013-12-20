#ifndef _NESAPP_H_
#define _NESAPP_H_

#include <SDL.h>
#include <SDL_ttf.h>

#include "NES.hpp"
#include "utility/Console.hpp"

class NESApp {
    public:
        NESApp();
        int  onExecute();
        bool onInit();
        void onEvent(SDL_Event* Event);
        void onLoop();
        void onRender();
        void onCleanup();
        void renderConsole();

    private:
        bool            m_running;
        bool            m_console_text_dirty;
        SDL_Surface*    m_display;
        SDL_Surface*    m_text;
        TTF_Font*       m_font;
        NES             m_nes;
        Console         m_console;
};

#endif
