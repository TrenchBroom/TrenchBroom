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
#include "Assets/TextureManager.h"
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
#include "IO/GameConfigParser.h"
#include "IO/ImageSpriteParser.h"
#include "IO/LoadTextureCollection.h"
#include "IO/Md2Parser.h"
#include "IO/Md3Parser.h"
#include "IO/MdlParser.h"
#include "IO/MdxParser.h"
#include "IO/NodeReader.h"
#include "IO/NodeWriter.h"
#include "IO/ObjSerializer.h"
#include "IO/PathInfo.h"
#include "IO/SimpleParserStatus.h"
#include "IO/SprParser.h"
#include "IO/SystemPaths.h"
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
#include <kdl/path_utils.h>
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
GameImpl::GameImpl(GameConfig& config, std::filesystem::path gamePath, Logger& logger)
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

std::filesystem::path GameImpl::doGamePath() const
{
  return m_gamePath;
}

void GameImpl::doSetGamePath(const std::filesystem::path& gamePath, Logger& logger)
{
  if (gamePath != m_gamePath)
  {
    m_gamePath = gamePath;
    initializeFileSystem(logger);
  }
}

void GameImpl::doSetAdditionalSearchPaths(
  const std::vector<std::filesystem::path>& searchPaths, Logger& logger)
{
  if (searchPaths != m_additionalSearchPaths)
  {
    m_additionalSearchPaths = searchPaths;
    initializeFileSystem(logger);
  }
}

