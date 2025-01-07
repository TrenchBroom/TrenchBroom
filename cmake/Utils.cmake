macro(SET_XCODE_ATTRIBUTES TARGET)
    if (CMAKE_GENERATOR STREQUAL "Xcode")
        # Set Debug information format
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT[variant=Debug] "dwarf")
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT[variant=Release] "dwarf-with-dsym")
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT[variant=RelWithDebInfo] "dwarf-with-dsym")

        # Set some warnings
        # See https://github.com/jonreid/XcodeWarnings/blob/master/XcodeWarnings.xcconfig

        # Warning policies
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_PEDANTIC YES)

        # All languages
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_ASSIGN_ENUM YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_BOOL_CONVERSION YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_COMMA YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_CONSTANT_CONVERSION YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_DOCUMENTATION_COMMENTS YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_EMPTY_BODY YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_ENUM_CONVERSION YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_IMPLICIT_SIGN_CONVERSION YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_INFINITE_RECURSION YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_INT_CONVERSION YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_NON_LITERAL_NULL_CONVERSION YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_PRAGMA_PACK YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_PRIVATE_MODULE YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_QUOTED_INCLUDE_IN_FRAMEWORK_HEADER YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_SEMICOLON_BEFORE_METHOD_BODY YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_STRICT_PROTOTYPES YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_SUSPICIOUS_IMPLICIT_CONVERSION YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_UNGUARDED_AVAILABILITY YES_AGGRESSIVE)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_UNREACHABLE_CODE YES_AGGRESSIVE)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_CLANG_WARN_FLOAT_CONVERSION YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_TREAT_IMPLICIT_FUNCTION_DECLARATIONS_AS_ERRORS YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_TREAT_INCOMPATIBLE_POINTER_TYPE_WARNINGS_AS_ERRORS YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_64_TO_32_BIT_CONVERSION YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_ABOUT_DEPRECATED_FUNCTIONS YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_ABOUT_MISSING_FIELD_INITIALIZERS YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_ABOUT_MISSING_NEWLINE YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_ABOUT_MISSING_PROTOTYPES YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_ABOUT_POINTER_SIGNEDNESS YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_ABOUT_RETURN_TYPE YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_CHECK_SWITCH_STATEMENTS YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_FOUR_CHARACTER_CONSTANTS YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_HIDDEN_VIRTUAL_FUNCTIONS YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_INITIALIZER_NOT_FULLY_BRACKETED YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_MISSING_PARENTHESES YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_SHADOW YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_SIGN_COMPARE YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_TYPECHECK_CALLS_TO_PRINTF YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_UNINITIALIZED_AUTOS YES_AGGRESSIVE)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_UNKNOWN_PRAGMAS YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_UNUSED_FUNCTION YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_UNUSED_LABEL YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_UNUSED_PARAMETER YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_UNUSED_VALUE YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_UNUSED_VARIABLE YES)

        # C++
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN__EXIT_TIME_DESTRUCTORS YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_ATOMIC_IMPLICIT_SEQ_CST YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_CXX0X_EXTENSIONS YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_DELETE_NON_VIRTUAL_DTOR YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_RANGE_LOOP_ANALYSIS YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_SUSPICIOUS_MOVE YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_VEXING_PARSE YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_ABOUT_INVALID_OFFSETOF_MACRO YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_NON_VIRTUAL_DESTRUCTOR YES)

        # Undefined behavior sanitizer
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_UNDEFINED_BEHAVIOR_SANITIZER_INTEGER YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_UNDEFINED_BEHAVIOR_SANITIZER_NULLABILITY YES)

        # Address sanitizer
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_ADDRESS_SANITIZER_CONTAINER_OVERFLOW YES)

        # Code generation
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_STRICT_ALIASING YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_REUSE_STRINGS YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_NO_COMMON_BLOCKS YES)


        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=Debug] YES)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=Release] NO)
        set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=RelWithDebInfo] NO)
    endif()
endmacro(SET_XCODE_ATTRIBUTES)

