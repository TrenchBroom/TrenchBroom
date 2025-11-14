set(LIB_FS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(LIB_FS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(LIB_FS_HEADER
  ${LIB_FS_INCLUDE_DIR}/fs/DiskFileSystem.h
  ${LIB_FS_INCLUDE_DIR}/fs/DiskIO.h
  ${LIB_FS_INCLUDE_DIR}/fs/File.h
  ${LIB_FS_INCLUDE_DIR}/fs/FileSystem.h
  ${LIB_FS_INCLUDE_DIR}/fs/FileSystemMetadata.h
  ${LIB_FS_INCLUDE_DIR}/fs/ImageFileSystem.h
  ${LIB_FS_INCLUDE_DIR}/fs/PathInfo.h
  ${LIB_FS_INCLUDE_DIR}/fs/PathMatcher.h
  ${LIB_FS_INCLUDE_DIR}/fs/TraversalMode.h
  ${LIB_FS_INCLUDE_DIR}/fs/VirtualFileSystem.h
)

set(LIB_FS_SOURCE
  ${LIB_FS_SOURCE_DIR}/DiskFileSystem.cpp
  ${LIB_FS_SOURCE_DIR}/DiskIO.cpp
  ${LIB_FS_SOURCE_DIR}/File.cpp
  ${LIB_FS_SOURCE_DIR}/FileSystem.cpp
  ${LIB_FS_SOURCE_DIR}/FileSystemMetadata.cpp
  ${LIB_FS_SOURCE_DIR}/ImageFileSystem.cpp
  ${LIB_FS_SOURCE_DIR}/PathInfo.cpp
  ${LIB_FS_SOURCE_DIR}/PathMatcher.cpp
  ${LIB_FS_SOURCE_DIR}/TraversalMode.cpp
  ${LIB_FS_SOURCE_DIR}/VirtualFileSystem.cpp
)

add_library(lib-fs STATIC ${LIB_FS_SOURCE} ${LIB_FS_HEADER})
target_include_directories(lib-fs PUBLIC ${LIB_FS_INCLUDE_DIR})
target_link_libraries(lib-fs
  PUBLIC lib-kdl lib-base
  PRIVATE fmt::fmt-header-only)

target_link_libraries(lib-fs PRIVATE compiler-config precompile-std-headers)

add_subdirectory(test)