Game::PathErrors GameImpl::doCheckAdditionalSearchPaths(
  const std::vector<std::filesystem::path>& searchPaths) const
{
  auto result = PathErrors{};
  for (const auto& searchPath : searchPaths)
  {
    const auto absPath = m_gamePath / searchPath;
    if (!absPath.is_absolute() || IO::Disk::pathInfo(absPath) != IO::PathInfo::Directory)
    {
      result.emplace(searchPath, "Directory not found: '" + searchPath.string() + "'");
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
  if (
    !initialMapFilePath.empty()
    && IO::Disk::pathInfo(initialMapFilePath) == IO::PathInfo::File)
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
    .transform([&](Brush&& b) {
      worldNode->defaultLayer()->addChild(new BrushNode{std::move(b)});
    })
    .transform_error([&](const Model::BrushError e) {
      logger.error() << "Could not create default brush: " << e;
    });

  return worldNode;
}

std::unique_ptr<WorldNode> GameImpl::doLoadMap(
  const MapFormat format,
  const vm::bbox3& worldBounds,
  const std::filesystem::path& path,
  Logger& logger) const
{
  auto parserStatus = IO::SimpleParserStatus{logger};
  auto file = IO::Disk::openFile(path);
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
  WorldNode& world, const std::filesystem::path& path, const bool exporting) const
{
  IO::Disk::withOutputStream(path, [&](auto& stream) {
    const auto mapFormatName = formatName(world.mapFormat());
    stream << "// Game: " << gameName() << "\n"
           << "// Format: " << mapFormatName << "\n";

    auto writer = IO::NodeWriter{world, stream};
    writer.setExporting(exporting);
    writer.writeMap();
  }).if_error([](const auto& e) { throw FileSystemException{e.msg.c_str()}; });
}

void GameImpl::doWriteMap(WorldNode& world, const std::filesystem::path& path) const
{
  doWriteMap(world, path, false);
}

void GameImpl::doExportMap(WorldNode& world, const IO::ExportOptions& options) const
{
  std::visit(
    kdl::overload(
      [&](const IO::ObjExportOptions& objOptions) {
        IO::Disk::withOutputStream(objOptions.exportPath, [&](auto& objStream) {
          const auto mtlPath = kdl::path_replace_extension(objOptions.exportPath, ".mtl");
          return IO::Disk::withOutputStream(mtlPath, [&](auto& mtlStream) {
            auto writer = IO::NodeWriter{
              world,
              std::make_unique<IO::ObjSerializer>(
                objStream, mtlStream, mtlPath.filename().string(), objOptions)};
            writer.setExporting(true);
            writer.writeMap();
          });
        }).if_error([](const auto& e) { throw FileSystemException{e.msg.c_str()}; });
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

void GameImpl::doLoadTextureCollections(Assets::TextureManager& textureManager) const
{
  textureManager.reload(m_fs, m_config.textureConfig);
}

void GameImpl::doReloadWads(
  const std::filesystem::path& documentPath,
  const std::vector<std::filesystem::path>& wadPaths,
  Logger& logger)
{
  const auto searchPaths = std::vector<std::filesystem::path>{
    documentPath.parent_path(), // Search for assets relative to the map file.
    m_gamePath,                 // Search for assets relative to the location of the game.
    IO::SystemPaths::appDirectory(), // Search for assets relative to the application.
  };
  m_fs.reloadWads(m_config.textureConfig.root, searchPaths, wadPaths, logger);
}

void GameImpl::doReloadShaders()
{
  m_fs.reloadShaders();
}

bool GameImpl::doIsEntityDefinitionFile(const std::filesystem::path& path) const
{
  static const auto extensions = {".fgd", ".def", ".ent"};

  return std::any_of(extensions.begin(), extensions.end(), [&](const auto& extension) {
    return kdl::ci::str_is_equal(extension, path.extension().string());
  });
}

std::vector<Assets::EntityDefinition*> GameImpl::doLoadEntityDefinitions(
  IO::ParserStatus& status, const std::filesystem::path& path) const
{
  const auto extension = path.extension().string();
  const auto& defaultColor = m_config.entityConfig.defaultColor;

  if (kdl::ci::str_is_equal(".fgd", extension))
  {
    auto file = IO::Disk::openFile(path);
    auto reader = file->reader().buffer();
    auto parser = IO::FgdParser{reader.stringView(), defaultColor, file->path()};
    return parser.parseDefinitions(status);
  }
  if (kdl::ci::str_is_equal(".def", extension))
  {
    auto file = IO::Disk::openFile(path);
    auto reader = file->reader().buffer();
    auto parser = IO::DefParser{reader.stringView(), defaultColor};
    return parser.parseDefinitions(status);
  }
  if (kdl::ci::str_is_equal(".ent", extension))
  {
    auto file = IO::Disk::openFile(path);
    auto reader = file->reader().buffer();
    auto parser = IO::EntParser{reader.stringView(), defaultColor};
    return parser.parseDefinitions(status);
  }

  throw GameException{"Unknown entity definition format: '" + path.string() + "'"};
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

std::filesystem::path GameImpl::doFindEntityDefinitionFile(
  const Assets::EntityDefinitionFileSpec& spec,
  const std::vector<std::filesystem::path>& searchPaths) const
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

  if (path.is_absolute())
  {
    return path;
  }

  return IO::Disk::resolvePath(searchPaths, path);
}

template <typename GetPalette, typename Function>
static auto withEntityParser(
  const GameFileSystem& fs,
  const std::filesystem::path& path,
  const GetPalette& getPalette,
  const Function& fun)
{
  auto file = fs.openFile(path);
  ensure(file != nullptr, "file is null");

  const auto modelName = path.filename().string();
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
  throw GameException{"Unsupported model format '" + path.string() + "'"};
}

std::unique_ptr<Assets::EntityModel> GameImpl::doInitializeModel(
  const std::filesystem::path& path, Logger& logger) const
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
      "Could not load model " + path.string() + ": " + std::string{e.what()}};
  }
  catch (const AssetException& e)
  {
    throw GameException{
      "Could not load model " + path.string() + ": " + std::string{e.what()}};
  }
  catch (const ParserException& e)
  {
    throw GameException{
      "Could not load model " + path.string() + ": " + std::string{e.what()}};
  }
}

void GameImpl::doLoadFrame(
  const std::filesystem::path& path,
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

    return withEntityParser(
      m_fs,
      path,
      [&]() { return loadTexturePalette(); },
      [&](auto& parser) { return parser.loadFrame(frameIndex, model, logger); });
  }
  catch (FileSystemException& e)
  {
    throw GameException{
      "Could not load model " + path.string() + ": " + std::string{e.what()}};
  }
  catch (AssetException& e)
  {
    throw GameException{
      "Could not load model " + path.string() + ": " + std::string{e.what()}};
  }
}

Assets::Palette GameImpl::loadTexturePalette() const
{
  auto file = m_fs.openFile(m_config.textureConfig.palette);
  return Assets::loadPalette(*file)
    .if_error([](const auto& e) { throw AssetException{e.msg.c_str()}; })
    .value();
}

std::vector<std::string> GameImpl::doAvailableMods() const
{
  auto result = std::vector<std::string>{};
  if (m_gamePath.empty() || IO::Disk::pathInfo(m_gamePath) != IO::PathInfo::Directory)
  {
    return result;
  }

  const auto& defaultMod = m_config.fileSystemConfig.searchPath.filename().string();
  const auto fs = IO::DiskFileSystem{m_gamePath};
  const auto subDirs = fs.find(
    std::filesystem::path{}, IO::makePathInfoPathMatcher({IO::PathInfo::Directory}));
  for (const auto& subDir : subDirs)
  {
    const auto mod = subDir.filename().string();
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
  return m_config.fileSystemConfig.searchPath.string();
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