macro(set_compiler_config TARGET)
    if(COMPILER_IS_CLANG)
        # disable warning about duplicate libraries, see https://gitlab.kitware.com/cmake/cmake/-/issues/25297
        if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 15)
            target_link_options(${TARGET} PRIVATE LINKER:-no_warn_duplicate_libraries)
        endif()

        target_compile_options(${TARGET} PRIVATE -Wall -Wextra -Wconversion -Wshadow-all -Wnon-virtual-dtor -Wmissing-prototypes -pedantic)
        target_compile_options(${TARGET} PRIVATE -Wno-global-constructors -Wno-exit-time-destructors -Wno-padded -Wno-format-nonliteral -Wno-used-but-marked-unused)

        # disable C++98 compatibility warnings
        target_compile_options(${TARGET} PRIVATE -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-c++98-compat-bind-to-temporary-copy)

        # FIXME: investigate further and turn off these warnings if possible
        target_compile_options(${TARGET} PRIVATE -Wno-weak-vtables -Wno-weak-template-vtables)
        target_compile_options(${TARGET} PRIVATE "$<$<CONFIG:RELEASE>:-O3>")

        # FIXME: Remove once we switch to Xcode 10
        target_compile_options(${TARGET} PRIVATE -Wno-missing-braces)

        # FIXME: Suppress warnings in moc generated files:
        target_compile_options(${TARGET} PRIVATE -Wno-redundant-parens)

        # Disable a warning in clang when using PCH:
        target_compile_options(${TARGET} PRIVATE -Wno-pragma-system-header-outside-header)

        if(${CMAKE_VERSION} VERSION_EQUAL "3.24.1") 
            # Disable missing prototype for automoc files, see https://gitlab.kitware.com/cmake/cmake/-/merge_requests/7558
            set_source_files_properties("${TARGET}_autogen/mocs_compilation.cpp" PROPERTIES COMPILE_FLAGS "-Wno-missing-prototypes")
        endif()
  elseif(COMPILER_IS_GNU)
        target_compile_options(${TARGET} PRIVATE -Wall -Wextra -Wconversion -Wshadow=local -Wnon-virtual-dtor -Wmissing-declarations -pedantic)
        target_compile_options(${TARGET} PRIVATE "$<$<CONFIG:RELEASE>:-O3>")

        # FIXME: enable -Wcpp once we found a workaround for glew / QOpenGLWindow problem, see RenderView.h
        target_compile_options(${TARGET} PRIVATE -Wno-cpp)

        # gcc <= 7 warns about unused structured bindings, see https://github.com/TrenchBroom/TrenchBroom/issues/3751
        if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8)
            target_compile_options(${TARGET} PRIVATE -Wno-unused-variable)
        endif()

        if(${CMAKE_VERSION} VERSION_EQUAL "3.24.1") 
            # Disable missing prototype for automoc files, see https://gitlab.kitware.com/cmake/cmake/-/merge_requests/7558
            set_source_files_properties("${TARGET}_autogen/mocs_compilation.cpp" PROPERTIES COMPILE_FLAGS "-Wno-missing-declarations")
        endif()
  elseif(COMPILER_IS_MSVC)
        target_compile_definitions(${TARGET} PRIVATE _CRT_SECURE_NO_DEPRECATE _CRT_NONSTDC_NO_DEPRECATE)
        target_compile_options(${TARGET} PRIVATE /W4 /EHsc /MP)

        # signed/unsigned mismatch: https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4365
        target_compile_options(${TARGET} PRIVATE /w44365)

        # class has virtual functions, but destructor is not virtual: https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-3-c4265
        target_compile_options(${TARGET} PRIVATE /w44265)

        # disable warnings on external code: https://blogs.msdn.microsoft.com/vcblog/2017/12/13/broken-warnings-theory/
        target_compile_options(${TARGET} PRIVATE /experimental:external /external:anglebrackets /external:W0)

        # workaround /external generating some spurious warnings
        # https://developercommunity.visualstudio.com/content/problem/220812/experimentalexternal-generates-a-lot-of-c4193-warn.html
        target_compile_options(${TARGET} PRIVATE /wd4193)

        target_compile_options(${TARGET} PRIVATE "$<$<CONFIG:RELEASE>:/Ox>")

        # Generate debug symbols even for Release; we build a stripped pdb in Release mode, see TrenchBroomApp.cmake
        target_compile_options(${TARGET} PRIVATE "$<$<CONFIG:RELEASE>:/Zi>")
    else()
        message(FATAL_ERROR "Cannot set compile options for target ${TARGET}")
    endif()
    SET_XCODE_ATTRIBUTES(${TARGET})
endmacro(set_compiler_config)

macro(FIX_WIN32_PATH VARNAME)
    if(WIN32)
        STRING(REPLACE "/" "\\" ${VARNAME} "${${VARNAME}}")
    endif()
endmacro(FIX_WIN32_PATH)

