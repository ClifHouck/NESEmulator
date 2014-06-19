#ifndef _NESAPP_H_
#define _NESAPP_H_

#include "SDL2/SDL_ttf.h"

#include "NES.hpp"
#include "EmuWindow.hpp"
#include "utility/Console.hpp"

class NESApp {
    public:
        NESApp(std::string startup_script);
        int  onExecute();
        bool onInit();
        void onEvent(SDL_Event* Event);
        void onLoop();
        void onCleanup();
        void onRender();

        void runStartupScript();

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

        class CpuWindow : public DiagnosticWindow {
            public:
                CpuWindow(const Cpu65XX& cpu);

                virtual void render(); 
                virtual void onEvent(SDL_Event* Event) {}

            private:
                const Cpu65XX& m_cpu;
                SDL_Renderer* m_sdl_renderer;
                TTF_Font*     m_font;
        };

        class PpuWindow : public DiagnosticWindow {
            public:
                // FIXME: We should be taking a const reference here...
                PpuWindow(PPU& ppu);

                virtual void render();
                virtual void onEvent(SDL_Event* Event) {}

            protected:
                PPU& m_ppu;
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

        std::string m_startup_script_name;
};

#endif
