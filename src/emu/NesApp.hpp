#ifndef _NESAPP_H_
#define _NESAPP_H_

#include "SDL2/SDL_ttf.h"

#include "NES.hpp"
#include "EmuWindow.hpp"
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

        class ConsoleWindow : public EmuWindow {
            public:
                ConsoleWindow(Console & console);
                virtual ~ConsoleWindow();

                virtual void render();
                virtual void onEvent(SDL_Event* Event);

            private:
                std::string   m_current_input;
                SDL_Renderer* m_sdl_renderer;
                TTF_Font*     m_font; 
                bool          m_console_text_dirty;
                Console&      m_console;
        };

        class CpuWindow : public EmuWindow {
            public:
                CpuWindow();

                virtual void render() {}
                virtual void onEvent(SDL_Event* Event) {}
        };

        class PpuWindow : public EmuWindow {
            public:
                PpuWindow();

                virtual void render() {}
                virtual void onEvent(SDL_Event* Event) {}
        };

    protected:
        static void checkSDLError(bool condition, std::string errmsg);        
        static void checkTTFError(bool condition, std::string errmsg);        

    private:
        bool            m_running;
        NES             m_nes;
        Console         m_console;

        ConsoleWindow*         m_console_window;
        CpuWindow*             m_cpu_window;
        PpuWindow*             m_ppu_window;
        EmuWindow*             m_focused_window;
        std::map<unsigned int, EmuWindow*> m_windows;
};

#endif
