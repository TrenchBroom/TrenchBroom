add_library(TbImgLib STATIC)

target_sources(TbImgLib
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DecodeBmpPalette.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DecodeImage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ImageAlphaDomain.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ImageSupport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/InitFreeImage.cpp

  PUBLIC FILE_SET headers TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(TbImgLib
  PRIVATE
    CompilerConfig
    PrecompileStdHeaders
    FreeImage
  PUBLIC
    KdLib
    TbBaseLib
    fmt::fmt-header-only
)

add_subdirectory(test)
