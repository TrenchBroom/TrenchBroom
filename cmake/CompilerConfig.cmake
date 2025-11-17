add_library(CompilerConfig INTERFACE)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
  # disable warning about duplicate libraries, see https://gitlab.kitware.com/cmake/cmake/-/issues/25297
  if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 15)
    target_link_options(CompilerConfig INTERFACE LINKER:-no_warn_duplicate_libraries)
  endif()

  target_compile_options(CompilerConfig INTERFACE -Wall -Wextra -Wconversion -Wshadow-all -Wnon-virtual-dtor -Wmissing-prototypes -pedantic)
  target_compile_options(CompilerConfig INTERFACE -Wno-used-but-marked-unused)

  # disable C++98 compatibility warnings
  target_compile_options(CompilerConfig INTERFACE -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-c++98-compat-bind-to-temporary-copy -Wno-c++20-compat)

  # FIXME: investigate further and turn off these warnings if possible
  target_compile_options(CompilerConfig INTERFACE -Wno-weak-vtables -Wno-weak-template-vtables)
  target_compile_options(CompilerConfig INTERFACE "$<$<CONFIG:RELEASE>:-O3>")

  # FIXME: Remove once we switch to Xcode 10
  target_compile_options(CompilerConfig INTERFACE -Wno-missing-braces)

  # FIXME: Suppress warnings in moc generated files:
  target_compile_options(CompilerConfig INTERFACE -Wno-redundant-parens)

  # Disable a warning in clang when using PCH:
  target_compile_options(CompilerConfig INTERFACE -Wno-pragma-system-header-outside-header)

  if(${CMAKE_VERSION} VERSION_EQUAL "3.24.1") 
    # Disable missing prototype for automoc files, see https://gitlab.kitware.com/cmake/cmake/-/merge_requests/7558
    set_source_files_properties("CompilerConfig_autogen/mocs_compilation.cpp" PROPERTIES COMPILE_FLAGS "-Wno-missing-prototypes")
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(CompilerConfig INTERFACE -Wall -Wextra -Wconversion -Wshadow=local -Wnon-virtual-dtor -Wmissing-declarations -pedantic)
  target_compile_options(CompilerConfig INTERFACE "$<$<CONFIG:RELEASE>:-O3>")

  # FIXME: enable -Wcpp once we found a workaround for glew / QOpenGLWindow problem, see RenderView.h
  target_compile_options(CompilerConfig INTERFACE -Wno-cpp)

  # gcc <= 7 warns about unused structured bindings, see https://github.com/TrenchBroom/TrenchBroom/issues/3751
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8)
    target_compile_options(CompilerConfig INTERFACE -Wno-unused-variable)
  endif()

  if(${CMAKE_VERSION} VERSION_EQUAL "3.24.1") 
    # Disable missing prototype for automoc files, see https://gitlab.kitware.com/cmake/cmake/-/merge_requests/7558
    set_source_files_properties("CompilerConfig_autogen/mocs_compilation.cpp" PROPERTIES COMPILE_FLAGS "-Wno-missing-declarations")
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  target_compile_definitions(CompilerConfig INTERFACE _CRT_SECURE_NO_DEPRECATE _CRT_NONSTDC_NO_DEPRECATE)
  target_compile_options(CompilerConfig INTERFACE /W4 /EHsc /MP)

  # signed/unsigned mismatch: https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4365
  target_compile_options(CompilerConfig INTERFACE /w44365)

  # class has virtual functions, but destructor is not virtual: https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-3-c4265
  target_compile_options(CompilerConfig INTERFACE /w44265)

  # disable warnings on external code: https://blogs.msdn.microsoft.com/vcblog/2017/12/13/broken-warnings-theory/
  target_compile_options(CompilerConfig INTERFACE /experimental:external /external:anglebrackets /external:W0)

  # workaround /external generating some spurious warnings
  # https://developercommunity.visualstudio.com/content/problem/220812/experimentalexternal-generates-a-lot-of-c4193-warn.html
  target_compile_options(CompilerConfig INTERFACE /wd4193)
  
  # required to compile large objects; only needed in debug mode currently as release mode strips enough sections
  # https://learn.microsoft.com/en-us/cpp/build/reference/bigobj-increase-number-of-sections-in-dot-obj-file
  target_compile_options(CompilerConfig INTERFACE "$<$<CONFIG:DEBUG>:/bigobj>")

  target_compile_options(CompilerConfig INTERFACE "$<$<CONFIG:RELEASE>:/Ox>")

  # Generate debug symbols even for Release; we build a stripped pdb in Release mode, see TrenchBroomApp.cmake
  target_compile_options(CompilerConfig INTERFACE "$<$<CONFIG:RELEASE>:/Z7>")
else()
  message(FATAL_ERROR "Cannot set compile options for target CompilerConfig")
endif()
