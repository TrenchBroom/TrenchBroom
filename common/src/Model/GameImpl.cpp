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

#include "Assets/EntityDefinition.h"
#include "Assets/EntityDefinitionFileSpec.h"
#include "Assets/EntityModel.h"
#include "Assets/MaterialManager.h"
#include "Assets/Palette.h"
#include "Ensure.h"
#include "Error.h"
#include "Exceptions.h"
#include "IO/BrushFaceReader.h"
#include "IO/DefParser.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/EntParser.h"
#include "IO/ExportOptions.h"
#include "IO/FgdParser.h"
#include "IO/File.h"
#include "IO/GameConfigParser.h"
#include "IO/LoadEntityModel.h"
#include "IO/LoadMaterialCollection.h"
#include "IO/NodeReader.h"
#include "IO/NodeWriter.h"
#include "IO/ObjSerializer.h"
#include "IO/PathInfo.h"
#include "IO/SimpleParserStatus.h"
#include "IO/SystemPaths.h"
#include "IO/TraversalMode.h"
#include "IO/WorldReader.h"
#include "Logger.h"
#include "Macros.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityProperties.h"
#include "Model/GameConfig.h"
#include "Model/LayerNode.h"
#include "Model/WorldNode.h"

#include "kdl/overload.h"
#include "kdl/path_utils.h"
#include "kdl/result.h"
#include "kdl/string_compare.h"
#include "kdl/string_format.h"
#include "kdl/string_utils.h"
#include "kdl/vector_utils.h"

#include "vm/vec_io.h"

#include <fstream>
#include <string>
#include <vector>

