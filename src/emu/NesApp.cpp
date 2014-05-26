#include "emu/NesApp.hpp"
#include "emu/SDLKeycodes.hpp"
#include "emu/RenderText.hpp"

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <algorithm>

#include <ctype.h>

const unsigned int CONSOLE_WIDTH        = 256 * 4;
const unsigned int CONSOLE_HEIGHT       = 240 * 2;
const unsigned int NES_DISPLAY_WIDTH    = 256;
const unsigned int NES_DISPLAY_HEIGHT   = 240;

NESApp::
NESApp() :
    m_running (true),
    m_console_window (nullptr)
{
}

int  
NESApp::
onExecute() 
{
    if (onInit() == false) {
        return -1;
    }

    SDL_Event Event;

    while (m_running) {
        while (SDL_PollEvent(&Event)) {
            onEvent(&Event);
        }
        onLoop();
        onRender();
    }

    onCleanup();

    return 0;
}

NESApp::ConsoleWindow::
ConsoleWindow(Console& console) :
    EmuWindow("win-console",
              "NES Command Console",
              5, 5,
              CONSOLE_WIDTH, CONSOLE_HEIGHT),
    m_sdl_renderer (nullptr),
    m_font (nullptr),
    m_console (console),
    m_console_text_dirty (true)
{
    m_sdl_renderer = SDL_CreateRenderer(m_sdl_window, -1, SDL_RENDERER_SOFTWARE);
    checkSDLError(NULL == m_sdl_renderer, "SDL_CreateRenderer() failed: ");
}

NESApp::ConsoleWindow::
~ConsoleWindow()
{
}

void
NESApp::ConsoleWindow::
onEvent(SDL_Event* Event)
{
    if (Event->type == SDL_KEYDOWN) {
        std::cout << "Got keydown event!\n";
        SDL_Keycode keyPressed = Event->key.keysym.sym;
        bool shift = Event->key.keysym.mod & KMOD_SHIFT;
        // Handle general characters.
        if (keyPressed >= SDLK_SPACE && keyPressed <= SDLK_z) {
            std::cout << "Key pressed is a printable char...\n";
            KeycodeMap::iterator it = SDLKeycodesToChar.find(keyPressed);
            if (it != SDLKeycodesToChar.end()) {
                Uint16 unicode_char = it->second;
                char ascii_char = (char)unicode_char;
                if (shift) {
                    ascii_char = (char)toupper(ascii_char); 
                }
                std::cout << "Char: " << ascii_char << "\n";
                m_current_input.push_back(ascii_char);
                // FIXME: Race condition?
                m_console_text_dirty = true;
            }
        }
        // Delete a character from the input.
        else if (keyPressed == SDLK_BACKSPACE ||
                 keyPressed == SDLK_DELETE) {
            if (m_current_input.length() > 0) {
                m_current_input.pop_back();
                m_console_text_dirty = true;
            }
        }
        // Enter the command!
        else if (keyPressed == SDLK_RETURN) {
            m_console.receive_input(m_current_input);
            std::cout << "Entered command!\n";
            m_current_input = std::string();
            // FIXME: Race condition?
            m_console_text_dirty = true;
        }
    }
}

