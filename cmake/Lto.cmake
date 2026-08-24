# Enables link-time / interprocedural optimization for Release builds, for our own targets
# only (this file is included after cmake/Dependencies.cmake, so it does not affect how
# third-party dependencies are built).
#
# LTO is opt-in via TB_ENABLE_LTO, since it slows down the link step considerably. CI only
# sets it for release-tag builds (see CI-*.sh/.bat); PR and master builds skip it, since
# the link-time cost isn't worth paying on every commit for a benefit that only matters
# for the binaries we actually ship.
if(TB_ENABLE_LTO)
  # GCC's LTO miscompiles our code: whole-program IPA analysis across ActionManager's
  # constructor and QApplication's construction corrupts QCoreApplication::self, causing
  # every Qt app we build (DumpShortcuts, TrenchBroom itself) to segfault on startup with
  # sender=0x0 in QMetaObject::activate. Reproduced with both GCC 15.2 (release) and GCC
  # 16.0.1 (trunk snapshot), on both the offscreen and Wayland QPA platforms, so this isn't
  # a version-specific regression we can just wait out. Skip LTO for GCC until this is
  # understood/fixed upstream.
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(WARNING "LTO is disabled for GCC due to a whole-program miscompilation that crashes Qt apps on startup (corrupts QCoreApplication::self)")
  else()
    include(CheckIPOSupported)
    check_ipo_supported(RESULT TB_IPO_SUPPORTED OUTPUT TB_IPO_NOT_SUPPORTED_REASON)

    if(TB_IPO_SUPPORTED)
      message(STATUS "Enabling LTO for Release builds")
      set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
    else()
      message(WARNING "LTO is not supported by this toolchain: ${TB_IPO_NOT_SUPPORTED_REASON}")
    endif()
  endif()
endif()
