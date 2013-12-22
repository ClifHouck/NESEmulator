#include "emu/NesApp.hpp"

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <algorithm>

#define WIDTH  (256 * 4)
#define HEIGHT (240 * 2)

NESApp::
NESApp() :
    m_running (true),
    m_display (nullptr),
    m_font    (nullptr),
    m_console_text_dirty (true),
    m_text (nullptr)
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

bool 
NESApp::
onInit()
{

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        return false;
    }

    SDL_EnableUNICODE(SDL_ENABLE);

    const SDL_VideoInfo* video = SDL_GetVideoInfo();
    if (NULL == video) {
        return false;
    }

    if (TTF_Init() != 0) {
     std::cerr << "TTF_OpenFont() Failed: " << TTF_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        exit(1);
    }

    // Load a font
    m_font = TTF_OpenFont("Inconsolata.ttf", 16);

    if (m_font == NULL)
    {
        std::cerr << "TTF_OpenFont() Failed: " << TTF_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        exit(1);
    }

    /* Set the minimum requirements for the OpenGL window */
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

    m_display = SDL_SetVideoMode(WIDTH, HEIGHT, video->vfmt->BitsPerPixel, SDL_SWSURFACE ); 

    /*
    if ((m_display = SDL_SetVideoMode(WIDTH, HEIGHT, video->vfmt->BitsPerPixel, SDL_OPENGL)) == NULL) { return false; }

    glClearColor(0, 0, 0, 0);
    glClearDepth(1.0f);

    glViewport(0, 0, WIDTH, HEIGHT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
 
    glOrtho(0, WIDTH, HEIGHT, 0, 1, -1);
 
    glMatrixMode(GL_MODELVIEW);
 
    glEnable(GL_TEXTURE_2D);
 
    glLoadIdentity();
    */

    return true;
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
NESApp::
renderConsole()
{
    if (m_console.contents().size() == 0 &&
        m_current_input.size() == 0) {
        return;
    }

    std::vector<SDL_Surface*> text_surfaces;
    if (m_console_text_dirty) {
        //FIXME: This is gotta be really slow...
        std::stringstream output;
        output << m_console.contents();
        output << "> " << m_current_input << "\n";

        std::string console_text = output.str();

        std::vector<std::string> lines = split(console_text, '\n');

        SDL_Color text_color = {255, 255, 255};

        // Render each line to a surface.
        std::for_each(lines.begin(), lines.end(), [this, text_color, &text_surfaces](std::string line) {
            if (line.length() == 0) {
                line = std::string(" ");
            }
            // Write text to surface
            SDL_Surface* surface = TTF_RenderText_Blended(m_font,
                        line.c_str(),
                        text_color);
            if (surface == NULL)
            {
                std::cerr << "TTF_RenderText_Solid() Failed: " << TTF_GetError() << std::endl;
                TTF_Quit();
                SDL_Quit();
                exit(1);
            }
            text_surfaces.push_back(surface);
        });


        // Clear the screen
        if (SDL_FillRect(m_display, NULL, SDL_MapRGB( m_display->format, 0,0,0)) != 0)
        {
            std::cerr << "SDL_FillRect() Failed: " << SDL_GetError() << std::endl;
            return;
        }

        SDL_Rect offset {};
        offset.y = HEIGHT - 20;
        int lines_to_render = HEIGHT / 15;
        auto it = text_surfaces.rbegin();
        for (int n = 0; n < lines_to_render; ++n) {
            if (it == text_surfaces.rend()) {
                break;
            }
            // Apply the text to the display
            if (SDL_BlitSurface(*it, NULL, m_display, &offset) != 0)
            {
                std::cerr << "SDL_BlitSurface() Failed: " << SDL_GetError() << std::endl;
                return;
            }
            offset.y -= 15;
            SDL_FreeSurface(*it);
            ++it;
        }

        //Update the display
        SDL_Flip(m_display);
        m_console_text_dirty = false;
    }
}

void 
NESApp::
onEvent(SDL_Event* Event)
{
    if (Event->type == SDL_QUIT) {
        m_running = false;
    }

    if (Event->type == SDL_KEYDOWN) {
        SDLKey keyPressed = Event->key.keysym.sym;
        // Handle general characters.
        if (keyPressed >= SDLK_SPACE && keyPressed <= SDLK_z) {
            Uint16 unicode_char = Event->key.keysym.unicode;
            m_current_input.push_back((char)unicode_char);
            // FIXME: Race condition?
            m_console_text_dirty = true;
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
NESApp::
onLoop()
{
}

void 
NESApp::
onRender()
{
/*
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glBegin(GL_QUADS);
        glColor3f(1, 0, 0); glVertex3f(0, 0, 0);
        glColor3f(1, 1, 0); glVertex3f(WIDTH, 0, 0);
        glColor3f(1, 0, 1); glVertex3f(WIDTH, HEIGHT, 0);
        glColor3f(1, 1, 1); glVertex3f(0, HEIGHT, 0);
    glEnd();
*/
    //SDL_GL_SwapBuffers();

    renderConsole();
}

void 
NESApp::
onCleanup()
{
}
