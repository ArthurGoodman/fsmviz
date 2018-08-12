cmake_minimum_required(VERSION 3.0.0)
project(fsm)

################################################################################
# CMake options
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
# Options
################################################################################

option(BUILD_FSM_TESTS "Build FSM tests" off)

################################################################################
# Targets
################################################################################

set(FSM ${PROJECT_NAME})

add_subdirectory(src)

if(BUILD_FSM_TESTS)
    set(FSM_TEST ${PROJECT_NAME}_test)
    add_subdirectory(test)
endif()
