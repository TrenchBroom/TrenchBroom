add_executable(kdl-test)
target_sources(kdl-test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/GTestCompat.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/binary_relation_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/collection_utils_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/compact_trie_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/invoke_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/intrusive_circular_list_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/map_utils_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/meta_utils_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/result_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/run_all.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/set_adapter_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/skip_iterator_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/string_compare_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/string_format_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/string_utils_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/set_temp_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/test_utils.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/transform_range_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/vector_set_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/vector_utils_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/zip_iterator_test.cpp"
        )

set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/run_all.cpp" PROPERTY SKIP_UNITY_BUILD_INCLUSION ON)

target_link_libraries(kdl-test Catch2::Catch2 kdl)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    target_compile_options(kdl-test PRIVATE -Wall -Wextra -Weverything -pedantic -Wno-c++98-compat -Wno-global-constructors -Wno-zero-as-null-pointer-constant -Wno-weak-vtables)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(kdl-test PRIVATE -Wall -Wextra -pedantic)
elseif(MSVC EQUAL 1)
    target_compile_options(kdl-test PRIVATE /W3 /EHsc /MP)
else()
    message(FATAL_ERROR "Cannot set compile options")
endif()
