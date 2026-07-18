message(STATUS "Fetching fmt...")
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG        40626af88bd7df9a5fb80be7b25ac85b122d6c21 # 11.2.0
  SYSTEM
)

set(FMT_DOC OFF)
set(FMT_TEST OFF)
set(FMT_INSTALL OFF)
FetchContent_MakeAvailable(fmt)

suppress_dependency_warnings(fmt)
