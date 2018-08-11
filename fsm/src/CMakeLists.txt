file(GLOB_RECURSE FSM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE FSM_HEADERS ${PROJECT_SOURCE_DIR}/include/*.hpp)

add_library(${FSM}
    ${FSM_SOURCES}
    ${FSM_HEADERS}
    )

target_include_directories(${FSM}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    )
