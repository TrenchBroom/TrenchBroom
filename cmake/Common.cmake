SET(COMMON_SOURCE_DIR ${CMAKE_SOURCE_DIR}/common/src)

FILE(GLOB_RECURSE COMMON_SOURCE
    "${COMMON_SOURCE_DIR}/*.cpp"
)

FILE(GLOB_RECURSE COMMON_HEADER
    "${COMMON_SOURCE_DIR}/*.h"
)

ADD_LIBRARY(common SHARED ${COMMON_SOURCE} ${COMMON_HEADER})
SET_XCODE_ATTRIBUTES(common)

INCLUDE_DIRECTORIES(${COMMON_SOURCE_DIR})

# Configure dependencies, etc.

IF(COMPILER_IS_GNU AND TB_ENABLE_ASAN)
	TARGET_LINK_LIBRARIES(common asan)
ENDIF()

TARGET_LINK_LIBRARIES(common glew ${wxWidgets_LIBRARIES} ${FREETYPE_LIBRARIES} ${FREEIMAGE_LIBRARIES} vecmath)

SET_TARGET_PROPERTIES(common PROPERTIES COMPILE_DEFINITIONS "GLEW_STATIC")

IF (COMPILER_IS_MSVC)
	TARGET_LINK_LIBRARIES(common stackwalker)

	# Generate a small stripped PDB for release builds so we get stack traces with symbols
	SET_TARGET_PROPERTIES(common PROPERTIES LINK_FLAGS_RELEASE "/DEBUG /PDBSTRIPPED:Release/common-stripped.pdb /PDBALTPATH:common-stripped.pdb")
ENDIF()

# Create the cmake script for generating the version information
CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/cmake/GenerateVersion.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/GenerateVersion.cmake" @ONLY)
ADD_TARGET_PROPERTY(common INCLUDE_DIRECTORIES ${CMAKE_CURRENT_BINARY_DIR})
ADD_CUSTOM_TARGET(GenerateVersion
		${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/GenerateVersion.cmake")
ADD_DEPENDENCIES(common GenerateVersion)

# CPack

INSTALL(TARGETS common LIBRARY DESTINATION bin COMPONENT TrenchBroom
                       RUNTIME DESTINATION bin COMPONENT TrenchBroom)

# cotire
set_target_properties(common PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "Prefix.h")
cotire(common)
