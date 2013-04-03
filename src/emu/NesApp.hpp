#ifndef _NESAPP_H_
#define _NESAPP_H_

#include <SDL.h>

#include "CPU/Cpu65XX.hpp"
#include "PPU/PPU.hpp"

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
        Cpu65XX         m_cpu;
        PPU             m_ppu;
};

#endif
