add_executable(TbMdlLibTest)

target_sources(TbMdlLibTest PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_AssetUtils.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Autosaver.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_BezierPatch.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Brush.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_BrushBuilder.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_BrushFace.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_BrushNode.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_CommandProcessor.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_DecalDefinition.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_DefParser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_EditorContext.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_EntParser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Entity.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_EntityColorPropertyValue.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_EntityDefinitionFileSpec.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_EntityDefinitionParser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_EntityDefinitionUtils.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_EntityLinkManager.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_EntityModel.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_EntityNode.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_EntityRotation.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_FgdParser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_GameFileSystem.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_GameManager.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Grid.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Group.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_GroupNode.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Issue.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Layer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_LayerNode.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_LinkedGroupUtils.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_LoadAseModel.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_LoadAssimpModel.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_LoadBspModel.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_LoadDdsTexture.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_LoadFreeImageTexture.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_LoadM8Texture.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_LoadMaterialCollections.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_LoadMd3Model.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_LoadMdlModel.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_LoadMipTexture.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_LoadWalTexture.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Map.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_MapHeader.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Map_Assets.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Map_Brushes.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Map_CopyPaste.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Map_Entities.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Map_EntityLinks.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Map_Geometry.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Map_Groups.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Map_Layers.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Map_NodeIndex.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Map_NodeLocking.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Map_NodeVisibility.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Map_Nodes.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Map_Picking.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Map_Selection.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Map_World.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_MaterialUtils.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ModelDefinition.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ModelUtils.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Node.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_NodeIndex.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_NodeQueries.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_NodeReader.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_NodeWriter.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ObjSerializer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Octree.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Palette.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ParseCompilationConfig.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ParseGameConfig.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ParseGameEngineConfig.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_PatchNode.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_PointTrace.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Polyhedron.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_PortalFile.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Quake3ShaderParser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Selection.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Tagging.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Transaction.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_UVCoordSystem.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_UpdateBrushFaceAttributes.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_UpdateLinkedGroupsCommand.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_UpdateLinkedGroupsHelper.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Validation.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_WorldNode.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_WorldReader.cpp
)

add_compile_definitions(CATCH_CONFIG_ENABLE_ALL_STRINGMAKERS=1)

target_link_libraries(TbMdlLibTest PRIVATE CompilerConfig PrecompileStdHeaders)
target_link_libraries(TbMdlLibTest PRIVATE Catch2::Catch2WithMain TbBaseTestUtilsLib TbElTestUtilsLib TbFsTestUtilsLib TbMdlLib TbMdlTestUtilsLib)

# Copy fixtures
set(TEST_FIXTURE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/fixture)
set(TEST_FIXTURE_DEST_DIR $<TARGET_FILE_DIR:TbMdlLibTest>/fixture)

add_custom_command(TARGET TbMdlLibTest POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E rm -rf "${TEST_FIXTURE_DEST_DIR}"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${TEST_FIXTURE_SOURCE_DIR}" "${TEST_FIXTURE_DEST_DIR}/test"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${TB_RESOURCE_DIR}/games" "${TEST_FIXTURE_DEST_DIR}/games"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${TB_RESOURCE_DIR}/games-testing" "${TEST_FIXTURE_DEST_DIR}/games"
)
