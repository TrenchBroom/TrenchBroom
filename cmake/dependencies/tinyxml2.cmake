message(STATUS "Fetching tinyxml2...")
FetchContent_Declare(
  tinyxml2
  GIT_REPOSITORY https://github.com/leethomason/tinyxml2
  GIT_TAG        1dee28e51f9175a31955b9791c74c430fe13dc82 # 9.0.0
  SYSTEM
)

set(tinyxml2_BUILD_TESTING OFF)
FetchContent_MakeAvailable(tinyxml2)

suppress_dependency_warnings(tinyxml2)
