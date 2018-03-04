file(GLOB_RECURSE FSMVIZ_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    )

file(GLOB_RECURSE FSMVIZ_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
    )

set(CMAKE_AUTOMOC true)
set(CMAKE_INCLUDE_CURRENT_DIR true)

qt5_wrap_cpp(${FSMVIZ_HEADERS})

add_executable(${FSMVIZ}
    ${FSMVIZ_SOURCES}
    ${FSMVIZ_HEADERS}
    )

target_link_libraries(${FSMVIZ}
    fsm
    Qt5::Widgets
    )

target_include_directories(${FSMVIZ}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    )
