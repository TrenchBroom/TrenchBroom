add_executable(TbBaseLibTest)

target_sources(TbBaseLibTest PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Color.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ColorComponentType.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ColorT.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ColorVariantT.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Notifier.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Tokenizer.cpp
)

add_compile_definitions(CATCH_CONFIG_ENABLE_ALL_STRINGMAKERS=1)

target_link_libraries(TbBaseLibTest PRIVATE CompilerConfig PrecompileStdHeaders)
target_link_libraries(TbBaseLibTest PRIVATE Catch2::Catch2WithMain TbBaseLib)
