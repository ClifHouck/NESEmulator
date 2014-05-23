#include "emu/NesApp.hpp"
#include "emu/SDLKeycodes.hpp"

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

    // Load a font
    m_font = TTF_OpenFont("Inconsolata.ttf", 20);
    checkTTFError(NULL == m_font, "TTF_OpenFont() Failed: ");

/*  return_code = SDL_SetRenderDrawBlendMode(m_console_renderer, SDL_BLENDMODE_ADD);
    checkSDLError(0 != return_code, "SDL_SetRenderDrawBlendMode() failed: "); */

}

NESApp::ConsoleWindow::
~ConsoleWindow()
{
    // TODO: Destroy font...
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

/* 
   FIXME - Move somewhere more appropriate.
   Shamelessly stolen from 
   http://stackoverflow.com/questions/236129/how-to-split-a-string-in-c 
*/
static 
std::vector<std::string> 
&split(const std::string &s, char delim, std::vector<std::string> &elems) 
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

static
std::vector<std::string> 
split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

void 
NESApp::ConsoleWindow::
render()
{
    std::vector<SDL_Surface*> text_surfaces;
    if (m_console_text_dirty) {

        //FIXME: This is gotta be really slow...
        std::stringstream output;
        output << m_console.contents();
        output << "> " << m_current_input;

        std::string console_text = output.str();

        std::vector<std::string> lines = split(console_text, '\n');

        SDL_Color text_color = { 255, 255, 255 };
        SDL_Color background_color = { 0, 0, 0 };

        // Render each line to a surface.
        std::for_each(lines.begin(), lines.end(), [this, background_color, text_color, &text_surfaces](std::string line) {
            // RenderText doesn't like empty strings.
            if (line.length() == 0) {
                line = std::string("   ");
            }

            // Write text to surface
            SDL_Surface* surface = TTF_RenderText_Shaded(m_font, line.c_str(), text_color, background_color);
            checkTTFError(NULL == surface, "TTF_RenderText_Solid() Failed: "); 
            text_surfaces.push_back(surface);
        });

        // Clear the screen
        SDL_SetRenderDrawColor(m_sdl_renderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderClear(m_sdl_renderer);

        SDL_Rect dest_rect {};
        dest_rect.x = 0;
        dest_rect.y = CONSOLE_HEIGHT - 25;
        int lines_to_render = CONSOLE_HEIGHT / 15;
        auto it = text_surfaces.rbegin();
        std::vector<SDL_Texture*> textures;
        // Render visible lines starting from the end of the console.
        for (int n = 0; n < lines_to_render; ++n) {
            if (it == text_surfaces.rend()) {
                break;
            }
            
            SDL_Texture *text_texture = SDL_CreateTextureFromSurface(m_sdl_renderer, *it);
            checkSDLError(NULL == text_texture, "SDL_CreateTextureFromSurface Failed: ");
            textures.push_back(text_texture);

            // Apply the text to the display
            dest_rect.w = (*it)->w;
            dest_rect.h = (*it)->h;
            SDL_Rect src_rect = { 0, 0, (*it)->w, (*it)->h };
            int return_code = SDL_RenderCopy(m_sdl_renderer, text_texture, &src_rect, &dest_rect);
            checkSDLError(0 != return_code, "SDL_RenderCopy Failed: ");
            
            dest_rect.y -= 20;
            ++it;
        }

        //Update the display
        SDL_RenderPresent(m_sdl_renderer);

        // Clean up surfaces and textures.
        // TODO: Only clean up surfaces and textures that we don't need anymore (ie offscreen).
        std::for_each(text_surfaces.begin(), text_surfaces.end(), [](SDL_Surface * surface) {
            SDL_FreeSurface(surface);
        });
        std::for_each(textures.begin(), textures.end(), [](SDL_Texture * texture) {
            SDL_DestroyTexture(texture);
        });

        m_console_text_dirty = false;
    }
}

NESApp::CpuWindow::
CpuWindow() :
    EmuWindow("win-cpu",
              "NES CPU Instrumentation",
              CONSOLE_WIDTH + 5, 5,
              CONSOLE_WIDTH, CONSOLE_HEIGHT)
{}

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
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        return false;
    }

    int return_code = TTF_Init();
    checkTTFError(return_code != 0, "TTF_OpenFont() Failed: ");

    m_console_window = new ConsoleWindow(m_console);    
    m_windows[m_console_window->id()] = m_console_window;

    m_focused_window = m_console_window;

    m_cpu_window = new CpuWindow();    
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
        EmuWindow* window = m_windows[Event->window.windowID];
        m_focused_window  = window;
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
    std::for_each(m_windows.begin(), m_windows.end(), [this](std::pair<unsigned int, EmuWindow*> it) { it.second->render(); });
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
