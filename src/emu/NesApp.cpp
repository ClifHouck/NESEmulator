#include "emu/NesApp.hpp"
#include "CPU/Cpu65XX.hpp"
#include "PPU/PPU.hpp"

#include <GL/gl.h>
#include <GL/glu.h>

#define WIDTH  256
#define HEIGHT 240

NESApp::
NESApp() :
    m_running (true),
    m_display ()
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

    /* Set the minimum requirements for the OpenGL window */
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

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

    return true;
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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glBegin(GL_QUADS);
        glColor3f(1, 0, 0); glVertex3f(0, 0, 0);
        glColor3f(1, 1, 0); glVertex3f(WIDTH, 0, 0);
        glColor3f(1, 0, 1); glVertex3f(WIDTH, HEIGHT, 0);
        glColor3f(1, 1, 1); glVertex3f(0, HEIGHT, 0);
    glEnd();

    SDL_GL_SwapBuffers();
}

void 
NESApp::
onCleanup()
{
}
