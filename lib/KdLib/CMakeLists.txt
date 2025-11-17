cmake_minimum_required(VERSION 3.12)

project(kdl VERSION 0.1.0 LANGUAGES CXX)

set(KDL_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(KDL_SOURCE
  "${KDL_SOURCE_DIR}/kdl/cmd_utils.cpp"
)

set(KDL_HEADER
  "${KDL_SOURCE_DIR}/kdl/binary_relation.h"
  "${KDL_SOURCE_DIR}/kdl/cmd_utils.h"
  "${KDL_SOURCE_DIR}/kdl/collection_utils.h"
  "${KDL_SOURCE_DIR}/kdl/compact_trie_forward.h"
  "${KDL_SOURCE_DIR}/kdl/compact_trie.h"
  "${KDL_SOURCE_DIR}/kdl/dynamic_bitset.cpp"
  "${KDL_SOURCE_DIR}/kdl/dynamic_bitset.h"
  "${KDL_SOURCE_DIR}/kdl/enum_array.h"
  "${KDL_SOURCE_DIR}/kdl/filesystem_utils.cpp"
  "${KDL_SOURCE_DIR}/kdl/filesystem_utils.h"
  "${KDL_SOURCE_DIR}/kdl/functional.h"
  "${KDL_SOURCE_DIR}/kdl/hash_utils.h"
  "${KDL_SOURCE_DIR}/kdl/intrusive_circular_list_forward.h"
  "${KDL_SOURCE_DIR}/kdl/intrusive_circular_list.h"
  "${KDL_SOURCE_DIR}/kdl/invoke.h"
  "${KDL_SOURCE_DIR}/kdl/k.h"
  "${KDL_SOURCE_DIR}/kdl/map_utils.h"
  "${KDL_SOURCE_DIR}/kdl/memory_utils.h"
  "${KDL_SOURCE_DIR}/kdl/meta_utils.h"
  "${KDL_SOURCE_DIR}/kdl/optional_utils.h"
  "${KDL_SOURCE_DIR}/kdl/overload.h"
  "${KDL_SOURCE_DIR}/kdl/path_hash.cpp"
  "${KDL_SOURCE_DIR}/kdl/path_hash.h"
  "${KDL_SOURCE_DIR}/kdl/path_utils.cpp"
  "${KDL_SOURCE_DIR}/kdl/path_utils.h"
  "${KDL_SOURCE_DIR}/kdl/range_utils.h"
  "${KDL_SOURCE_DIR}/kdl/ranges/adjacent_transform_view.h"
  "${KDL_SOURCE_DIR}/kdl/ranges/adjacent_view.h"
  "${KDL_SOURCE_DIR}/kdl/ranges/as_rvalue_view.h"
  "${KDL_SOURCE_DIR}/kdl/ranges/cartesian_product_view.h"
  "${KDL_SOURCE_DIR}/kdl/ranges/chunk_by_view.h"
  "${KDL_SOURCE_DIR}/kdl/ranges/chunk_view.h"
  "${KDL_SOURCE_DIR}/kdl/ranges/detail/integer_like.h"
  "${KDL_SOURCE_DIR}/kdl/ranges/detail/movable_box.h"
  "${KDL_SOURCE_DIR}/kdl/ranges/detail/non_propagating_cache.h"
  "${KDL_SOURCE_DIR}/kdl/ranges/detail/range_utils.h"
  "${KDL_SOURCE_DIR}/kdl/ranges/enumerate_view.h"
  "${KDL_SOURCE_DIR}/kdl/ranges/repeat_view.h"
  "${KDL_SOURCE_DIR}/kdl/ranges/slide_view.h"
  "${KDL_SOURCE_DIR}/kdl/ranges/stride_view.h"
  "${KDL_SOURCE_DIR}/kdl/ranges/to.h"
  "${KDL_SOURCE_DIR}/kdl/ranges/zip_transform_view.h"
  "${KDL_SOURCE_DIR}/kdl/ranges/zip_view.h"
  "${KDL_SOURCE_DIR}/kdl/reflection_decl.h"
  "${KDL_SOURCE_DIR}/kdl/reflection_impl.h"
  "${KDL_SOURCE_DIR}/kdl/regex_utils.cpp"
  "${KDL_SOURCE_DIR}/kdl/regex_utils.h"
  "${KDL_SOURCE_DIR}/kdl/resource.h"
  "${KDL_SOURCE_DIR}/kdl/result_error.cpp"
  "${KDL_SOURCE_DIR}/kdl/result_error.h"
  "${KDL_SOURCE_DIR}/kdl/result_fold.h"
  "${KDL_SOURCE_DIR}/kdl/result_forward.h"
  "${KDL_SOURCE_DIR}/kdl/result_io.h"
  "${KDL_SOURCE_DIR}/kdl/result.h"
  "${KDL_SOURCE_DIR}/kdl/set_adapter.h"
  "${KDL_SOURCE_DIR}/kdl/set_temp.h"
  "${KDL_SOURCE_DIR}/kdl/stable_remove_duplicates.h"
  "${KDL_SOURCE_DIR}/kdl/std_io.h"
  "${KDL_SOURCE_DIR}/kdl/string_compare_detail.h"
  "${KDL_SOURCE_DIR}/kdl/string_compare.cpp"
  "${KDL_SOURCE_DIR}/kdl/string_compare.h"
  "${KDL_SOURCE_DIR}/kdl/string_format.cpp"
  "${KDL_SOURCE_DIR}/kdl/string_format.h"
  "${KDL_SOURCE_DIR}/kdl/string_utils.cpp"
  "${KDL_SOURCE_DIR}/kdl/string_utils.h"
  "${KDL_SOURCE_DIR}/kdl/struct_io.h"
  "${KDL_SOURCE_DIR}/kdl/task_manager.cpp"
  "${KDL_SOURCE_DIR}/kdl/task_manager.h"
  "${KDL_SOURCE_DIR}/kdl/traits.h"
  "${KDL_SOURCE_DIR}/kdl/tuple_utils.h"
  "${KDL_SOURCE_DIR}/kdl/vector_set_forward.h"
  "${KDL_SOURCE_DIR}/kdl/vector_set.h"
  "${KDL_SOURCE_DIR}/kdl/vector_utils.h"
)

add_library(kdl STATIC ${KDL_SOURCE} ${KDL_HEADER})
target_include_directories(kdl PUBLIC ${KDL_SOURCE_DIR})

target_link_libraries(kdl PRIVATE CompilerConfig PrecompileStdHeaders)

# task_manager.h uses <thread>, etc., which requires this on Linux
find_package(Threads REQUIRED)
target_link_libraries(kdl PUBLIC Threads::Threads)

# Use the fast-float library for from_chars on AppleClang, where std::from_chars is not
# available for floating point types.
if (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
target_link_libraries(kdl PRIVATE FastFloat::fast_float)
endif()

add_subdirectory(test)
