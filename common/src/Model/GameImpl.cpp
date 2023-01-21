/*
 Copyright (C) 2010-2017 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "GameImpl.h"

#include "Assets/EntityDefinitionFileSpec.h"
#include "Assets/EntityModel.h"
#include "Assets/Palette.h"
#include "Ensure.h"
#include "Exceptions.h"
#include "IO/AseParser.h"
#include "IO/AssimpParser.h"
#include "IO/BrushFaceReader.h"
#include "IO/Bsp29Parser.h"
#include "IO/DefParser.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/DkmParser.h"
#include "IO/EntParser.h"
#include "IO/ExportOptions.h"
#include "IO/FgdParser.h"
#include "IO/File.h"
#include "IO/FileMatcher.h"
#include "IO/GameConfigParser.h"
#include "IO/IOUtils.h"
#include "IO/ImageSpriteParser.h"
#include "IO/Md2Parser.h"
#include "IO/Md3Parser.h"
#include "IO/MdlParser.h"
#include "IO/MdxParser.h"
#include "IO/NodeReader.h"
#include "IO/NodeWriter.h"
#include "IO/ObjParser.h"
#include "IO/ObjSerializer.h"
#include "IO/SimpleParserStatus.h"
#include "IO/SprParser.h"
#include "IO/SystemPaths.h"
#include "IO/TextureLoader.h"
#include "IO/WorldReader.h"
#include "Logger.h"
#include "Macros.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushError.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityProperties.h"
#include "Model/GameConfig.h"
#include "Model/LayerNode.h"
#include "Model/WorldNode.h"

#include <kdl/overload.h>
#include <kdl/result.h>
#include <kdl/string_compare.h>
#include <kdl/string_format.h>
#include <kdl/string_utils.h>
#include <kdl/vector_utils.h>

#include <vecmath/vec_io.h>

#include <fstream>
#include <string>
#include <vector>

namespace TrenchBroom
{
namespace Model
{
GameImpl::GameImpl(GameConfig& config, IO::Path gamePath, Logger& logger)
  : m_config{config}
  , m_gamePath{std::move(gamePath)}
{
  initializeFileSystem(logger);
}

void GameImpl::initializeFileSystem(Logger& logger)
{
  m_fs.initialize(m_config, m_gamePath, m_additionalSearchPaths, logger);
}

const std::string& GameImpl::doGameName() const
{
  return m_config.name;
}

IO::Path GameImpl::doGamePath() const
{
  return m_gamePath;
}

void GameImpl::doSetGamePath(const IO::Path& gamePath, Logger& logger)
{
  if (gamePath != m_gamePath)
  {
    m_gamePath = gamePath;
    initializeFileSystem(logger);
  }
}

void GameImpl::doSetAdditionalSearchPaths(
  const std::vector<IO::Path>& searchPaths, Logger& logger)
{
  if (searchPaths != m_additionalSearchPaths)
  {
    m_additionalSearchPaths = searchPaths;
    initializeFileSystem(logger);
  }
}

Game::PathErrors GameImpl::doCheckAdditionalSearchPaths(
  const std::vector<IO::Path>& searchPaths) const
{
  auto result = PathErrors{};
  for (const auto& searchPath : searchPaths)
  {
    const auto absPath = m_gamePath + searchPath;
    if (!absPath.isAbsolute() || !IO::Disk::directoryExists(absPath))
    {
      result.emplace(searchPath, "Directory not found: '" + searchPath.asString() + "'");
    }
  }
  return result;
}

const CompilationConfig& GameImpl::doCompilationConfig()
{
  return m_config.compilationConfig;
}

size_t GameImpl::doMaxPropertyLength() const
{
  return m_config.maxPropertyLength;
}

std::optional<vm::bbox3> GameImpl::doSoftMapBounds() const
{
  return m_config.softMapBounds;
}

Game::SoftMapBounds GameImpl::doExtractSoftMapBounds(const Entity& entity) const
{
  if (const auto* mapValue = entity.property(EntityPropertyKeys::SoftMapBounds))
  {
    return *mapValue == EntityPropertyValues::NoSoftMapBounds
             ? SoftMapBounds{SoftMapBoundsType::Map, std::nullopt}
             : SoftMapBounds{
               SoftMapBoundsType::Map, IO::parseSoftMapBoundsString(*mapValue)};
  }

  // Not set in map -> use Game value
  return SoftMapBounds{SoftMapBoundsType::Game, doSoftMapBounds()};
}

const std::vector<SmartTag>& GameImpl::doSmartTags() const
{
  return m_config.smartTags;
}

std::unique_ptr<WorldNode> GameImpl::doNewMap(
  const MapFormat format, const vm::bbox3& worldBounds, Logger& logger) const
{
  const auto initialMapFilePath = m_config.findInitialMap(formatName(format));
  if (!initialMapFilePath.isEmpty() && IO::Disk::fileExists(initialMapFilePath))
  {
    return doLoadMap(format, worldBounds, initialMapFilePath, logger);
  }

  auto propertyConfig = entityPropertyConfig();
  auto worldEntity = Model::Entity{};
  if (
    format == MapFormat::Valve || format == MapFormat::Quake2_Valve
    || format == MapFormat::Quake3_Valve)
  {
    worldEntity.addOrUpdateProperty(
      propertyConfig, EntityPropertyKeys::ValveVersion, "220");
  }

  auto worldNode = std::make_unique<WorldNode>(
    std::move(propertyConfig), std::move(worldEntity), format);

  const auto builder =
    Model::BrushBuilder{worldNode->mapFormat(), worldBounds, defaultFaceAttribs()};
  builder.createCuboid({128.0, 128.0, 32.0}, Model::BrushFaceAttributes::NoTextureName)
    .visit(kdl::overload(
      [&](Brush&& b) {
        worldNode->defaultLayer()->addChild(new BrushNode{std::move(b)});
      },
      [&](const Model::BrushError e) {
        logger.error() << "Could not create default brush: " << e;
      }));

  return worldNode;
}

std::unique_ptr<WorldNode> GameImpl::doLoadMap(
  const MapFormat format,
  const vm::bbox3& worldBounds,
  const IO::Path& path,
  Logger& logger) const
{
  auto parserStatus = IO::SimpleParserStatus{logger};
  auto file = IO::Disk::openFile(IO::Disk::fixPath(path));
  auto fileReader = file->reader().buffer();
  if (format == MapFormat::Unknown)
  {
    // Try all formats listed in the game config
    const auto possibleFormats = kdl::vec_transform(
      m_config.fileFormats,
      [](const auto& config) { return Model::formatFromName(config.format); });

    return IO::WorldReader::tryRead(
      fileReader.stringView(),
      possibleFormats,
      worldBounds,
      entityPropertyConfig(),
      parserStatus);
  }
  else
  {
    auto worldReader =
      IO::WorldReader{fileReader.stringView(), format, entityPropertyConfig()};
    return worldReader.read(worldBounds, parserStatus);
  }
}

void GameImpl::doWriteMap(
  WorldNode& world, const IO::Path& path, const bool exporting) const
{
  const auto mapFormatName = formatName(world.mapFormat());

  auto file = openPathAsOutputStream(path);
  if (!file)
  {
    throw FileSystemException{"Cannot open file: " + path.asString()};
  }
  IO::writeGameComment(file, gameName(), mapFormatName);

  auto writer = IO::NodeWriter{world, file};
  writer.setExporting(exporting);
  writer.writeMap();
}

void GameImpl::doWriteMap(WorldNode& world, const IO::Path& path) const
{
  doWriteMap(world, path, false);
}

void GameImpl::doExportMap(WorldNode& world, const IO::ExportOptions& options) const
{
  std::visit(
    kdl::overload(
      [&](const IO::ObjExportOptions& objOptions) {
        auto objFile = openPathAsOutputStream(objOptions.exportPath);
        if (!objFile)
        {
          throw FileSystemException{
            "Cannot open file: " + objOptions.exportPath.asString()};
        }

        auto mtlPath = objOptions.exportPath.replaceExtension("mtl");
        auto mtlFile = openPathAsOutputStream(mtlPath);
        if (!mtlFile)
        {
          throw FileSystemException{"Cannot open file: " + mtlPath.asString()};
        }

        auto writer = IO::NodeWriter{
          world,
          std::make_unique<IO::ObjSerializer>(
            objFile, mtlFile, mtlPath.filename(), objOptions)};
        writer.setExporting(true);
        writer.writeMap();
      },
      [&](const IO::MapExportOptions& mapOptions) {
        doWriteMap(world, mapOptions.exportPath, true);
      }),
    options);
}

std::vector<Node*> GameImpl::doParseNodes(
  const std::string& str,
  const MapFormat mapFormat,
  const vm::bbox3& worldBounds,
  const std::vector<std::string>& linkedGroupsToKeep,
  Logger& logger) const
{
  auto parserStatus = IO::SimpleParserStatus{logger};
  return IO::NodeReader::read(
    str,
    mapFormat,
    worldBounds,
    entityPropertyConfig(),
    linkedGroupsToKeep,
    parserStatus);
}

std::vector<BrushFace> GameImpl::doParseBrushFaces(
  const std::string& str,
  const MapFormat mapFormat,
  const vm::bbox3& worldBounds,
  Logger& logger) const
{
  auto parserStatus = IO::SimpleParserStatus{logger};
  auto reader = IO::BrushFaceReader{str, mapFormat};
  return reader.read(worldBounds, parserStatus);
}

void GameImpl::doWriteNodesToStream(
  WorldNode& world, const std::vector<Node*>& nodes, std::ostream& stream) const
{
  auto writer = IO::NodeWriter{world, stream};
  writer.writeNodes(nodes);
}

void GameImpl::doWriteBrushFacesToStream(
  WorldNode& world, const std::vector<BrushFace>& faces, std::ostream& stream) const
{
  auto writer = IO::NodeWriter{world, stream};
  writer.writeBrushFaces(faces);
}

Game::TexturePackageType GameImpl::doTexturePackageType() const
{
  using Model::GameConfig;
  return std::visit(
    kdl::overload(
      [](const TextureFilePackageConfig&) { return TexturePackageType::File; },
      [](const TextureDirectoryPackageConfig&) { return TexturePackageType::Directory; }),
    m_config.textureConfig.package);
}

void GameImpl::doLoadTextureCollections(
  const Entity& entity,
  const IO::Path& documentPath,
  Assets::TextureManager& textureManager,
  Logger& logger) const
{
  const auto paths = extractTextureCollections(entity);

  const auto fileSearchPaths = textureCollectionSearchPaths(documentPath);
  auto textureLoader =
    IO::TextureLoader{m_fs, fileSearchPaths, m_config.textureConfig, logger};
  textureLoader.loadTextures(paths, textureManager);
}

std::vector<IO::Path> GameImpl::textureCollectionSearchPaths(
  const IO::Path& documentPath) const
{
  return {
    documentPath, // Search for assets relative to the map file.
    m_gamePath,   // Search for assets relative to the location of the game.
    IO::SystemPaths::appDirectory(), // Search for assets relative to the application.
  };
}

bool GameImpl::doIsTextureCollection(const IO::Path& path) const
{
  return std::visit(
    kdl::overload(
      [&](const TextureFilePackageConfig& filePackageConfig) {
        return path.hasExtension(filePackageConfig.fileFormat.extensions, false);
      },
      [](const TextureDirectoryPackageConfig&) { return false; }),
    m_config.textureConfig.package);
}

std::vector<IO::Path> GameImpl::doFindTextureCollections() const
{
  try
  {
    const auto searchPath = getRootDirectory(m_config.textureConfig.package);
    if (!searchPath.isEmpty() && m_fs.directoryExists(searchPath))
    {
      return kdl::vec_concat(
        std::vector<IO::Path>{searchPath},
        m_fs.findItemsRecursively(
          searchPath, IO::FileTypeMatcher{!true(files), true(directories)}));
    }
    return {};
  }
  catch (FileSystemException& e)
  {
    throw GameException{"Could not find texture collections: " + std::string{e.what()}};
  }
}

std::vector<std::string> GameImpl::doFileTextureCollectionExtensions() const
{
  return std::visit(
    kdl::overload(
      [](const TextureFilePackageConfig& filePackageConfig) {
        return filePackageConfig.fileFormat.extensions;
      },
      [](const TextureDirectoryPackageConfig&) { return std::vector<std::string>{}; }),
    m_config.textureConfig.package);
}

std::vector<IO::Path> GameImpl::doExtractTextureCollections(const Entity& entity) const
{
  if (const auto& propertyKey = m_config.textureConfig.property; !propertyKey.empty())
  {
    if (const auto* pathsValue = entity.property(propertyKey))
    {
      return IO::Path::asPaths(kdl::str_split(*pathsValue, ";"));
    }
  }

  return {};
}

void GameImpl::doUpdateTextureCollections(
  Entity& entity, const std::vector<IO::Path>& paths) const
{
  if (const auto& propertyKey = m_config.textureConfig.property; !propertyKey.empty())
  {
    const auto value = kdl::str_join(IO::Path::asStrings(paths, "/"), ";");
    entity.addOrUpdateProperty(entityPropertyConfig(), propertyKey, value);
  }
}

void GameImpl::doReloadShaders()
{
  m_fs.reloadShaders();
}

bool GameImpl::doIsEntityDefinitionFile(const IO::Path& path) const
{
  static const auto extensions = {"fgd", "def", "ent"};

  return std::any_of(extensions.begin(), extensions.end(), [&](const auto& extension) {
    return kdl::ci::str_is_equal(extension, path.extension());
  });
}

std::vector<Assets::EntityDefinition*> GameImpl::doLoadEntityDefinitions(
  IO::ParserStatus& status, const IO::Path& path) const
{
  const auto extension = path.extension();
  const auto& defaultColor = m_config.entityConfig.defaultColor;

  if (kdl::ci::str_is_equal("fgd", extension))
  {
    auto file = IO::Disk::openFile(IO::Disk::fixPath(path));
    auto reader = file->reader().buffer();
    auto parser = IO::FgdParser{reader.stringView(), defaultColor, file->path()};
    return parser.parseDefinitions(status);
  }
  if (kdl::ci::str_is_equal("def", extension))
  {
    auto file = IO::Disk::openFile(IO::Disk::fixPath(path));
    auto reader = file->reader().buffer();
    auto parser = IO::DefParser{reader.stringView(), defaultColor};
    return parser.parseDefinitions(status);
  }
  if (kdl::ci::str_is_equal("ent", extension))
  {
    auto file = IO::Disk::openFile(IO::Disk::fixPath(path));
    auto reader = file->reader().buffer();
    auto parser = IO::EntParser{reader.stringView(), defaultColor};
    return parser.parseDefinitions(status);
  }

  throw GameException{"Unknown entity definition format: '" + path.asString() + "'"};
}

std::vector<Assets::EntityDefinitionFileSpec> GameImpl::doAllEntityDefinitionFiles() const
{
  return kdl::vec_transform(m_config.entityConfig.defFilePaths, [](const auto& path) {
    return Assets::EntityDefinitionFileSpec::builtin(path);
  });
}

Assets::EntityDefinitionFileSpec GameImpl::doExtractEntityDefinitionFile(
  const Entity& entity) const
{
  if (const auto* defValue = entity.property(EntityPropertyKeys::EntityDefinitions))
  {
    return Assets::EntityDefinitionFileSpec::parse(*defValue);
  }

  return defaultEntityDefinitionFile();
}

Assets::EntityDefinitionFileSpec GameImpl::defaultEntityDefinitionFile() const
{
  if (const auto paths = m_config.entityConfig.defFilePaths; !paths.empty())
  {
    return Assets::EntityDefinitionFileSpec::builtin(paths.front());
  }

  throw GameException{"No entity definition files found for game '" + gameName() + "'"};
}

IO::Path GameImpl::doFindEntityDefinitionFile(
  const Assets::EntityDefinitionFileSpec& spec,
  const std::vector<IO::Path>& searchPaths) const
{
  if (!spec.valid())
  {
    throw GameException{"Invalid entity definition file spec"};
  }

  const auto& path = spec.path();
  if (spec.builtin())
  {
    return m_config.findConfigFile(path);
  }

  if (path.isAbsolute())
  {
    return path;
  }

  return IO::Disk::resolvePath(searchPaths, path);
}

template <typename GetPalette, typename Function>
static auto withEntityParser(
  const GameFileSystem& fs,
  const IO::Path& path,
  const GetPalette& getPalette,
  const Function& fun)
{
  auto file = fs.openFile(path);
  ensure(file != nullptr, "file is null");

  const auto modelName = path.lastComponent().asString();
  auto reader = file->reader().buffer();

  if (IO::MdlParser::canParse(path, reader))
  {
    const auto palette = getPalette();
    auto parser = IO::MdlParser{modelName, reader, palette};
    return fun(parser);
  }
  if (IO::Md2Parser::canParse(path, reader))
  {
    const auto palette = getPalette();
    auto parser = IO::Md2Parser{modelName, reader, palette, fs};
    return fun(parser);
  }
  if (IO::Md3Parser::canParse(path, reader))
  {
    auto parser = IO::Md3Parser{modelName, reader, fs};
    return fun(parser);
  }
  if (IO::MdxParser::canParse(path, reader))
  {
    auto parser = IO::MdxParser{modelName, reader, fs};
    return fun(parser);
  }
  if (IO::Bsp29Parser::canParse(path, reader))
  {
    const auto palette = getPalette();
    auto parser = IO::Bsp29Parser{modelName, reader, palette, fs};
    return fun(parser);
  }
  if (IO::DkmParser::canParse(path, reader))
  {
    auto parser = IO::DkmParser{modelName, reader, fs};
    return fun(parser);
  }
  if (IO::AseParser::canParse(path))
  {
    auto parser = IO::AseParser{modelName, reader.stringView(), fs};
    return fun(parser);
  }
  if (IO::NvObjParser::canParse(path))
  {
    // has to be the whole path for implicit textures!
    auto parser = IO::NvObjParser{path, reader.stringView(), fs};
    return fun(parser);
  }
  if (IO::ImageSpriteParser::canParse(path))
  {
    auto parser = IO::ImageSpriteParser{modelName, file, fs};
    return fun(parser);
  }
  if (IO::SprParser::canParse(path, reader))
  {
    const auto palette = getPalette();
    auto parser = IO::SprParser{modelName, reader, palette};
    return fun(parser);
  }
  if (IO::AssimpParser::canParse(path))
  {
    auto parser = IO::AssimpParser{path, fs};
    return fun(parser);
  }
  throw GameException{"Unsupported model format '" + path.asString() + "'"};
}

std::unique_ptr<Assets::EntityModel> GameImpl::doInitializeModel(
  const IO::Path& path, Logger& logger) const
{
  try
  {
    return withEntityParser(
      m_fs,
      path,
      [&]() { return loadTexturePalette(); },
      [&](auto& parser) { return parser.initializeModel(logger); });
  }
  catch (const FileSystemException& e)
  {
    throw GameException{
      "Could not load model " + path.asString() + ": " + std::string{e.what()}};
  }
  catch (const AssetException& e)
  {
    throw GameException{
      "Could not load model " + path.asString() + ": " + std::string{e.what()}};
  }
  catch (const ParserException& e)
  {
    throw GameException{
      "Could not load model " + path.asString() + ": " + std::string{e.what()}};
  }
}

void GameImpl::doLoadFrame(
  const IO::Path& path,
  size_t frameIndex,
  Assets::EntityModel& model,
  Logger& logger) const
{
  try
  {
    ensure(model.frame(frameIndex) != nullptr, "invalid frame index");
    ensure(!model.frame(frameIndex)->loaded(), "frame already loaded");

    const auto file = m_fs.openFile(path);
    ensure(file != nullptr, "file is null");

    const auto modelName = path.lastComponent().asString();
    const auto extension = kdl::str_to_lower(path.extension());

    return withEntityParser(
      m_fs,
      path,
      [&]() { return loadTexturePalette(); },
      [&](auto& parser) { return parser.loadFrame(frameIndex, model, logger); });
  }
  catch (FileSystemException& e)
  {
    throw GameException{
      "Could not load model " + path.asString() + ": " + std::string{e.what()}};
  }
  catch (AssetException& e)
  {
    throw GameException{
      "Could not load model " + path.asString() + ": " + std::string{e.what()}};
  }
}

Assets::Palette GameImpl::loadTexturePalette() const
{
  return Assets::Palette::loadFile(m_fs, m_config.textureConfig.palette);
}

std::vector<std::string> GameImpl::doAvailableMods() const
{
  auto result = std::vector<std::string>{};
  if (m_gamePath.isEmpty() || !IO::Disk::directoryExists(m_gamePath))
  {
    return result;
  }

  const auto& defaultMod =
    m_config.fileSystemConfig.searchPath.lastComponent().asString();
  const auto fs = IO::DiskFileSystem{m_gamePath};
  const auto subDirs =
    fs.findItems(IO::Path{}, IO::FileTypeMatcher{!true(files), true(directories)});
  for (const auto& subDir : subDirs)
  {
    const auto mod = subDir.lastComponent().asString();
    if (!kdl::ci::str_is_equal(mod, defaultMod))
    {
      result.push_back(mod);
    }
  }
  return result;
}

std::vector<std::string> GameImpl::doExtractEnabledMods(const Entity& entity) const
{
  if (const auto* modStr = entity.property(EntityPropertyKeys::Mods))
  {
    return kdl::str_split(*modStr, ";");
  }
  return {};
}

std::string GameImpl::doDefaultMod() const
{
  return m_config.fileSystemConfig.searchPath.asString();
}

const FlagsConfig& GameImpl::doSurfaceFlags() const
{
  return m_config.faceAttribsConfig.surfaceFlags;
}

const FlagsConfig& GameImpl::doContentFlags() const
{
  return m_config.faceAttribsConfig.contentFlags;
}

const BrushFaceAttributes& GameImpl::doDefaultFaceAttribs() const
{
  return m_config.faceAttribsConfig.defaults;
}

const std::vector<CompilationTool>& GameImpl::doCompilationTools() const
{
  return m_config.compilationTools;
}

EntityPropertyConfig GameImpl::entityPropertyConfig() const
{
  return {
    m_config.entityConfig.scaleExpression, m_config.entityConfig.setDefaultProperties};
}

void GameImpl::writeLongAttribute(
  EntityNodeBase& node,
  const std::string& baseName,
  const std::string& value,
  const size_t maxLength) const
{
  auto entity = node.entity();
  entity.removeNumberedProperty(entityPropertyConfig(), baseName);

  auto nameStr = std::stringstream{};
  for (size_t i = 0; i <= value.size() / maxLength; ++i)
  {
    nameStr.str("");
    nameStr << baseName << i + 1;
    entity.addOrUpdateProperty(
      entityPropertyConfig(), nameStr.str(), value.substr(i * maxLength, maxLength));
  }

  node.setEntity(std::move(entity));
}

std::string GameImpl::readLongAttribute(
  const EntityNodeBase& node, const std::string& baseName) const
{
  size_t index = 1;
  auto nameStr = std::stringstream{};
  auto valueStr = std::stringstream{};
  nameStr << baseName << index;

  const auto& entity = node.entity();
  while (entity.hasProperty(nameStr.str()))
  {
    if (const auto* value = entity.property(nameStr.str()))
    {
      valueStr << *value;
    }
    nameStr.str("");
    nameStr << baseName << ++index;
  }

  return valueStr.str();
}
} // namespace Model
} // namespace TrenchBroom
