#include "RenderText.hpp"

#include "SDL2/SDL_ttf.h"

#include <algorithm>
#include <sstream>
#include <vector>

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
RenderText::
render_text(SDL_Renderer* renderer,
            unsigned int x, unsigned int y, 
            SDL_Color* text_color, SDL_Color* background_color,
            const std::string& text,
            bool reverse)
{
    // Load a font
    static TTF_Font* font = nullptr;
    if (font == nullptr) {
        font = TTF_OpenFont("Inconsolata.ttf", 20);
        //TODO: Raise exception if loading font fails!
    }

    std::vector<std::string> lines = split(text, '\n');

    std::vector<SDL_Surface*> text_surfaces;

    // Render each line to a surface.
    std::for_each(lines.begin(), lines.end(), [background_color, text_color, &text_surfaces](std::string line) {
        // RenderText doesn't like empty strings.
        if (line.length() == 0) {
            line = std::string("   ");
        }

        // Write text to surface
        SDL_Surface* surface = TTF_RenderText_Shaded(font, line.c_str(), *text_color, *background_color);
        //checkTTFError(NULL == surface, "TTF_RenderText_Solid() Failed: "); 
        text_surfaces.push_back(surface);
    });

    SDL_Rect dest_rect {};
    dest_rect.x = x;
    dest_rect.y = y;
    std::vector<SDL_Texture*> textures;

    int step = 20;
    if (reverse) { 
        step = -20; 
        std::reverse(text_surfaces.begin(), text_surfaces.end());
    }

    std::for_each(text_surfaces.begin(), text_surfaces.end(), [&](SDL_Surface* surface){
        SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, surface);
        //checkSDLError(NULL == text_texture, "SDL_CreateTextureFromSurface Failed: ");
        textures.push_back(text_texture);

        // Apply the text to the display
        dest_rect.w = surface->w;
        dest_rect.h = surface->h;
        SDL_Rect src_rect = { 0, 0, surface->w, surface->h };
        int return_code = SDL_RenderCopy(renderer, text_texture, &src_rect, &dest_rect);
        //checkSDLError(0 != return_code, "SDL_RenderCopy Failed: ");

        dest_rect.y += step;
    });

    // Clean up surfaces and textures.
    std::for_each(text_surfaces.begin(), text_surfaces.end(), [](SDL_Surface * surface) {
        SDL_FreeSurface(surface);
    });
    std::for_each(textures.begin(), textures.end(), [](SDL_Texture * texture) {
        SDL_DestroyTexture(texture);
    });
}

