#ifndef _NESAPP_H_
#define _NESAPP_H_

#include <SDL.h>

#include "NES.hpp"

class NESApp {
    public:
        NESApp();
        int  onExecute();
        bool onInit();
        void onEvent(SDL_Event* Event);
        void onLoop();
        void onRender();
        void onCleanup();

    private:
        bool            m_running;
        SDL_Surface*    m_display;
        NES             m_nes;
};

#endif
