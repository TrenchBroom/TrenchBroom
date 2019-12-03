add_executable(kdl-test)
target_sources(kdl-test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/collection_utils_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/map_utils_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/run_all.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/vector_set_test.cpp"
        )

target_link_libraries(kdl-test gtest kdl)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    target_compile_options(kdl-test PRIVATE -Wall -Wextra -Weverything -pedantic -Wno-c++98-compat -Wno-global-constructors -Wno-zero-as-null-pointer-constant)
    #  -Wno-format -Wno-variadic-macros -Wno-c99-extensions -Wno-padded -Wno-unused-parameter -Wno-global-constructors -Wno-exit-time-destructors -Wno-weak-vtables -Wno-weak-template-vtables -Wno-float-equal -Wno-used-but-marked-unused -Wno-format-nonliteral -Wno-missing-noreturn -Wno-unused-local-typedef -Wno-double-promotion -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-implicit-fallthrough -Wno-zero-as-null-pointer-constant -Wno-switch-enum
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(kdl-test PRIVATE -Wall -Wextra -pedantic)
    #  -Wno-format -Wno-variadic-macros -Wno-padded -Wno-unused-parameter -Wno-float-equal -Wno-format-nonliteral -Wno-missing-noreturn -Wno-zero-as-null-pointer-constant -Wno-error=maybe-uninitialized
elseif(MSVC EQUAL 1)
    target_compile_options(kdl-test PRIVATE /W3 /EHsc /MP)
else()
    message(FATAL_ERROR "Cannot set compile options")
endif()
