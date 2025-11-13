set(LIB_BASE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(LIB_BASE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(LIB_BASE_HEADER
  ${LIB_BASE_INCLUDE_DIR}/Color.h
  ${LIB_BASE_INCLUDE_DIR}/ColorChannel.h
  ${LIB_BASE_INCLUDE_DIR}/ColorComponentType.h
  ${LIB_BASE_INCLUDE_DIR}/ColorT.h
  ${LIB_BASE_INCLUDE_DIR}/ColorVariantT.h
  ${LIB_BASE_INCLUDE_DIR}/Ensure.h
  ${LIB_BASE_INCLUDE_DIR}/Error.h
  ${LIB_BASE_INCLUDE_DIR}/FileLocation.h
  ${LIB_BASE_INCLUDE_DIR}/Logger.h
  ${LIB_BASE_INCLUDE_DIR}/LoggerCache.h
  ${LIB_BASE_INCLUDE_DIR}/Macros.h
  ${LIB_BASE_INCLUDE_DIR}/Notifier.h
  ${LIB_BASE_INCLUDE_DIR}/NotifierConnection.h
  ${LIB_BASE_INCLUDE_DIR}/Parser.h
  ${LIB_BASE_INCLUDE_DIR}/ParserException.h
  ${LIB_BASE_INCLUDE_DIR}/ParserStatus.h
  ${LIB_BASE_INCLUDE_DIR}/Result.h
  ${LIB_BASE_INCLUDE_DIR}/Token.h
  ${LIB_BASE_INCLUDE_DIR}/Tokenizer.h
)

set(LIB_BASE_SOURCE
  ${LIB_BASE_SOURCE_DIR}/Color.cpp
  ${LIB_BASE_SOURCE_DIR}/ColorChannel.cpp
  ${LIB_BASE_SOURCE_DIR}/Ensure.cpp
  ${LIB_BASE_SOURCE_DIR}/FileLocation.cpp
  ${LIB_BASE_SOURCE_DIR}/Logger.cpp
  ${LIB_BASE_SOURCE_DIR}/LoggerCache.cpp
  ${LIB_BASE_SOURCE_DIR}/NotifierConnection.cpp
  ${LIB_BASE_SOURCE_DIR}/ParserException.cpp
  ${LIB_BASE_SOURCE_DIR}/ParserStatus.cpp
)

add_library(lib-base STATIC ${LIB_BASE_SOURCE} ${LIB_BASE_HEADER})
target_include_directories(lib-base PUBLIC ${LIB_BASE_INCLUDE_DIR})
target_link_libraries(lib-base
  PUBLIC lib-kdl lib-vm
  PRIVATE fmt::fmt-header-only)

target_link_libraries(lib-base PRIVATE compiler-config precompile-std-headers)

add_subdirectory(test)
