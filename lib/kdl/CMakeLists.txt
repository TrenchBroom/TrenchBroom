cmake_minimum_required(VERSION 3.12)

project(kdl VERSION 0.1.0 LANGUAGES CXX)

# instruct cmake not to set default warning levels for MSVC projects (cmake 3.15 or higher)
if (POLICY CMP0092)
    cmake_policy(SET CMP0092 NEW)
endif()

set(KDL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(kdl INTERFACE)
target_include_directories(kdl INTERFACE
        $<BUILD_INTERFACE:${KDL_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:kdl/include/kdl>)

# task_manager.h uses <thread>, etc., which requires this on Linux
find_package(Threads REQUIRED)
target_link_libraries(kdl INTERFACE Threads::Threads)

target_sources(kdl INTERFACE
    "${KDL_INCLUDE_DIR}/kdl/binary_relation.h"
    "${KDL_INCLUDE_DIR}/kdl/bitset.h"
    "${KDL_INCLUDE_DIR}/kdl/collection_utils.h"
    "${KDL_INCLUDE_DIR}/kdl/compact_trie_forward.h"
    "${KDL_INCLUDE_DIR}/kdl/compact_trie.h"
    "${KDL_INCLUDE_DIR}/kdl/enum_array.h"
    "${KDL_INCLUDE_DIR}/kdl/functional.h"
    "${KDL_INCLUDE_DIR}/kdl/grouped_range.h"
    "${KDL_INCLUDE_DIR}/kdl/hash_utils.h"
    "${KDL_INCLUDE_DIR}/kdl/intrusive_circular_list_forward.h"
    "${KDL_INCLUDE_DIR}/kdl/intrusive_circular_list.h"
    "${KDL_INCLUDE_DIR}/kdl/invoke.h"
    "${KDL_INCLUDE_DIR}/kdl/k.h"
    "${KDL_INCLUDE_DIR}/kdl/map_utils.h"
    "${KDL_INCLUDE_DIR}/kdl/memory_utils.h"
    "${KDL_INCLUDE_DIR}/kdl/meta_utils.h"
    "${KDL_INCLUDE_DIR}/kdl/overload.h"
    "${KDL_INCLUDE_DIR}/kdl/optional_utils.h"
    "${KDL_INCLUDE_DIR}/kdl/pair_iterator.h"
    "${KDL_INCLUDE_DIR}/kdl/path_hash.h"
    "${KDL_INCLUDE_DIR}/kdl/path_utils.h"
    "${KDL_INCLUDE_DIR}/kdl/product_iterator.h"
    "${KDL_INCLUDE_DIR}/kdl/range_io.h"
    "${KDL_INCLUDE_DIR}/kdl/range_to.h"
    "${KDL_INCLUDE_DIR}/kdl/range_to_vector.h"
    "${KDL_INCLUDE_DIR}/kdl/range_utils.h"
    "${KDL_INCLUDE_DIR}/kdl/range.h"
    "${KDL_INCLUDE_DIR}/kdl/reflection_decl.h"
    "${KDL_INCLUDE_DIR}/kdl/reflection_impl.h"
    "${KDL_INCLUDE_DIR}/kdl/regex_utils.h"
    "${KDL_INCLUDE_DIR}/kdl/resource.h"
    "${KDL_INCLUDE_DIR}/kdl/result_fold.h"
    "${KDL_INCLUDE_DIR}/kdl/result_forward.h"
    "${KDL_INCLUDE_DIR}/kdl/result_io.h"
    "${KDL_INCLUDE_DIR}/kdl/result.h"
    "${KDL_INCLUDE_DIR}/kdl/set_adapter.h"
    "${KDL_INCLUDE_DIR}/kdl/set_temp.h"
    "${KDL_INCLUDE_DIR}/kdl/skip_iterator.h"
    "${KDL_INCLUDE_DIR}/kdl/stable_remove_duplicates.h"
    "${KDL_INCLUDE_DIR}/kdl/std_io.h"
    "${KDL_INCLUDE_DIR}/kdl/string_compare_detail.h"
    "${KDL_INCLUDE_DIR}/kdl/string_compare.h"
    "${KDL_INCLUDE_DIR}/kdl/string_format.h"
    "${KDL_INCLUDE_DIR}/kdl/string_utils.h"
    "${KDL_INCLUDE_DIR}/kdl/struct_io.h"
    "${KDL_INCLUDE_DIR}/kdl/task_manager.h"
    "${KDL_INCLUDE_DIR}/kdl/traits.h"
    "${KDL_INCLUDE_DIR}/kdl/tuple_utils.h"
    "${KDL_INCLUDE_DIR}/kdl/vector_set_forward.h"
    "${KDL_INCLUDE_DIR}/kdl/vector_set.h"
    "${KDL_INCLUDE_DIR}/kdl/vector_utils.h"
    "${KDL_INCLUDE_DIR}/kdl/zip_iterator.h"
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    target_compile_options(kdl INTERFACE -Wall -Wextra -Wconversion -Wshadow-all -pedantic -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded -Wno-exit-time-destructors)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(kdl INTERFACE -Wall -Wextra -Wconversion -Wshadow=local -pedantic)

    # gcc <= 7 warns about unused structured bindings, see https://github.com/TrenchBroom/TrenchBroom/issues/3751
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8)
        target_compile_options(kdl INTERFACE -Wno-unused-variable)
    endif()
elseif(MSVC EQUAL 1)
    target_compile_options(kdl INTERFACE /W4 /EHsc /MP)

    # signed/unsigned mismatch: https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4365
    target_compile_options(kdl INTERFACE /w44365)
else()
    message(FATAL_ERROR "Cannot set compile options for target")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# add_subdirectory(lib)
add_subdirectory(test)