namespace TrenchBroom::Model
{
GameImpl::GameImpl(GameConfig& config, std::filesystem::path gamePath, Logger& logger)
  : m_config{config}
  , m_gamePath{std::move(gamePath)}
{
  initializeFileSystem(logger);
}

Result<std::vector<std::unique_ptr<Assets::EntityDefinition>>> GameImpl::
  loadEntityDefinitions(IO::ParserStatus& status, const std::filesystem::path& path) const
{
  const auto extension = path.extension().string();
  const auto& defaultColor = m_config.entityConfig.defaultColor;

  try
  {
    if (kdl::ci::str_is_equal(".fgd", extension))
    {
      return IO::Disk::openFile(path) | kdl::transform([&](auto file) {
               auto reader = file->reader().buffer();
               auto parser = IO::FgdParser{reader.stringView(), defaultColor, path};
               return parser.parseDefinitions(status);
             });
    }
    if (kdl::ci::str_is_equal(".def", extension))
    {
      return IO::Disk::openFile(path) | kdl::transform([&](auto file) {
               auto reader = file->reader().buffer();
               auto parser = IO::DefParser{reader.stringView(), defaultColor};
               return parser.parseDefinitions(status);
             });
    }
    if (kdl::ci::str_is_equal(".ent", extension))
    {
      return IO::Disk::openFile(path) | kdl::transform([&](auto file) {
               auto reader = file->reader().buffer();
               auto parser = IO::EntParser{reader.stringView(), defaultColor};
               return parser.parseDefinitions(status);
             });
    }

    return Error{"Unknown entity definition format: '" + path.string() + "'"};
  }
  catch (const ParserException& e)
  {
    return Error{e.what()};
  }
}

std::unique_ptr<Assets::EntityModel> GameImpl::initializeModel(
  const std::filesystem::path& path, Logger& logger) const
{
  return IO::initializeEntityModel(m_fs, m_config.materialConfig, path, logger);
}

void GameImpl::loadFrame(
  const std::filesystem::path& path,
  size_t frameIndex,
  Assets::EntityModel& model,
  Logger& logger) const
{
  return IO::loadEntityModelFrame(
    m_fs, m_config.materialConfig, path, frameIndex, model, logger);
}

const GameConfig& GameImpl::config() const
{
  return m_config;
}

const IO::FileSystem& GameImpl::gameFileSystem() const
{
  return m_fs;
}

std::filesystem::path GameImpl::gamePath() const
{
  return m_gamePath;
}

void GameImpl::setGamePath(const std::filesystem::path& gamePath, Logger& logger)
{
  if (gamePath != m_gamePath)
  {
    m_gamePath = gamePath;
    initializeFileSystem(logger);
  }
}

void GameImpl::setAdditionalSearchPaths(
  const std::vector<std::filesystem::path>& searchPaths, Logger& logger)
{
  if (searchPaths != m_additionalSearchPaths)
  {
    m_additionalSearchPaths = searchPaths;
    initializeFileSystem(logger);
  }
}

Game::PathErrors GameImpl::checkAdditionalSearchPaths(
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

Game::SoftMapBounds GameImpl::extractSoftMapBounds(const Entity& entity) const
{
  if (const auto* mapValue = entity.property(EntityPropertyKeys::SoftMapBounds))
  {
    return *mapValue == EntityPropertyValues::NoSoftMapBounds
             ? SoftMapBounds{SoftMapBoundsType::Map, std::nullopt}
             : SoftMapBounds{
               SoftMapBoundsType::Map, IO::parseSoftMapBoundsString(*mapValue)};
  }

  // Not set in map -> use Game value
  return SoftMapBounds{SoftMapBoundsType::Game, config().softMapBounds};
}

Result<std::unique_ptr<WorldNode>> GameImpl::newMap(
  const MapFormat format, const vm::bbox3& worldBounds, Logger& logger) const
{
  const auto initialMapFilePath = m_config.findInitialMap(formatName(format));
  if (
    !initialMapFilePath.empty()
    && IO::Disk::pathInfo(initialMapFilePath) == IO::PathInfo::File)
  {
    return loadMap(format, worldBounds, initialMapFilePath, logger);
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

  if (m_config.materialConfig.property)
  {
    worldEntity.addOrUpdateProperty(
      propertyConfig, *m_config.materialConfig.property, "");
  }

  auto worldNode = std::make_unique<WorldNode>(
    std::move(propertyConfig), std::move(worldEntity), format);

  const auto builder = Model::BrushBuilder{
    worldNode->mapFormat(), worldBounds, config().faceAttribsConfig.defaults};
  builder.createCuboid({128.0, 128.0, 32.0}, Model::BrushFaceAttributes::NoMaterialName)
    | kdl::transform(
      [&](auto b) { worldNode->defaultLayer()->addChild(new BrushNode{std::move(b)}); })
    | kdl::transform_error(
      [&](auto e) { logger.error() << "Could not create default brush: " << e.msg; });

  return worldNode;
}

Result<std::unique_ptr<WorldNode>> GameImpl::loadMap(
  const MapFormat format,
  const vm::bbox3& worldBounds,
  const std::filesystem::path& path,
  Logger& logger) const
{
  auto parserStatus = IO::SimpleParserStatus{logger};
  return IO::Disk::openFile(path) | kdl::transform([&](auto file) {
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

           auto worldReader =
             IO::WorldReader{fileReader.stringView(), format, entityPropertyConfig()};
           return worldReader.read(worldBounds, parserStatus);
         });
}

Result<void> GameImpl::writeMap(
  WorldNode& world, const std::filesystem::path& path, const bool exporting) const
{
  return IO::Disk::withOutputStream(path, [&](auto& stream) {
    const auto mapFormatName = formatName(world.mapFormat());
    stream << "// Game: " << config().name << "\n"
           << "// Format: " << mapFormatName << "\n";

    auto writer = IO::NodeWriter{world, stream};
    writer.setExporting(exporting);
    writer.writeMap();
  });
}

Result<void> GameImpl::writeMap(WorldNode& world, const std::filesystem::path& path) const
{
  return writeMap(world, path, false);
}

Result<void> GameImpl::exportMap(WorldNode& world, const IO::ExportOptions& options) const
{
  return std::visit(
    kdl::overload(
      [&](const IO::ObjExportOptions& objOptions) {
        return IO::Disk::withOutputStream(objOptions.exportPath, [&](auto& objStream) {
          const auto mtlPath = kdl::path_replace_extension(objOptions.exportPath, ".mtl");
          return IO::Disk::withOutputStream(mtlPath, [&](auto& mtlStream) {
            auto writer = IO::NodeWriter{
              world,
              std::make_unique<IO::ObjSerializer>(
                objStream, mtlStream, mtlPath.filename().string(), objOptions)};
            writer.setExporting(true);
            writer.writeMap();
          });
        });
      },
      [&](const IO::MapExportOptions& mapOptions) {
        return writeMap(world, mapOptions.exportPath, true);
      }),
    options);
}

std::vector<Node*> GameImpl::parseNodes(
  const std::string& str,
  const MapFormat mapFormat,
  const vm::bbox3& worldBounds,
  Logger& logger) const
{
  auto parserStatus = IO::SimpleParserStatus{logger};
  return IO::NodeReader::read(
    str, mapFormat, worldBounds, entityPropertyConfig(), parserStatus);
}

std::vector<BrushFace> GameImpl::parseBrushFaces(
  const std::string& str,
  const MapFormat mapFormat,
  const vm::bbox3& worldBounds,
  Logger& logger) const
{
  auto parserStatus = IO::SimpleParserStatus{logger};
  auto reader = IO::BrushFaceReader{str, mapFormat};
  return reader.read(worldBounds, parserStatus);
}

void GameImpl::writeNodesToStream(
  WorldNode& world, const std::vector<Node*>& nodes, std::ostream& stream) const
{
  auto writer = IO::NodeWriter{world, stream};
  writer.writeNodes(nodes);
}

void GameImpl::writeBrushFacesToStream(
  WorldNode& world, const std::vector<BrushFace>& faces, std::ostream& stream) const
{
  auto writer = IO::NodeWriter{world, stream};
  writer.writeBrushFaces(faces);
}

void GameImpl::loadMaterialCollections(
  Assets::MaterialManager& materialManager,
  const Assets::CreateTextureResource& createResource) const
{
  materialManager.reload(m_fs, m_config.materialConfig, createResource);
}

void GameImpl::reloadWads(
  const std::filesystem::path& documentPath,
  const std::vector<std::filesystem::path>& wadPaths,
  Logger& logger)
{
  const auto searchPaths = std::vector<std::filesystem::path>{
    documentPath.parent_path(), // Search for assets relative to the map file.
    m_gamePath,                 // Search for assets relative to the location of the game.
    IO::SystemPaths::appDirectory(), // Search for assets relative to the application.
  };
  m_fs.reloadWads(m_config.materialConfig.root, searchPaths, wadPaths, logger);
}

bool GameImpl::isEntityDefinitionFile(const std::filesystem::path& path) const
{
  static const auto extensions = {".fgd", ".def", ".ent"};

  return std::any_of(extensions.begin(), extensions.end(), [&](const auto& extension) {
    return kdl::ci::str_is_equal(extension, path.extension().string());
  });
}

std::vector<Assets::EntityDefinitionFileSpec> GameImpl::allEntityDefinitionFiles() const
{
  return kdl::vec_transform(m_config.entityConfig.defFilePaths, [](const auto& path) {
    return Assets::EntityDefinitionFileSpec::builtin(path);
  });
}

Assets::EntityDefinitionFileSpec GameImpl::extractEntityDefinitionFile(
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

  throw GameException{
    "No entity definition files found for game '" + config().name + "'"};
}

std::filesystem::path GameImpl::findEntityDefinitionFile(
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

Result<std::vector<std::string>> GameImpl::availableMods() const
{
  if (m_gamePath.empty() || IO::Disk::pathInfo(m_gamePath) != IO::PathInfo::Directory)
  {
    return Result<std::vector<std::string>>{std::vector<std::string>{}};
  }

  const auto& defaultMod = m_config.fileSystemConfig.searchPath.filename().string();
  const auto fs = IO::DiskFileSystem{m_gamePath};
  return fs.find(
           "",
           IO::TraversalMode::Flat,
           IO::makePathInfoPathMatcher({IO::PathInfo::Directory}))
         | kdl::transform([](auto subDirs) {
             return kdl::vec_transform(std::move(subDirs), [](auto subDir) {
               return subDir.filename().string();
             });
           })
         | kdl::transform([&](auto mods) {
             return kdl::vec_filter(std::move(mods), [&](const auto& mod) {
               return !kdl::ci::str_is_equal(mod, defaultMod);
             });
           });
}

std::vector<std::string> GameImpl::extractEnabledMods(const Entity& entity) const
{
  if (const auto* modStr = entity.property(EntityPropertyKeys::Mods))
  {
    return kdl::str_split(*modStr, ";");
  }
  return {};
}

std::string GameImpl::defaultMod() const
{
  return m_config.fileSystemConfig.searchPath.string();
}

void GameImpl::initializeFileSystem(Logger& logger)
{
  m_fs.initialize(m_config, m_gamePath, m_additionalSearchPaths, logger);
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
} // namespace TrenchBroom::Model
