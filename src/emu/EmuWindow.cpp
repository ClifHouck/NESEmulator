#include "EmuWindow.hpp"

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
