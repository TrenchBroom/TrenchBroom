cmake_minimum_required(VERSION 3.12)

project(KdLib VERSION 0.1.0 LANGUAGES CXX)

set(KD_LIB_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(KD_LIB_SOURCE
  "${KD_LIB_SOURCE_DIR}/kd/cmd_utils.cpp"
)

set(KD_LIB_HEADER
  "${KD_LIB_SOURCE_DIR}/kd/binary_relation.h"
  "${KD_LIB_SOURCE_DIR}/kd/cmd_utils.h"
  "${KD_LIB_SOURCE_DIR}/kd/collection_utils.h"
  "${KD_LIB_SOURCE_DIR}/kd/compact_trie_forward.h"
  "${KD_LIB_SOURCE_DIR}/kd/compact_trie.h"
  "${KD_LIB_SOURCE_DIR}/kd/dynamic_bitset.cpp"
  "${KD_LIB_SOURCE_DIR}/kd/dynamic_bitset.h"
  "${KD_LIB_SOURCE_DIR}/kd/enum_array.h"
  "${KD_LIB_SOURCE_DIR}/kd/filesystem_utils.cpp"
  "${KD_LIB_SOURCE_DIR}/kd/filesystem_utils.h"
  "${KD_LIB_SOURCE_DIR}/kd/functional.h"
  "${KD_LIB_SOURCE_DIR}/kd/hash_utils.h"
  "${KD_LIB_SOURCE_DIR}/kd/intrusive_circular_list_forward.h"
  "${KD_LIB_SOURCE_DIR}/kd/intrusive_circular_list.h"
  "${KD_LIB_SOURCE_DIR}/kd/invoke.h"
  "${KD_LIB_SOURCE_DIR}/kd/k.h"
  "${KD_LIB_SOURCE_DIR}/kd/map_utils.h"
  "${KD_LIB_SOURCE_DIR}/kd/memory_utils.h"
  "${KD_LIB_SOURCE_DIR}/kd/meta_utils.h"
  "${KD_LIB_SOURCE_DIR}/kd/optional_utils.h"
  "${KD_LIB_SOURCE_DIR}/kd/overload.h"
  "${KD_LIB_SOURCE_DIR}/kd/path_hash.cpp"
  "${KD_LIB_SOURCE_DIR}/kd/path_hash.h"
  "${KD_LIB_SOURCE_DIR}/kd/path_utils.cpp"
  "${KD_LIB_SOURCE_DIR}/kd/path_utils.h"
  "${KD_LIB_SOURCE_DIR}/kd/range_utils.h"
  "${KD_LIB_SOURCE_DIR}/kd/ranges/adjacent_transform_view.h"
  "${KD_LIB_SOURCE_DIR}/kd/ranges/adjacent_view.h"
  "${KD_LIB_SOURCE_DIR}/kd/ranges/as_rvalue_view.h"
  "${KD_LIB_SOURCE_DIR}/kd/ranges/cartesian_product_view.h"
  "${KD_LIB_SOURCE_DIR}/kd/ranges/chunk_by_view.h"
  "${KD_LIB_SOURCE_DIR}/kd/ranges/chunk_view.h"
  "${KD_LIB_SOURCE_DIR}/kd/ranges/detail/integer_like.h"
  "${KD_LIB_SOURCE_DIR}/kd/ranges/detail/movable_box.h"
  "${KD_LIB_SOURCE_DIR}/kd/ranges/detail/non_propagating_cache.h"
  "${KD_LIB_SOURCE_DIR}/kd/ranges/detail/range_utils.h"
  "${KD_LIB_SOURCE_DIR}/kd/ranges/enumerate_view.h"
  "${KD_LIB_SOURCE_DIR}/kd/ranges/repeat_view.h"
  "${KD_LIB_SOURCE_DIR}/kd/ranges/slide_view.h"
  "${KD_LIB_SOURCE_DIR}/kd/ranges/stride_view.h"
  "${KD_LIB_SOURCE_DIR}/kd/ranges/to.h"
  "${KD_LIB_SOURCE_DIR}/kd/ranges/zip_transform_view.h"
  "${KD_LIB_SOURCE_DIR}/kd/ranges/zip_view.h"
  "${KD_LIB_SOURCE_DIR}/kd/reflection_decl.h"
  "${KD_LIB_SOURCE_DIR}/kd/reflection_impl.h"
  "${KD_LIB_SOURCE_DIR}/kd/regex_utils.cpp"
  "${KD_LIB_SOURCE_DIR}/kd/regex_utils.h"
  "${KD_LIB_SOURCE_DIR}/kd/resource.h"
  "${KD_LIB_SOURCE_DIR}/kd/result_error.cpp"
  "${KD_LIB_SOURCE_DIR}/kd/result_error.h"
  "${KD_LIB_SOURCE_DIR}/kd/result_fold.h"
  "${KD_LIB_SOURCE_DIR}/kd/result_forward.h"
  "${KD_LIB_SOURCE_DIR}/kd/result_io.h"
  "${KD_LIB_SOURCE_DIR}/kd/result.h"
  "${KD_LIB_SOURCE_DIR}/kd/set_adapter.h"
  "${KD_LIB_SOURCE_DIR}/kd/set_temp.h"
  "${KD_LIB_SOURCE_DIR}/kd/stable_remove_duplicates.h"
  "${KD_LIB_SOURCE_DIR}/kd/std_io.h"
  "${KD_LIB_SOURCE_DIR}/kd/string_compare_detail.h"
  "${KD_LIB_SOURCE_DIR}/kd/string_compare.cpp"
  "${KD_LIB_SOURCE_DIR}/kd/string_compare.h"
  "${KD_LIB_SOURCE_DIR}/kd/string_format.cpp"
  "${KD_LIB_SOURCE_DIR}/kd/string_format.h"
  "${KD_LIB_SOURCE_DIR}/kd/string_utils.cpp"
  "${KD_LIB_SOURCE_DIR}/kd/string_utils.h"
  "${KD_LIB_SOURCE_DIR}/kd/struct_io.h"
  "${KD_LIB_SOURCE_DIR}/kd/task_manager.cpp"
  "${KD_LIB_SOURCE_DIR}/kd/task_manager.h"
  "${KD_LIB_SOURCE_DIR}/kd/traits.h"
  "${KD_LIB_SOURCE_DIR}/kd/tuple_utils.h"
  "${KD_LIB_SOURCE_DIR}/kd/vector_set_forward.h"
  "${KD_LIB_SOURCE_DIR}/kd/vector_set.h"
  "${KD_LIB_SOURCE_DIR}/kd/vector_utils.h"
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
