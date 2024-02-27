add_executable(cpp_playground
    main.cpp
)

target_include_directories(cpp_playground
    PRIVATE ${SYSTEMC_INCLUDE}
)

target_link_libraries(cpp_playground
    PRIVATE ${SYSTEMC_LIBRARY}
)