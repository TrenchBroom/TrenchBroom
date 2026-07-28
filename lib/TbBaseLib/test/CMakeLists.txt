add_executable(TbBaseLibTest)
EMBED_UTF8_MANIFEST(TbBaseLibTest)

target_sources(TbBaseLibTest PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Color.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ColorComponentType.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ColorT.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ColorVariantT.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_FileLocation.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Logger.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_LoggerCache.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_LoggingHub.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Notifier.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_NotifierConnection.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ParserException.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ParserStatus.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Preference.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_PreferenceManager.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_SimpleParserStatus.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Tokenizer.cpp
)

add_compile_definitions(CATCH_CONFIG_ENABLE_ALL_STRINGMAKERS=1)

target_link_libraries(TbBaseLibTest
  PRIVATE
    CompilerConfig
    PrecompileStdHeaders
    Catch2::Catch2WithMain
    TbBaseLib
)

include(Catch)
catch_discover_tests(TbBaseLibTest)
