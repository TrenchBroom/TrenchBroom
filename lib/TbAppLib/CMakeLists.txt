add_library(TbAppLib STATIC)

target_sources(TbAppLib
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DrawShapeToolExtension.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DrawShapeToolExtensionManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DrawShapeToolExtensions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DrawShapeToolParameters.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MapDocument.cpp

  PUBLIC FILE_SET headers TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
      ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/DrawShapeToolExtension.h
      ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/DrawShapeToolExtensionKind.h
      ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/DrawShapeToolExtensionManager.h
      ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/DrawShapeToolExtensions.h
      ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/DrawShapeToolParameters.h
      ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/MapDocument.h
)

target_link_libraries(TbAppLib
  PRIVATE
    CompilerConfig
    PrecompileStdHeaders
  PUBLIC
    KdLib
    TbBaseLib
    TbMdlLib
    TbPreferencesLib
    TbRenderLib
    VmLib
)

add_subdirectory(test-utils)
add_subdirectory(test)
