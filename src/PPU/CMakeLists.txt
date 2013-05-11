add_library(PPU 
    PPU.cpp
    Palette.cpp
)

target_link_libraries(PPU
    Utility
)
