cmake_minimum_required(VERSION 3.12)

project(kdl VERSION 0.1.0 LANGUAGES CXX)

set(LIB_KDL_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(LIB_KDL_SOURCE
  "${LIB_KDL_SOURCE_DIR}/kdl/cmd_utils.cpp"
)

set(LIB_KDL_HEADER
  "${LIB_KDL_SOURCE_DIR}/kdl/binary_relation.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/cmd_utils.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/collection_utils.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/compact_trie_forward.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/compact_trie.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/dynamic_bitset.cpp"
  "${LIB_KDL_SOURCE_DIR}/kdl/dynamic_bitset.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/enum_array.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/filesystem_utils.cpp"
  "${LIB_KDL_SOURCE_DIR}/kdl/filesystem_utils.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/functional.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/hash_utils.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/intrusive_circular_list_forward.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/intrusive_circular_list.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/invoke.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/k.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/map_utils.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/memory_utils.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/meta_utils.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/optional_utils.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/overload.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/path_hash.cpp"
  "${LIB_KDL_SOURCE_DIR}/kdl/path_hash.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/path_utils.cpp"
  "${LIB_KDL_SOURCE_DIR}/kdl/path_utils.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/range_utils.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/ranges/adjacent_transform_view.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/ranges/adjacent_view.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/ranges/as_rvalue_view.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/ranges/cartesian_product_view.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/ranges/chunk_by_view.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/ranges/chunk_view.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/ranges/detail/integer_like.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/ranges/detail/movable_box.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/ranges/detail/non_propagating_cache.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/ranges/detail/range_utils.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/ranges/enumerate_view.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/ranges/repeat_view.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/ranges/slide_view.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/ranges/stride_view.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/ranges/to.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/ranges/zip_transform_view.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/ranges/zip_view.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/reflection_decl.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/reflection_impl.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/regex_utils.cpp"
  "${LIB_KDL_SOURCE_DIR}/kdl/regex_utils.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/resource.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/result_error.cpp"
  "${LIB_KDL_SOURCE_DIR}/kdl/result_error.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/result_fold.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/result_forward.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/result_io.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/result.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/set_adapter.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/set_temp.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/stable_remove_duplicates.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/std_io.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/string_compare_detail.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/string_compare.cpp"
  "${LIB_KDL_SOURCE_DIR}/kdl/string_compare.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/string_format.cpp"
  "${LIB_KDL_SOURCE_DIR}/kdl/string_format.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/string_utils.cpp"
  "${LIB_KDL_SOURCE_DIR}/kdl/string_utils.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/struct_io.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/task_manager.cpp"
  "${LIB_KDL_SOURCE_DIR}/kdl/task_manager.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/traits.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/tuple_utils.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/vector_set_forward.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/vector_set.h"
  "${LIB_KDL_SOURCE_DIR}/kdl/vector_utils.h"
)

add_library(lib-kdl STATIC ${LIB_KDL_SOURCE} ${LIB_KDL_HEADER})
target_include_directories(lib-kdl PUBLIC ${LIB_KDL_SOURCE_DIR})

# task_manager.h uses <thread>, etc., which requires this on Linux
find_package(Threads REQUIRED)
target_link_libraries(lib-kdl PUBLIC Threads::Threads)

# Use the fast-float library for from_chars on AppleClang, where std::from_chars is not
# available for floating point types.
if (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
target_link_libraries(lib-kdl PRIVATE FastFloat::fast_float)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    target_compile_options(lib-kdl INTERFACE -Wall -Wextra -Wconversion -Wshadow-all -pedantic -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-c++20-compat -Wno-padded -Wno-exit-time-destructors -Wno-poison-system-directories)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(lib-kdl INTERFACE -Wall -Wextra -Wconversion -Wshadow=local -pedantic)

    # gcc <= 7 warns about unused structured bindings, see https://github.com/TrenchBroom/TrenchBroom/issues/3751
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8)
        target_compile_options(lib-kdl INTERFACE -Wno-unused-variable)
    endif()
elseif(MSVC EQUAL 1)
    target_compile_options(lib-kdl INTERFACE /W4 /EHsc /MP)

    # signed/unsigned mismatch: https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4365
    target_compile_options(lib-kdl INTERFACE /w44365)
else()
    message(FATAL_ERROR "Cannot set compile options for target")
endif()

add_subdirectory(test)
