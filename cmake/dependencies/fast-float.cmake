message(STATUS "Fetching fast_float...")
FetchContent_Declare(
  fast_float
  GIT_REPOSITORY https://github.com/fastfloat/fast_float
  GIT_TAG        3e57d8dcfb0a04b5a8a26b486b54490a2e9b310f # v6.1.4
  SYSTEM
)

set(FASTFLOAT_TEST OFF)
set(FASTFLOAT_INSTALL OFF)
# fast_float declares an old cmake_minimum_required, which triggers a deprecation
# warning under CMake 4. Use >= 3.13 so CMP0077 is NEW and the FASTFLOAT_*
# variables above override fast_float's option() defaults without warnings.
set(CMAKE_POLICY_VERSION_MINIMUM 3.13)
FetchContent_MakeAvailable(fast_float)
unset(CMAKE_POLICY_VERSION_MINIMUM)
