include(FindSDL)

find_package(SDL REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(nesemu 
        main.cpp 
        NesApp.cpp
        NES.cpp
)

target_link_libraries(nesemu 
        Cpu65XX 
        PPU 
        Utility
        ${SDL_LIBRARY} 
        ${OPENGL_LIBRARIES}
)
