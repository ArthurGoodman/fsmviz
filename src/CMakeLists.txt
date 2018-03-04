file(GLOB_RECURSE SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
    )

add_executable(${FSMVIZ} ${SOURCES})

qt5_use_modules(${FSMVIZ}
    Core
    Gui
    Widgets
    )

target_link_libraries(${FSMVIZ}
    fsm
    )

target_include_directories(${FSMVIZ}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    )
