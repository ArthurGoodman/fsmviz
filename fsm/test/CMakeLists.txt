add_executable(${FSM_TEST}
    main.cpp
    )

target_link_libraries(${FSM_TEST}
    PRIVATE ${FSM}
    )