void 
NESApp::ConsoleWindow::
render()
{
    if (m_console_text_dirty) {
        //FIXME: This is gotta be really slow...
        std::stringstream output;
        output << m_console.contents();
        output << "> " << m_current_input;

        // FIXME: Truncate so that only visible lines are sent to the render_text function.
        std::string console_text = output.str();

        SDL_Color text_color = { 255, 255, 255 };
        SDL_Color background_color = { 0, 0, 0 };

        // Clear the screen
        SDL_SetRenderDrawColor(m_sdl_renderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderClear(m_sdl_renderer);

        RenderText::render_text(m_sdl_renderer, 
                0, CONSOLE_HEIGHT - 25,
                &text_color, &background_color,
                console_text,
                true);

        //Update the display
        SDL_RenderPresent(m_sdl_renderer);

        m_console_text_dirty = false;
    }
}

NESApp::CpuWindow::
CpuWindow(const Cpu65XX& cpu) :
    EmuWindow("win-cpu",
              "NES CPU Instrumentation",
              CONSOLE_WIDTH + 5, 5,
              CONSOLE_WIDTH, CONSOLE_HEIGHT),
    m_cpu (cpu)
{
    m_sdl_renderer = SDL_CreateRenderer(m_sdl_window, -1, SDL_RENDERER_SOFTWARE);
    checkSDLError(NULL == m_sdl_renderer, "SDL_CreateRenderer() failed: ");
}

void
NESApp::CpuWindow::
render()
{
    // FIXME: Only render on tick change.. or only when the simulation is paused.
    // Get information from the CPU.
    std::stringstream output;
    output << "Cpu65XX Status" << std::endl
           << std::hex << "A:  0x" << (int)m_cpu.A()  << std::endl
           << std::hex << "X:  0x" << (int)m_cpu.X()  << std::endl
           << std::hex << "Y:  0x" << (int)m_cpu.Y()  << std::endl
           << std::hex << "PC: 0x" << (int)m_cpu.PC() << std::endl
           << std::hex << "S:  0x" << (int)m_cpu.S()  << std::endl;

    std::string cpu_output = output.str();

    // Clear the screen
    SDL_SetRenderDrawColor(m_sdl_renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(m_sdl_renderer);

    SDL_Color text_color = { 255, 255, 255 };
    SDL_Color background_color = { 0, 0, 0 };

    RenderText::render_text(m_sdl_renderer, 
            10, 10,
            &text_color, &background_color,
            cpu_output);

    //Update the display
    SDL_RenderPresent(m_sdl_renderer);
}


NESApp::PpuWindow::
PpuWindow() :
    EmuWindow("win-ppu",
              "NES PPU Instrumentation",
              (CONSOLE_WIDTH + 5) * 2, 5,
              CONSOLE_WIDTH, CONSOLE_HEIGHT)
{}

bool 
NESApp::
onInit()
{
    // Init SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        return false;
    }

    int return_code = TTF_Init();
    checkTTFError(return_code != 0, "TTF_OpenFont() Failed: ");

    // Setup windows.
    m_console_window = new ConsoleWindow(m_console);    
    m_windows[m_console_window->id()] = m_console_window;

    m_focused_window = m_console_window;

    m_cpu_window = new CpuWindow(m_nes.cpu());    
    m_windows[m_cpu_window->id()] = m_cpu_window;

    m_ppu_window = new PpuWindow();    
    m_windows[m_ppu_window->id()] = m_ppu_window;
/*
    if ((m_nes_display = SDL_SetVideoMode(NES_DISPLAY_WIDTH, NES_DISPLAY_HEIGHT, video->vfmt->BitsPerPixel, SDL_OPENGL)) == NULL) { 
        return false; 
    }

    glClearColor(0, 0, 0, 0);
    glClearDepth(1.0f);

    glViewport(0, 0, NES_DISPLAY_WIDTH, NES_DISPLAY_HEIGHT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
 
    glOrtho(0, NES_DISPLAY_WIDTH, NES_DISPLAY_HEIGHT, 0, 1, -1);
 
    glMatrixMode(GL_MODELVIEW);
 
    glEnable(GL_TEXTURE_2D);
 
    glLoadIdentity();
*/

    return true;
}




void 
NESApp::
onEvent(SDL_Event* Event)
{
    if (Event->type == SDL_QUIT) {
        m_running = false;
        return;
    }

    if (Event->type == SDL_WINDOWEVENT_FOCUS_GAINED) {
        m_focused_window = m_windows[Event->window.windowID];
        return;
    }

    // Otherwise send the even to the focused window.
    m_focused_window->onEvent(Event);
}

void 
NESApp::
onLoop()
{
}

void 
NESApp::
onRender()
{
    std::for_each(m_windows.begin(), m_windows.end(), [](std::pair<unsigned int, EmuWindow*> it) { it.second->render(); });
}

void 
NESApp::
onCleanup()
{
    delete m_console_window;
}

void 
NESApp::
checkTTFError(bool condition, std::string errmsg)
{
    if (condition) {
        std::cerr << errmsg << TTF_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        exit(1);
    }
}

void
NESApp::
checkSDLError(bool condition, std::string errmsg)
{
    if (condition) {
        std::cerr << errmsg << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        exit(1);
    }
}
