add_library(CompilerConfig INTERFACE)

# see https://github.com/cpp-best-practices/cppbestpractices/blob/master/02-Use_the_Tools_Available.md

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
  # disable warning about duplicate libraries, see https://gitlab.kitware.com/cmake/cmake/-/issues/25297
  if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 15)
    target_link_options(CompilerConfig INTERFACE LINKER:-no_warn_duplicate_libraries)
  endif()

  target_compile_options(CompilerConfig INTERFACE -Wall -Wextra
    -pedantic # warn on language extensions
    -Wshadow-all # warn the user if a variable declaration shadows one from a parent context
    -Wnon-virtual-dtor # warn the user if a class with virtual functions has a non-virtual destructor
    -Wold-style-cast # warn for c-style casts
    -Wcast-align # warn for potential performance problem casts
    -Wunused # warn on anything being unused
    -Woverloaded-virtual # warn if you overload (not override) a virtual function
    -Wconversion # warn on type conversions that may lose data
    -Wsign-conversion # (Clang all versions, GCC >= 4.3) warn on sign conversions
    -Wmisleading-indentation # (only in GCC >= 6.0) warn if indentation implies blocks where blocks do not exist
    -Wnull-dereference # (only in GCC >= 6.0) warn if a null dereference is detected
    # -Wdouble-promotion # (GCC >= 4.6, Clang >= 3.8) warn if float is implicitly promoted to double
    -Wformat=2 # warn on security issues around functions that format output (i.e., printf)
    -Wimplicit-fallthrough # Warns when case statements fall-through. (Included with -Wextra in GCC, not in clang)
  )

  target_compile_options(CompilerConfig INTERFACE "$<$<CONFIG:RELEASE>:-O3>")

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(CompilerConfig INTERFACE -Wall -Wextra
    -pedantic # warn on language extensions
    -Wshadow=local # warn the user if a variable declaration shadows one from a parent context (but only local scopes)
    -Wnon-virtual-dtor # warn the user if a class with virtual functions has a non-virtual destructor
    -Wold-style-cast # warn for c-style casts
    -Wcast-align # warn for potential performance problem casts
    -Wunused # warn on anything being unused
    # -Woverloaded-virtual # warn if you overload (not override) a virtual function
    -Wconversion # warn on type conversions that may lose data
    -Wsign-conversion # (Clang all versions, GCC >= 4.3) warn on sign conversions
    -Wmisleading-indentation # (only in GCC >= 6.0) warn if indentation implies blocks where blocks do not exist
    -Wduplicated-cond # (only in GCC >= 6.0) warn if if / else chain has duplicated conditions
    -Wduplicated-branches # (only in GCC >= 7.0) warn if if / else branches have duplicated code
    -Wlogical-op # (only in GCC) warn about logical operations being used where bitwise were probably wanted
    # -Wnull-dereference # (only in GCC >= 6.0) warn if a null dereference is detected
    # -Wuseless-cast # (only in GCC >= 4.8) warn if you perform a cast to the same type
    # -Wdouble-promotion # (GCC >= 4.6, Clang >= 3.8) warn if float is implicitly promoted to double
    -Wformat=2 # warn on security issues around functions that format output (i.e., printf)
    -Wimplicit-fallthrough # Warns when case statements fall-through. (Included with -Wextra in GCC, not in clang)
  )
  
  target_compile_options(CompilerConfig INTERFACE "$<$<CONFIG:RELEASE>:-O3>")

  # FIXME: enable -Wcpp once we found a workaround for glew / QOpenGLWindow problem, see RenderView.h
  target_compile_options(CompilerConfig INTERFACE -Wno-cpp)

  # gcc <= 7 warns about unused structured bindings, see https://github.com/TrenchBroom/TrenchBroom/issues/3751
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8)
    target_compile_options(CompilerConfig INTERFACE -Wno-unused-variable)
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  target_compile_definitions(CompilerConfig INTERFACE _CRT_SECURE_NO_DEPRECATE _CRT_NONSTDC_NO_DEPRECATE)
  target_compile_options(CompilerConfig INTERFACE /EHsc /MP)
  
  target_compile_options(CompilerConfig INTERFACE /W4
    /w14242 # 'identfier': conversion from 'type1' to 'type1', possible loss of data
    /w14254 # 'operator': conversion from 'type1:field_bits' to 'type2:field_bits', possible loss of data
    /w14263 # 'function': member function does not override any base class virtual member function
    /w14265 # 'classname': class has virtual functions, but destructor is not virtual instances of this class may not be destructed correctly
    /w14287 # 'operator': unsigned/negative constant mismatch
    /w14296 # 'operator': expression is always 'boolean_value'
    /w14311 # 'variable': pointer truncation from 'type1' to 'type2'
    /w14545 # expression before comma evaluates to a function which is missing an argument list
    /w14546 # function call before comma missing argument list
    /w14547 # 'operator': operator before comma has no effect; expected operator with side-effect
    /w14549 # 'operator': operator before comma has no effect; did you intend 'operator'?
    /w14555 # expression has no effect; expected expression with side-effect
    /w14619 # pragma warning: there is no warning number 'number'
    /w14640 # Enable warning on thread unsafe static member initialization
    /w14826 # Conversion from 'type1' to 'type_2' is sign-extended. This may cause unexpected runtime behavior.
    /w14905 # wide string literal cast to 'LPSTR'
    /w14906 # string literal cast to 'LPWSTR'
    /w14928 # illegal copy-initialization; more than one user-defined conversion has been implicitly applied
    /we4289 # nonstandard extension used: 'variable': loop control variable declared in the for-loop is used outside the for-loop scope
  )

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
