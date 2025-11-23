cmake_minimum_required(VERSION 3.12)

add_library(TbFsLib STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/DiskFileSystem.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/DiskIO.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/DkPakFileSystem.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/File.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/FileSystem.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/FileSystemMetadata.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/IdPakFileSystem.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ImageFileSystem.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PathInfo.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PathMatcher.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Reader.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/TraversalMode.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/VirtualFileSystem.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/WadFileSystem.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ZipFileSystem.cpp
)

target_include_directories(TbFsLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(TbFsLib PRIVATE CompilerConfig PrecompileStdHeaders)
target_link_libraries(TbFsLib PUBLIC KdLib TbBaseLib fmt::fmt-header-only miniz::miniz)

add_subdirectory(test)
add_subdirectory(test-utils)
