add_executable(kdl-test)
target_sources(kdl-test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ranges/range_test_utils.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ranges/tst_adjacent_transform_view.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ranges/tst_adjacent_view.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ranges/tst_as_rvalue_view.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ranges/tst_cartesian_product_view.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ranges/tst_chunk_by_view.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ranges/tst_chunk_view.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ranges/tst_enumerate_view.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ranges/tst_movable_box.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ranges/tst_repeat_view.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ranges/tst_slide_view.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ranges/tst_stride_view.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ranges/tst_to.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ranges/tst_zip_transform_view.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ranges/tst_zip_view.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/test_utils.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_binary_relation.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_cmd_utils.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_collection_utils.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_compact_trie.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_filesystem_utils.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_functional.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_grouped_range.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_hash_utils.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_intrusive_circular_list.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_invoke.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_map_utils.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_meta_utils.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_optional_utils.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_pair_iterator.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_path_utils.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_product_iterator.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_range_utils.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_reflection.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_resource.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_result.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_set_adapter.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_set_temp.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_stable_remove_duplicates.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_std_io.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_string_compare.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_string_format.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_string_utils.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_struct_io.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_task_manager.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_tuple_utils.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_vector_set.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_vector_utils.cpp"
)

add_compile_definitions(CATCH_CONFIG_ENABLE_ALL_STRINGMAKERS=1)

target_link_libraries(kdl-test Catch2::Catch2WithMain kdl)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    target_compile_options(kdl-test PRIVATE -Wall -Wextra -Weverything -pedantic -Wno-c++98-compat -Wno-c++20-compat -Wno-global-constructors -Wno-zero-as-null-pointer-constant -Wno-weak-vtables)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(kdl-test PRIVATE -Wall -Wextra -pedantic)
elseif(MSVC EQUAL 1)
    target_compile_options(kdl-test PRIVATE /W4 /EHsc /MP)
else()
    message(FATAL_ERROR "Cannot set compile options")
endif()
