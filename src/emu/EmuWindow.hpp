#ifndef EMU_WINDOW_H
#define EMU_WINDOW_H

#include "SDL2/SDL.h"

#include "utility/Commandable.hpp"

#include <cassert>

class EmuWindow : public Commandable {
    public:
        EmuWindow(const char * name,
                  const char * title,
                  unsigned int x, unsigned int y, 
                  unsigned int width, unsigned int height);
        virtual ~EmuWindow();

        virtual void render() = 0;
        virtual void onEvent(SDL_Event* Event) = 0;

        unsigned int id();

        // Commandable interface.
        static const CommandCode SHOW_WINDOW_COMMAND;
        static const CommandCode HIDE_WINDOW_COMMAND;

        virtual CommandResult        receiveCommand(CommandInput command);
        virtual std::string          typeName() { return std::string("EmuWindow"); }

    protected:
        void registerCommands();

        SDL_Window*     m_sdl_window;
};

class DiagnosticWindow : public EmuWindow {
    public:
        DiagnosticWindow(const char * name,
                  const char * title,
                  unsigned int x, unsigned int y, 
                  unsigned int width, unsigned int height) :
            EmuWindow(name, title, x, y, width, height),
            m_sdl_renderer(nullptr)
        {
            m_sdl_renderer = SDL_CreateRenderer(m_sdl_window, -1, SDL_RENDERER_SOFTWARE);
            assert(NULL != m_sdl_renderer && "DiagnosticWindow: SDL_CreateRenderer() failed!");
        }

    protected:
        void render_diagnostic_text(std::string output);

        SDL_Renderer* m_sdl_renderer;
};

#endif //EMU_WINDOW_H
