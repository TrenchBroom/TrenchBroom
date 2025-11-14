set(LIB_EL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(LIB_EL_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(LIB_EL_HEADER
  ${LIB_EL_INCLUDE_DIR}/el/EL_Forward.h
  ${LIB_EL_INCLUDE_DIR}/el/ELExceptions.h
  ${LIB_EL_INCLUDE_DIR}/el/ELParser.h
  ${LIB_EL_INCLUDE_DIR}/el/EvaluationContext.h
  ${LIB_EL_INCLUDE_DIR}/el/Expression.h
  ${LIB_EL_INCLUDE_DIR}/el/Interpolate.h
  ${LIB_EL_INCLUDE_DIR}/el/Types.h
  ${LIB_EL_INCLUDE_DIR}/el/Value.h
  ${LIB_EL_INCLUDE_DIR}/el/VariableStore.h
)

set(LIB_EL_SOURCE
  ${LIB_EL_SOURCE_DIR}/ELExceptions.cpp
  ${LIB_EL_SOURCE_DIR}/ELParser.cpp
  ${LIB_EL_SOURCE_DIR}/EvaluationContext.cpp
  ${LIB_EL_SOURCE_DIR}/Expression.cpp
  ${LIB_EL_SOURCE_DIR}/Interpolate.cpp
  ${LIB_EL_SOURCE_DIR}/Types.cpp
  ${LIB_EL_SOURCE_DIR}/Types.cpp
  ${LIB_EL_SOURCE_DIR}/Value.cpp
  ${LIB_EL_SOURCE_DIR}/VariableStore.cpp
)

add_library(lib-el STATIC ${LIB_EL_SOURCE} ${LIB_EL_HEADER})
target_include_directories(lib-el PUBLIC ${LIB_EL_INCLUDE_DIR})
target_link_libraries(lib-el
  PUBLIC lib-kdl lib-base
  PRIVATE fmt::fmt-header-only)

target_link_libraries(lib-el PRIVATE compiler-config precompile-std-headers)

add_subdirectory(test)
