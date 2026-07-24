message(STATUS "Fetching ctre...")
FetchContent_Declare(
  ctre
  GIT_REPOSITORY https://github.com/hanickadot/compile-time-regular-expressions
  GIT_TAG        e34c26ba149b9fd9c34aa0f678e39739641a0d1e # v3.10.0
  SYSTEM
)

set(CTRE_BUILD_TESTS OFF)
set(CTRE_BUILD_PACKAGE OFF)
set(CTRE_BUILD_PACKAGE_DEB OFF)
set(CTRE_BUILD_PACKAGE_RPM OFF)
FetchContent_MakeAvailable(ctre)
