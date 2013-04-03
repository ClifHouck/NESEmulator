add_executable(Cpu65XXTest 
        main.cpp
)

add_custom_target(nestestrom 
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/nestest.nes ${CMAKE_CURRENT_BINARY_DIR}/
    COMMENT "Copying nestest.nes ROM to test directory..."
    DEPENDS ${CMAKE_CURRENT_LIST_DIR}/nestest.nes
)

add_dependencies(Cpu65XXTest nestestrom)

target_link_libraries(Cpu65XXTest
        iNESFile
        Cpu65XX
)

add_test(Cpu65XXTest ${CMAKE_CURRENT_BINARY_DIR}/Cpu65XXTest)
