cmake_minimum_required(VERSION 3.12)

project(KdLib VERSION 0.1.0 LANGUAGES CXX)

set(KD_LIB_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(KD_LIB_SOURCE
  "${KD_LIB_SOURCE_DIR}/kdl/cmd_utils.cpp"
)

set(KD_LIB_HEADER
  "${KD_LIB_SOURCE_DIR}/kdl/binary_relation.h"
  "${KD_LIB_SOURCE_DIR}/kdl/cmd_utils.h"
  "${KD_LIB_SOURCE_DIR}/kdl/collection_utils.h"
  "${KD_LIB_SOURCE_DIR}/kdl/compact_trie_forward.h"
  "${KD_LIB_SOURCE_DIR}/kdl/compact_trie.h"
  "${KD_LIB_SOURCE_DIR}/kdl/dynamic_bitset.cpp"
  "${KD_LIB_SOURCE_DIR}/kdl/dynamic_bitset.h"
  "${KD_LIB_SOURCE_DIR}/kdl/enum_array.h"
  "${KD_LIB_SOURCE_DIR}/kdl/filesystem_utils.cpp"
  "${KD_LIB_SOURCE_DIR}/kdl/filesystem_utils.h"
  "${KD_LIB_SOURCE_DIR}/kdl/functional.h"
  "${KD_LIB_SOURCE_DIR}/kdl/hash_utils.h"
  "${KD_LIB_SOURCE_DIR}/kdl/intrusive_circular_list_forward.h"
  "${KD_LIB_SOURCE_DIR}/kdl/intrusive_circular_list.h"
  "${KD_LIB_SOURCE_DIR}/kdl/invoke.h"
  "${KD_LIB_SOURCE_DIR}/kdl/k.h"
  "${KD_LIB_SOURCE_DIR}/kdl/map_utils.h"
  "${KD_LIB_SOURCE_DIR}/kdl/memory_utils.h"
  "${KD_LIB_SOURCE_DIR}/kdl/meta_utils.h"
  "${KD_LIB_SOURCE_DIR}/kdl/optional_utils.h"
  "${KD_LIB_SOURCE_DIR}/kdl/overload.h"
  "${KD_LIB_SOURCE_DIR}/kdl/path_hash.cpp"
  "${KD_LIB_SOURCE_DIR}/kdl/path_hash.h"
  "${KD_LIB_SOURCE_DIR}/kdl/path_utils.cpp"
  "${KD_LIB_SOURCE_DIR}/kdl/path_utils.h"
  "${KD_LIB_SOURCE_DIR}/kdl/range_utils.h"
  "${KD_LIB_SOURCE_DIR}/kdl/ranges/adjacent_transform_view.h"
  "${KD_LIB_SOURCE_DIR}/kdl/ranges/adjacent_view.h"
  "${KD_LIB_SOURCE_DIR}/kdl/ranges/as_rvalue_view.h"
  "${KD_LIB_SOURCE_DIR}/kdl/ranges/cartesian_product_view.h"
  "${KD_LIB_SOURCE_DIR}/kdl/ranges/chunk_by_view.h"
  "${KD_LIB_SOURCE_DIR}/kdl/ranges/chunk_view.h"
  "${KD_LIB_SOURCE_DIR}/kdl/ranges/detail/integer_like.h"
  "${KD_LIB_SOURCE_DIR}/kdl/ranges/detail/movable_box.h"
  "${KD_LIB_SOURCE_DIR}/kdl/ranges/detail/non_propagating_cache.h"
  "${KD_LIB_SOURCE_DIR}/kdl/ranges/detail/range_utils.h"
  "${KD_LIB_SOURCE_DIR}/kdl/ranges/enumerate_view.h"
  "${KD_LIB_SOURCE_DIR}/kdl/ranges/repeat_view.h"
  "${KD_LIB_SOURCE_DIR}/kdl/ranges/slide_view.h"
  "${KD_LIB_SOURCE_DIR}/kdl/ranges/stride_view.h"
  "${KD_LIB_SOURCE_DIR}/kdl/ranges/to.h"
  "${KD_LIB_SOURCE_DIR}/kdl/ranges/zip_transform_view.h"
  "${KD_LIB_SOURCE_DIR}/kdl/ranges/zip_view.h"
  "${KD_LIB_SOURCE_DIR}/kdl/reflection_decl.h"
  "${KD_LIB_SOURCE_DIR}/kdl/reflection_impl.h"
  "${KD_LIB_SOURCE_DIR}/kdl/regex_utils.cpp"
  "${KD_LIB_SOURCE_DIR}/kdl/regex_utils.h"
  "${KD_LIB_SOURCE_DIR}/kdl/resource.h"
  "${KD_LIB_SOURCE_DIR}/kdl/result_error.cpp"
  "${KD_LIB_SOURCE_DIR}/kdl/result_error.h"
  "${KD_LIB_SOURCE_DIR}/kdl/result_fold.h"
  "${KD_LIB_SOURCE_DIR}/kdl/result_forward.h"
  "${KD_LIB_SOURCE_DIR}/kdl/result_io.h"
  "${KD_LIB_SOURCE_DIR}/kdl/result.h"
  "${KD_LIB_SOURCE_DIR}/kdl/set_adapter.h"
  "${KD_LIB_SOURCE_DIR}/kdl/set_temp.h"
  "${KD_LIB_SOURCE_DIR}/kdl/stable_remove_duplicates.h"
  "${KD_LIB_SOURCE_DIR}/kdl/std_io.h"
  "${KD_LIB_SOURCE_DIR}/kdl/string_compare_detail.h"
  "${KD_LIB_SOURCE_DIR}/kdl/string_compare.cpp"
  "${KD_LIB_SOURCE_DIR}/kdl/string_compare.h"
  "${KD_LIB_SOURCE_DIR}/kdl/string_format.cpp"
  "${KD_LIB_SOURCE_DIR}/kdl/string_format.h"
  "${KD_LIB_SOURCE_DIR}/kdl/string_utils.cpp"
  "${KD_LIB_SOURCE_DIR}/kdl/string_utils.h"
  "${KD_LIB_SOURCE_DIR}/kdl/struct_io.h"
  "${KD_LIB_SOURCE_DIR}/kdl/task_manager.cpp"
  "${KD_LIB_SOURCE_DIR}/kdl/task_manager.h"
  "${KD_LIB_SOURCE_DIR}/kdl/traits.h"
  "${KD_LIB_SOURCE_DIR}/kdl/tuple_utils.h"
  "${KD_LIB_SOURCE_DIR}/kdl/vector_set_forward.h"
  "${KD_LIB_SOURCE_DIR}/kdl/vector_set.h"
  "${KD_LIB_SOURCE_DIR}/kdl/vector_utils.h"
)

add_library(KdLib STATIC ${KD_LIB_SOURCE} ${KD_LIB_HEADER})
target_include_directories(KdLib PUBLIC ${KD_LIB_SOURCE_DIR})

target_link_libraries(KdLib PRIVATE CompilerConfig PrecompileStdHeaders)

# task_manager.h uses <thread>, etc., which requires this on Linux
find_package(Threads REQUIRED)
target_link_libraries(KdLib PUBLIC Threads::Threads)

# Use the fast-float library for from_chars on AppleClang, where std::from_chars is not
# available for floating point types.
if (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
target_link_libraries(KdLib PRIVATE FastFloat::fast_float)
endif()

add_subdirectory(test)
