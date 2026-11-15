add_executable(TbSingleInstanceLibTest)
EMBED_UTF8_MANIFEST(TbSingleInstanceLibTest)
target_sources(TbSingleInstanceLibTest PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_SingleInstance.cpp"
)

target_link_libraries(TbSingleInstanceLibTest
  PRIVATE
    CompilerConfig
    TbSingleInstanceLib
    Catch2::Catch2WithMain
    Qt6::Network
    Qt6::Test
)

set_target_properties(TbSingleInstanceLibTest PROPERTIES AUTOMOC TRUE)

include(Catch)
catch_discover_tests(TbSingleInstanceLibTest)
