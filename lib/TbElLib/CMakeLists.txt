add_library(TbElLib STATIC)

target_sources(TbElLib
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ELParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EvaluationContext.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Exceptions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Expression.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Interpolate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ParseExpression.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Types.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Value.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/VariableStore.cpp

  PUBLIC FILE_SET headers TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(TbElLib PRIVATE CompilerConfig PrecompileStdHeaders)
target_link_libraries(TbElLib PUBLIC KdLib TbBaseLib fmt::fmt-header-only)

add_subdirectory(test)
add_subdirectory(test-utils)
