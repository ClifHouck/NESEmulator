#include "EmuWindow.hpp"
#include "RenderText.hpp"

const CommandCode EmuWindow::SHOW_WINDOW_COMMAND = 1;
const CommandCode EmuWindow::HIDE_WINDOW_COMMAND = 2;

EmuWindow::
EmuWindow(const char * name,
          const char * title,
          unsigned int x, unsigned int y, 
          unsigned int width, unsigned int height) :
    Commandable(std::string(name))
{
    registerCommands();
    m_sdl_window   = SDL_CreateWindow(title,
                          x, y,
                          width, height,
                          0);
}
    
EmuWindow::
~EmuWindow() 
{
    SDL_DestroyWindow(m_sdl_window);
}

unsigned int
EmuWindow::
id() 
{
    return SDL_GetWindowID(m_sdl_window);
}

void
EmuWindow::
registerCommands()
{
    Command show_window_command;
    show_window_command.m_keyword = "show";
    show_window_command.m_helpText = "Show/display the window.";
    show_window_command.m_code     = SHOW_WINDOW_COMMAND;
    show_window_command.m_numArguments = 0;
    addCommand(show_window_command);

    Command hide_window_command;
    hide_window_command.m_keyword = "hide";
    hide_window_command.m_helpText = "Hide the window.";
    hide_window_command.m_code     = HIDE_WINDOW_COMMAND;
    hide_window_command.m_numArguments = 0;
    addCommand(hide_window_command);
}

CommandResult
EmuWindow::
receiveCommand(CommandInput command)
{
    CommandResult result;

    switch (command.m_code) {
        case SHOW_WINDOW_COMMAND:
            {
                SDL_ShowWindow(m_sdl_window);
                result.m_code = CommandResult::ResultCode::OK;
            }
           break; 
        case HIDE_WINDOW_COMMAND:
           {
                SDL_HideWindow(m_sdl_window);
                result.m_code = CommandResult::ResultCode::OK;
           }
           break;
    }

    return result;
}

void
DiagnosticWindow::
render_diagnostic_text(std::string output)
{
    // Clear the screen
    SDL_SetRenderDrawColor(m_sdl_renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(m_sdl_renderer);

    SDL_Color text_color = { 255, 255, 255 };
    SDL_Color background_color = { 0, 0, 0 };

    RenderText::render_text(m_sdl_renderer, 
            10, 10,
            &text_color, &background_color,
            output);

    //Update the display
    SDL_RenderPresent(m_sdl_renderer);
}
