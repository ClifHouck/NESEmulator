include(FindSDL)

find_package(SDL        REQUIRED)
find_package(SDL_ttf    REQUIRED)
find_package(OpenGL     REQUIRED)
find_package(GLUT       REQUIRED)

add_executable(nesemu 
        main.cpp 
        NesApp.cpp
        NES.cpp
)

target_link_libraries(nesemu 
        Utility
        Cpu65XX 
        PPU 
        NESIO
        iNESFile
        Mapper
        ${SDL_LIBRARY} 
        ${SDL_TTF_LIBRARY}
        ${GLUT_LIBRARY} 
        ${OPENGL_LIBRARIES}
)
