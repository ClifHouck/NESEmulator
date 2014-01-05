include(FindSDL)

find_package(SDL2       REQUIRED)
find_package(SDL2TTF    REQUIRED)
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
        ${SDL2_LIBRARY} 
        ${SDL2TTF_LIBRARY}
        ${GLUT_LIBRARY} 
        ${OPENGL_LIBRARIES}
)
