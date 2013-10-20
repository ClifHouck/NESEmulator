include(FindSDL)

find_package(SDL REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(PPUTestApp
    PPUTest.cpp
)

target_link_libraries(PPUTestApp
        iNESFile
        PPU
        ${SDL_LIBRARY}
        ${OPENGL_LIBRARIES}
)

add_test(PPUTestApp 
    ${CMAKE_CURRENT_BINARY_DIR}/PPUTestApp
)
