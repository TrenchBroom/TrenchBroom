MACRO(SET_XCODE_ATTRIBUTES TARGET)
    # Set Debug information format
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT[variant=Debug] "dwarf")
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT[variant=Release] "dwarf-with-dsym")
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT[variant=RelWithDebInfo] "dwarf-with-dsym")
    # Set some warnings
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_PEDANTIC YES)
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_64_TO_32_BIT_CONVERSION YES)
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_ABOUT_RETURN_TYPE YES)
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_SIGN_COMPARE YES)
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_UNINITIALIZED_AUTOS YES)
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_UNUSED_FUNCTION YES)
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_UNUSED_VARIABLE YES)
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_NON_VIRTUAL_DESTRUCTOR YES)
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_UNDECLARED_SELECTOR YES)
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_CHECK_SWITCH_STATEMENTS YES)
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_SHADOW YES)
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_INITIALIZER_NOT_FULLY_BRACKETED YES)
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_MISSING_PARENTHESES YES)
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_ABOUT_MISSING_FIELD_INITIALIZERS YES)
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_ABOUT_MISSING_NEWLINE YES)
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_ABOUT_POINTER_SIGNEDNESS YES)
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_UNKNOWN_PRAGMAS YES)
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_UNUSED_LABEL YES)
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_UNUSED_VALUE YES)
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_HIDDEN_VIRTUAL_FUNCTIONS YES)
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_GCC_TREAT_INCOMPATIBLE_POINTER_TYPE_WARNINGS_AS_ERRORS YES)

    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_EMPTY_BODY YES)
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_INT_CONVERSION YES)
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_IMPLICIT_SIGN_CONVERSION YES)
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_ENUM_CONVERSION YES)
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_ASSIGN_ENUM YES)
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_SUSPICIOUS_IMPLICIT_CONVERSION YES)
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_UNREACHABLE_CODE YES)
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_CXX0X_EXTENSIONS YES)
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_DIRECT_OBJC_ISA_USAGE YES)
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_OBJC_IMPLICIT_ATOMIC_PROPERTIES YES)
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_SUSPICIOUS_MOVE YES)

    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=Debug] YES)
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=Release] NO)
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=RelWithDebInfo] NO)
ENDMACRO(SET_XCODE_ATTRIBUTES)

MACRO(set_compiler_config TARGET)
    if(COMPILER_IS_CLANG)
        target_compile_options(${TARGET} PRIVATE -Wall -Wextra -pedantic)
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
    elseif(COMPILER_IS_GNU)
        target_compile_options(${TARGET} PRIVATE -Wall -Wextra -pedantic)
        target_compile_options(${TARGET} PRIVATE "$<$<CONFIG:RELEASE>:-O3>")

        # FIXME: enable -Wcpp once we found a workaround for glew / QOpenGLWindow problem, see RenderView.h
        target_compile_options(${TARGET} PRIVATE -Wno-cpp)
    elseif(COMPILER_IS_MSVC)
        target_compile_definitions(${TARGET} PRIVATE _CRT_SECURE_NO_DEPRECATE _CRT_NONSTDC_NO_DEPRECATE)
        target_compile_options(${TARGET} PRIVATE /W4 /EHsc /MP)

        # signed/unsigned mismatch: https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4365
        target_compile_options(${TARGET} PRIVATE /w44365)

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
ENDMACRO(set_compiler_config)

MACRO(FIX_WIN32_PATH VARNAME)
    IF (WIN32)
        STRING(REPLACE "/" "\\" ${VARNAME} "${${VARNAME}}")
    ENDIF()
ENDMACRO(FIX_WIN32_PATH)

MACRO(ADD_TARGET_PROPERTY TARGET PROPERTY VALUE)
    GET_TARGET_PROPERTY(CURRENT_VALUE ${TARGET} ${PROPERTY})
    IF(NOT CURRENT_VALUE)
        SET_PROPERTY(TARGET ${TARGET} PROPERTY ${PROPERTY} ${VALUE})
    ELSE()
        LIST(APPEND CURRENT_VALUE ${VALUE})
        SET_PROPERTY(TARGET ${TARGET} PROPERTY ${PROPERTY} ${CURRENT_VALUE})
    ENDIF()
ENDMACRO(ADD_TARGET_PROPERTY)

MACRO(GET_APP_VERSION GIT_DESCRIBE VERSION_YEAR VERSION_NUMBER)
    IF(NOT ${GIT_DESCRIBE})
        SET(${GIT_DESCRIBE} "v0000.0")
    ENDIF()
    STRING(REGEX MATCH "v([0-9][0-9][0-9][0-9])[.]([0-9]+)" GIT_DESCRIBE_MATCH "${${GIT_DESCRIBE}}")

    IF(GIT_DESCRIBE_MATCH)
        SET(${VERSION_YEAR} ${CMAKE_MATCH_1})
        SET(${VERSION_NUMBER} ${CMAKE_MATCH_2})
    ELSE()
        MESSAGE(FATAL_ERROR "Couldn't parse version from git describe output '${${GIT_DESCRIBE}}'")
    ENDIF()
ENDMACRO(GET_APP_VERSION)

MACRO(GET_GIT_DESCRIBE GIT SOURCE_DIR GIT_DESCRIBE)
    EXECUTE_PROCESS(COMMAND ${GIT} describe --dirty WORKING_DIRECTORY ${SOURCE_DIR} OUTPUT_VARIABLE ${GIT_DESCRIBE} OUTPUT_STRIP_TRAILING_WHITESPACE)
ENDMACRO(GET_GIT_DESCRIBE)

MACRO(GET_BUILD_PLATFORM PLATFORM_NAME)
    IF(WIN32)
        SET(${PLATFORM_NAME} "Win32")
    ELSEIF(APPLE)
        SET(${PLATFORM_NAME} "MacOSX")
    ELSEIF(UNIX)
        SET(${PLATFORM_NAME} "Linux")
    ELSE()
        SET(${PLATFORM_NAME} "Unknown")
    ENDIF()
ENDMACRO(GET_BUILD_PLATFORM)
