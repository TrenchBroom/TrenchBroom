message(STATUS "Fetching Catch2...")
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2
  GIT_TAG        5af20cabb990f36f1671a6a95aef0e6a4cb2573d # v3.10.0
  SYSTEM
)

set(CATCH_INSTALL_DOCS OFF)
set(CATCH_INSTALL_EXTRAS OFF)
# Catch2 forces default symbol visibility and warns when a hidden visibility
# preset is set globally (as this project does). Set it to default around Catch2
# to avoid the warning; the resulting visibility is identical.
set(_tb_saved_cxx_visibility ${CMAKE_CXX_VISIBILITY_PRESET})
set(CMAKE_CXX_VISIBILITY_PRESET default)
FetchContent_MakeAvailable(Catch2)
set(CMAKE_CXX_VISIBILITY_PRESET ${_tb_saved_cxx_visibility})

suppress_dependency_warnings(Catch2)
suppress_dependency_warnings(Catch2WithMain)

# Make Catch2's CMake helpers (catch_discover_tests) available via include(Catch).
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
