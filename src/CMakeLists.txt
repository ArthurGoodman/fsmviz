file(GLOB_RECURSE FSMVIZ_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE FSMVIZ_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)

set(CMAKE_AUTOMOC on)
set(CMAKE_INCLUDE_CURRENT_DIR on)

qt5_wrap_cpp(${FSMVIZ_SOURCES})

add_executable(${FSMVIZ}
    ${FSMVIZ_SOURCES}
    ${FSMVIZ_HEADERS}
    )

target_link_libraries(${FSMVIZ}
    fsm
    qconsole
    Qt5::Widgets
    )

target_include_directories(${FSMVIZ}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    )
