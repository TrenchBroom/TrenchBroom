add_executable(TbImgLibTest)
EMBED_UTF8_MANIFEST(TbImgLibTest)

target_sources(TbImgLibTest PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_DecodeBmpPalette.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_DecodeImage.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ImageSupport.cpp
)

add_compile_definitions(CATCH_CONFIG_ENABLE_ALL_STRINGMAKERS=1)

target_link_libraries(TbImgLibTest
  PRIVATE
    CompilerConfig
    PrecompileStdHeaders
    Catch2::Catch2WithMain
    TbBaseTestUtilsLib
    TbImgLib
)

# Copy fixtures
set(TEST_FIXTURE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/fixture)
set(TEST_FIXTURE_DEST_DIR $<TARGET_FILE_DIR:TbImgLibTest>/fixture)

add_custom_command(TARGET TbImgLibTest POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E rm -rf "${TEST_FIXTURE_DEST_DIR}"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${TEST_FIXTURE_SOURCE_DIR}" "${TEST_FIXTURE_DEST_DIR}/test"
)

include(Catch)
catch_discover_tests(TbImgLibTest)
