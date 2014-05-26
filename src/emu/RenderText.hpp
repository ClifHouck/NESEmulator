#ifndef RENDER_TEXT_H
#define RENDER_TEXT_H

#include "SDL2/SDL_ttf.h"

#include <string>

namespace RenderText
{

// Draw text block at specified coordinates on the specified renderer.
void render_text(SDL_Renderer* renderer,
                 unsigned int x, unsigned int y, 
                 SDL_Color* text_color, SDL_Color* background_color,
                 const std::string& text,
                 bool reverse = false);

}

#endif  // RENDER_TEXT_H

