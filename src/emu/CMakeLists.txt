include(FindSDL)

find_package(SDL REQUIRED)
find_package(OpenGL REQUIRED)

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
        ${SDL_LIBRARY} 
        ${OPENGL_LIBRARIES}
)
