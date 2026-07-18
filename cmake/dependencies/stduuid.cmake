message(STATUS "Fetching stduuid...")
FetchContent_Declare(
  stduuid
  GIT_REPOSITORY https://github.com/mariusbancila/stduuid
  GIT_TAG        3afe7193facd5d674de709fccc44d5055e144d7a # v1.2.3
  SYSTEM
)

# stduuid declares cmake_minimum_required(VERSION 3.7.0), which triggers a
# deprecation warning under CMake 4. Treat it as requesting >= 3.10 to silence it.
set(CMAKE_POLICY_VERSION_MINIMUM 3.10)
FetchContent_MakeAvailable(stduuid)
unset(CMAKE_POLICY_VERSION_MINIMUM)