macro(ADD_TARGET_PROPERTY TARGET PROPERTY VALUE)
    get_target_property(CURRENT_VALUE ${TARGET} ${PROPERTY})
    if(NOT CURRENT_VALUE)
        set_property(TARGET ${TARGET} PROPERTY ${PROPERTY} ${VALUE})
    else()
        LIST(APPEND CURRENT_VALUE ${VALUE})
        set_property(TARGET ${TARGET} PROPERTY ${PROPERTY} ${CURRENT_VALUE})
    endif()
endmacro(ADD_TARGET_PROPERTY)

macro(GET_APP_VERSION GIT_DESCRIBE VERSION_YEAR VERSION_NUMBER)
    STRING(REGEX MATCH "v([0-9][0-9][0-9][0-9])[.]([0-9]+)" GIT_DESCRIBE_MATCH "${${GIT_DESCRIBE}}")

    if(GIT_DESCRIBE_MATCH)
        set(${VERSION_YEAR} ${CMAKE_MATCH_1})
        set(${VERSION_NUMBER} ${CMAKE_MATCH_2})
    else()
        # GIT_DESCRIBE is not a release tag following the "v0000.0" format.
        set(${VERSION_YEAR} "0")
        set(${VERSION_NUMBER} "0")
    endif()
endmacro(GET_APP_VERSION)

macro(GET_GIT_DESCRIBE GIT SOURCE_DIR GIT_DESCRIBE)
    # When building tags, GitHub Actions checks them out as lightweight tags even if the original tag was annotated.
    # Pass --tags to enable git describe to match non-annotated tags.
    execute_process(COMMAND ${GIT} describe --dirty --tags WORKING_DIRECTORY ${SOURCE_DIR} OUTPUT_VARIABLE ${GIT_DESCRIBE} OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(${GIT_DESCRIBE})
        message(STATUS "Using version description \"${${GIT_DESCRIBE}}\" from git describe")
    endif()

    # On GitHub Actions, "git describe" will fail due to it being a shallow clone (for PR and branch builds),
    # and it is also useful to record in the version description whether it's a branch or PR build.
    # In this case, use:
    #  - TB_PULL_REQUEST_HEAD_SHA (ci.yml sets to the last commit on the PR branch before the merge commit)
    #  - or GITHUB_SHA (set by GitHub Actions)
    # and GITHUB_REF to construct a descriptive version string.
    # See: https://docs.github.com/en/actions/reference/environment-variables#default-environment-variables
    if(NOT ${GIT_DESCRIBE})
        if((NOT ("$ENV{GITHUB_SHA}" STREQUAL "")) AND (NOT ("$ENV{TB_PULL_REQUEST_HEAD_SHA}" STREQUAL "")))
            set(${GIT_DESCRIBE} "unstable-$ENV{GITHUB_REF}-$ENV{TB_PULL_REQUEST_HEAD_SHA}")
            # Replace / with _ because we need GIT_DESCRIBE to be valid to put in a filename for the final package
            string(REPLACE "/" "_" ${GIT_DESCRIBE} ${${GIT_DESCRIBE}})
            message(STATUS "Using version description \"${${GIT_DESCRIBE}}\" from environment variables TB_PULL_REQUEST_HEAD_SHA and GITHUB_REF")
        endif()
    endif()

    if(NOT ${GIT_DESCRIBE})
        if((NOT ("$ENV{GITHUB_SHA}" STREQUAL "")) AND (NOT ("$ENV{GITHUB_REF}" STREQUAL "")))
            set(${GIT_DESCRIBE} "unstable-$ENV{GITHUB_REF}-$ENV{GITHUB_SHA}")
            string(REPLACE "/" "_" ${GIT_DESCRIBE} ${${GIT_DESCRIBE}})
            message(STATUS "Using version description \"${${GIT_DESCRIBE}}\" from environment variables GITHUB_SHA and GITHUB_REF")
        endif()
    endif()

    if(NOT ${GIT_DESCRIBE})
        set(${GIT_DESCRIBE} "unknown")
        message(WARNING "Unable to determine version description; using \"${${GIT_DESCRIBE}}\"")
    endif()
endmacro(GET_GIT_DESCRIBE)

macro(GET_BUILD_PLATFORM PLATFORM_NAME)
    if(WIN32)
        set(${PLATFORM_NAME} "Win64")
    elseif(APPLE)
        set(${PLATFORM_NAME} "macOS")
    elseif(UNIX)
        set(${PLATFORM_NAME} "Linux")
    else()
        set(${PLATFORM_NAME} "Unknown")
    endif()
endmacro(GET_BUILD_PLATFORM)
