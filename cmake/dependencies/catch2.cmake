# Catch2 forces default symbol visibility and warns when a hidden visibility
# preset is set globally (as this project does). Set it to default around Catch2
# to avoid the warning; the resulting visibility is identical.
set(_tb_saved_cxx_visibility ${CMAKE_CXX_VISIBILITY_PRESET})
set(CMAKE_CXX_VISIBILITY_PRESET default)
CPMAddPackage("gh:catchorg/Catch2#v3.10.0")
set(CMAKE_CXX_VISIBILITY_PRESET ${_tb_saved_cxx_visibility})

suppress_dependency_warnings(Catch2)
suppress_dependency_warnings(Catch2WithMain)
apply_sanitizer_options(Catch2)
apply_sanitizer_options(Catch2WithMain)

# Make Catch2's CMake helpers (catch_discover_tests) available via include(Catch).
list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
