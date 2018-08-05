cmake_minimum_required (VERSION 3.1)
# SET(CMAKE_VERBOSE_MAKEFILE ON)

INCLUDE(cmake/Utils.cmake)

# Using C and CXX because GLEW is C
PROJECT(TrenchBroom C CXX)

# Compiler detection
SET(COMPILER_IS_CLANG FALSE)
SET(COMPILER_IS_GNU FALSE)
SET(COMPILER_IS_MSVC FALSE)

IF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    SET(COMPILER_IS_CLANG TRUE)
ENDIF()
IF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    SET(COMPILER_IS_GNU TRUE)
ENDIF()
IF(MSVC EQUAL 1)
    SET(COMPILER_IS_MSVC TRUE)
ENDIF()

# Compiler configuration

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Mac OS X specific configuration. In theory must be done before the PROJECT directive, but that doesn't actually work.
# These settings are ignored on other platforms
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.9)

# Library configuration
SET(LIB_DIR "${CMAKE_SOURCE_DIR}/lib")
SET(LIB_BIN_DIR "${LIB_DIR}/bin")
SET(LIB_INCLUDE_DIR "${LIB_DIR}/include")
SET(LIB_SOURCE_DIR "${LIB_DIR}/src")

INCLUDE(cmake/wxWidgets.cmake)
INCLUDE(cmake/FreeType.cmake)
INCLUDE(cmake/FreeImage.cmake)

INCLUDE(cmake/GTest.cmake)
INCLUDE(cmake/GMock.cmake)
INCLUDE(cmake/Glew.cmake)
INCLUDE(cmake/Common.cmake)
INCLUDE(cmake/StackWalker.cmake)

IF(COMPILER_IS_CLANG)
    MESSAGE(STATUS "Compiler is Clang")
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Weverything -pedantic -Wno-format -Wno-variadic-macros -Wno-c99-extensions -Wno-padded -Wno-unused-parameter -Wno-global-constructors -Wno-exit-time-destructors -Wno-weak-vtables -Wno-weak-template-vtables -Wno-float-equal -Wno-used-but-marked-unused -Wno-format-nonliteral -Wno-missing-noreturn -Wno-unused-local-typedef -Wno-double-promotion -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-implicit-fallthrough -Wno-zero-as-null-pointer-constant")
ELSEIF(COMPILER_IS_GNU)
    MESSAGE(STATUS "Compiler is GNU")
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wno-format -Wno-variadic-macros -Wno-padded -Wno-unused-parameter -Wno-float-equal -Wno-format-nonliteral -Wno-missing-noreturn")
ELSEIF(COMPILER_IS_MSVC)
    MESSAGE(STATUS "Compiler is MSVC")

	STRING(REGEX REPLACE "/W |/W$" "/W3" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	STRING(REGEX REPLACE "/W |/W$" "/W3" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
	
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Ox")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /MP /D _CRT_SECURE_NO_DEPRECATE /D _CRT_NONSTDC_NO_DEPRECATE")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")

    # Generate debug symbols even for Release; we build a stripped pdb in Release mode, see TrenchBroomApp.cmake
    SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
ELSE()
    MESSAGE(FATAL_ERROR "Unsupported compiler detected.")
ENDIF()

IF(TB_ENABLE_ASAN)
    MESSAGE(STATUS "AddressSanitizer requested via TB_ENABLE_ASAN cmake variable")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    SET(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")

    # Workaround for false positives on macOS
    IF(COMPILER_IS_CLANG)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_LIBCPP_HAS_NO_ASAN")
    ENDIF()
ENDIF()

IF(CMAKE_GENERATOR STREQUAL "Xcode")
    # Xcode requires these flags to allow debugging
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -O0 -fno-inline")

    # Xcode should treat the library header files as system headers
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isystem ${LIB_INCLUDE_DIR}")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -isystem ${LIB_INCLUDE_DIR}")
ELSE()
    INCLUDE_DIRECTORIES(SYSTEM "${LIB_INCLUDE_DIR}")
ENDIF()

# Set debug mode directives
IF(CMAKE_BUILD_TYPE MATCHES "Debug")
	ADD_DEFINITIONS(-DWXDEBUG -DDEBUG)
ENDIF()

INCLUDE(cmake/TrenchBroomApp.cmake)
INCLUDE(cmake/TrenchBroomTest.cmake)
