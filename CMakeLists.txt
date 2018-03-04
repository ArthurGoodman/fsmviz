cmake_minimum_required(VERSION 3.0.0)
project(fsmviz)

################################################################################
# Cmake Options
################################################################################

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

################################################################################
# Compiler settings
################################################################################

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_EXTENSIONS off)

set(FLAGS "-Wall -Wextra -Wunused-variable -Wno-comment")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS}")

################################################################################
# Libraries
################################################################################

set(FSM_DIR ${PROJECT_SOURCE_DIR}/../fsm)
add_library(fsm STATIC IMPORTED)
set_target_properties(fsm PROPERTIES
    IMPORTED_LOCATION ${FSM_DIR}/lib/libfsm.a
    INTERFACE_INCLUDE_DIRECTORIES ${FSM_DIR}/include
    )

# Qt5
set(CMAKE_AUTOMOC on)
find_package(Qt5
    COMPONENTS
        Core
        Gui
        Widgets
    REQUIRED
    )

################################################################################
# Targets
################################################################################

set(FSMVIZ ${PROJECT_NAME})

add_subdirectory(src)
