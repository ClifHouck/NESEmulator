#include "emu/NesApp.hpp"

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#define WIDTH  (256 * 2)
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
    m_font = TTF_OpenFont("Inconsolata.ttf", 24);

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

void
NESApp::
renderConsole()
{
    if (m_console_text_dirty) {
        // Write text to surface
        SDL_Color text_color = {255, 255, 255};
        m_text = TTF_RenderText_Solid(m_font,
                    "Testing...",
                    text_color);

        if (m_text == NULL)
        {
            std::cerr << "TTF_RenderText_Solid() Failed: " << TTF_GetError() << std::endl;
            TTF_Quit();
            SDL_Quit();
            exit(1);
        }
        m_console_text_dirty = false;
    }

    // Clear the screen
    if (SDL_FillRect(m_display, NULL, SDL_MapRGB( m_display->format, 0,0,0)) != 0)
    {
       std::cerr << "SDL_FillRect() Failed: " << SDL_GetError() << std::endl;
       return;
    }

    // Apply the text to the display
    if (SDL_BlitSurface(m_text, NULL, m_display, NULL) != 0)
    {
        std::cerr << "SDL_BlitSurface() Failed: " << SDL_GetError() << std::endl;
        return;
    }

    //Update the display
    SDL_Flip(m_display);
}

void 
NESApp::
onEvent(SDL_Event* Event)
{
    if (Event->type == SDL_QUIT) {
        m_running = false;
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
