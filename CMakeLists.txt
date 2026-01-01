# Using target_link_library to link an object library to qt requires 3.12
# 3.14 is required for generator expressions in install(CODE)
cmake_minimum_required(VERSION 3.25)

# Fix warning with Ninja and cotire (see https://github.com/sakra/cotire/issues/81)
if(POLICY CMP0058)
  cmake_policy(SET CMP0058 NEW)
endif()

# Instruct cmake not to set default warning levels for MSVC projects (cmake 3.15 or higher)
if(POLICY CMP0092)
  cmake_policy(SET CMP0092 NEW)
endif()

if(POLICY CMP0063)
  cmake_policy(SET CMP0063 NEW)
endif()

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_OVERLAY_PORTS "${CMAKE_SOURCE_DIR}/vcpkg-overlay-ports/freeimage;${CMAKE_SOURCE_DIR}/vcpkg-overlay-ports/openexr")

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

# Enable the source_group command for creating IDE folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set TrenchBroom item as default single startup project (NOTE: CMake 3.6 and newer)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT TrenchBroom)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Using C and CXX because GLEW is C
project(TrenchBroom C CXX)

# Configure CCache if available and requested
if(TB_ENABLE_CCACHE)
  find_program(CCACHE_PATH ccache)

  if(CCACHE_PATH)
    message(STATUS "Found ccache: ${CCACHE_PATH}")

    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT Embedded)
    cmake_policy(SET CMP0141 NEW)

    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PATH}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PATH}")
  endif(CCACHE_PATH)
endif(TB_ENABLE_CCACHE)

# Compiler detection
set(COMPILER_IS_CLANG FALSE)
set(COMPILER_IS_GNU FALSE)
set(COMPILER_IS_MSVC FALSE)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
  set(COMPILER_IS_CLANG TRUE)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(COMPILER_IS_GNU TRUE)
elseif(MSVC EQUAL 1)
  set(COMPILER_IS_MSVC TRUE)
else()
  message(FATAL_ERROR "Unsupported compiler detected.")
endif()

# Request 8MB stack on Windows. (NOTE: /STACK should be used on .exe targets only.)
# Default is 1MB and we need more for our current AABB builder on large maps (https://github.com/TrenchBroom/TrenchBroom/issues/2803)
if(COMPILER_IS_MSVC)
  set(TB_STACK_SIZE 8388608)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:${TB_STACK_SIZE}")

  # This might work in CMake 3.13 and avoid a global setting, but we are on 3.12 for now
  # target_link_options(common INTERFACE "/STACK:${TB_STACK_SIZE}")
endif()

# Enable sanitizers if possible and requested
if(TB_ENABLE_ASAN)
  message(STATUS "Enabling ASan")

  if(COMPILER_IS_CLANG OR COMPILER_IS_GNU)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
  else()
    message(WARNING "TB isn't set up to enable ASan for compiler ${CMAKE_CXX_COMPILER_ID}")
  endif()
endif()

if(TB_ENABLE_TSAN)
  message(STATUS "Enabling TSan")

  if(COMPILER_IS_CLANG OR COMPILER_IS_GNU)
    add_compile_options(-fsanitize=thread)
    add_link_options(-fsanitize=thread)
  else()
    message(WARNING "TB isn't set up to enable TSan for compiler ${CMAKE_CXX_COMPILER_ID}")
  endif()
endif()

if(TB_ENABLE_UBSAN)
  message(STATUS "Enabling UBSan")

  if(COMPILER_IS_CLANG OR COMPILER_IS_GNU)
    add_compile_options(-fsanitize=undefined)
    add_link_options(-fsanitize=undefined)
  else()
    message(WARNING "TB isn't set up to enable UBSan for compiler ${CMAKE_CXX_COMPILER_ID}")
  endif()
endif()

include(cmake/Utils.cmake)
include(cmake/CompilerConfig.cmake)
include(cmake/PrecompileHeaders.cmake)

# Find Git
find_package(Git)

if(NOT GIT_FOUND)
  message(FATAL_ERROR "Could not find git")
endif()

# Find Pandoc
if(NOT PANDOC_PATH)
  find_program(PANDOC_PATH NAMES "pandoc" DOC "Pandoc program location")

  if(PANDOC_PATH STREQUAL "PANDOC_PATH-NOTFOUND")
    message(FATAL_ERROR "Could not find pandoc")
  else()
    message(STATUS "Found Pandoc: ${PANDOC_PATH}")
  endif()
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Prevent gtest from being installed
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

find_package(assimp CONFIG REQUIRED)
find_package(cpptrace CONFIG REQUIRED)
find_package(freeimage CONFIG REQUIRED)
find_package(freetype CONFIG REQUIRED)
find_package(Catch2 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(GLEW REQUIRED)
find_package(miniz CONFIG REQUIRED)
find_package(tinyxml2 CONFIG REQUIRED)
find_package(stduuid REQUIRED)

# Find Qt and OpenGL
find_package(OpenGL REQUIRED)
find_package(Qt6 COMPONENTS Core Widgets OpenGL OpenGLWidgets Network Svg Test REQUIRED)

# Find threads lib, needed to work around a gtest bug, see: https://stackoverflow.com/questions/21116622/undefined-reference-to-pthread-key-create-linker-error
# The googletest target links to this
find_package(Threads)

# Use the fast-float library for from_chars on AppleClang, where std::from_chars is not
# available for floating point types.
if (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
find_package(FastFloat CONFIG REQUIRED)
endif()

# Populate version variables using git
get_git_describe("${GIT_EXECUTABLE}" "${CMAKE_SOURCE_DIR}" GIT_DESCRIBE)
get_app_version(GIT_DESCRIBE APP_VERSION_YEAR APP_VERSION_NUMBER APP_VERSION_RC)
get_app_version_str(APP_VERSION_YEAR APP_VERSION_NUMBER APP_VERSION_RC APP_VERSION_STR)
set(APP_BUILD_TYPE "${CMAKE_BUILD_TYPE}")

# Some global variables used in several targets
set(APP_DIR "${CMAKE_SOURCE_DIR}/app")
set(APP_RESOURCE_DIR "${APP_DIR}/resources")

add_subdirectory(lib)
add_subdirectory(CmdTool)
add_subdirectory(dump-shortcuts)
add_subdirectory(app)
