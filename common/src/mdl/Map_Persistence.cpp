/*
 Copyright (C) 2025 Kristian Duske

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

#include "Ensure.h"
#include "Logger.h"
#include "io/DiskIO.h"
#include "io/MapHeader.h"
#include "io/NodeWriter.h"
#include "io/ObjSerializer.h"
#include "io/PathInfo.h"
#include "io/SimpleParserStatus.h"
#include "io/WorldReader.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushNode.h"
#include "mdl/CommandProcessor.h"
#include "mdl/EditorContext.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/Game.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/MapFormat.h"
#include "mdl/WorldNode.h"

#include "kdl/path_utils.h"
#include "kdl/range_to_vector.h"

namespace tb::mdl
{
namespace
{

Result<std::unique_ptr<WorldNode>> loadMap(
  const GameConfig& config,
  const MapFormat mapFormat,
  const vm::bbox3d& worldBounds,
  const std::filesystem::path& path,
  kdl::task_manager& taskManager,
  Logger& logger)
{
  const auto entityPropertyConfig = EntityPropertyConfig{
    config.entityConfig.scaleExpression, config.entityConfig.setDefaultProperties};

  auto parserStatus = io::SimpleParserStatus{logger};
  return io::Disk::openFile(path) | kdl::and_then([&](auto file) {
           auto fileReader = file->reader().buffer();
           if (mapFormat == MapFormat::Unknown)
           {
             // Try all formats listed in the game config
             const auto possibleFormats =
               config.fileFormats | std::views::transform([](const auto& formatConfig) {
                 return formatFromName(formatConfig.format);
               })
               | kdl::to_vector;

             return io::WorldReader::tryRead(
               fileReader.stringView(),
               possibleFormats,
               worldBounds,
               entityPropertyConfig,
               parserStatus,
               taskManager);
           }

           auto worldReader =
             io::WorldReader{fileReader.stringView(), mapFormat, entityPropertyConfig};
           return worldReader.read(worldBounds, parserStatus, taskManager);
         });
}

Result<std::unique_ptr<WorldNode>> createMap(
  const GameConfig& config,
  const MapFormat format,
  const vm::bbox3d& worldBounds,
  kdl::task_manager& taskManager,
  Logger& logger)
{
  if (!config.forceEmptyNewMap)
  {
    const auto initialMapFilePath = config.findInitialMap(formatName(format));
    if (
      !initialMapFilePath.empty()
      && io::Disk::pathInfo(initialMapFilePath) == io::PathInfo::File)
    {
      return loadMap(
        config, format, worldBounds, initialMapFilePath, taskManager, logger);
    }
  }

  auto worldEntity = Entity{};
  if (!config.forceEmptyNewMap)
  {
    if (
      format == MapFormat::Valve || format == MapFormat::Quake2_Valve
      || format == MapFormat::Quake3_Valve)
    {
      worldEntity.addOrUpdateProperty(EntityPropertyKeys::ValveVersion, "220");
    }

    if (config.materialConfig.property)
    {
      worldEntity.addOrUpdateProperty(*config.materialConfig.property, "");
    }
  }

  auto entityPropertyConfig = EntityPropertyConfig{
    config.entityConfig.scaleExpression, config.entityConfig.setDefaultProperties};
  auto worldNode = std::make_unique<WorldNode>(
    std::move(entityPropertyConfig), std::move(worldEntity), format);

  if (!config.forceEmptyNewMap)
  {
    const auto builder = BrushBuilder{
      worldNode->mapFormat(), worldBounds, config.faceAttribsConfig.defaults};
    builder.createCuboid({128.0, 128.0, 32.0}, BrushFaceAttributes::NoMaterialName)
      | kdl::transform(
        [&](auto b) { worldNode->defaultLayer()->addChild(new BrushNode{std::move(b)}); })
      | kdl::transform_error(
        [&](auto e) { logger.error() << "Could not create default brush: " << e.msg; });
  }

  return worldNode;
}

void setWorldDefaultProperties(
  WorldNode& world, EntityDefinitionManager& entityDefinitionManager)
{
  const auto definition =
    entityDefinitionManager.definition(EntityPropertyValues::WorldspawnClassname);

  if (definition && world.entityPropertyConfig().setDefaultProperties)
  {
    auto entity = world.entity();
    setDefaultProperties(*definition, entity, SetDefaultPropertyMode::SetAll);
    world.setEntity(std::move(entity));
  }
}

} // namespace

Result<void> Map::create(
  const MapFormat mapFormat, const vm::bbox3d& worldBounds, std::unique_ptr<Game> game)
{
  m_logger.info() << "Creating new document";

  clear();

  return createMap(game->config(), mapFormat, m_worldBounds, m_taskManager, m_logger)
         | kdl::transform([&](auto worldNode) {
             setWorld(
               worldBounds, std::move(worldNode), std::move(game), DefaultDocumentName);
             setWorldDefaultProperties(*m_world, *m_entityDefinitionManager);
             clearModificationCount();
             mapWasCreatedNotifier(*this);
           });
}

Result<void> Map::load(
  const MapFormat mapFormat,
  const vm::bbox3d& worldBounds,
  std::unique_ptr<Game> game,
  const std::filesystem::path& path)
{
  m_logger.info() << fmt::format("Loading document from {}", path);

  clear();

  return loadMap(game->config(), mapFormat, worldBounds, path, m_taskManager, m_logger)
         | kdl::transform([&](auto worldNode) {
             setWorld(worldBounds, std::move(worldNode), std::move(game), path);
             mapWasLoadedNotifier(*this);
           });
}

Result<void> Map::reload()
{
  if (!persistent())
  {
    return Result<void>{Error{"Cannot reload transient document"}};
  }

  const auto mapFormat = m_world->mapFormat();
  const auto worldBounds = m_worldBounds;
  const auto path = m_path;
  auto game = std::move(m_game);

  clear();
  return load(mapFormat, worldBounds, std::move(game), path);
}

void Map::save()
{
  saveAs(m_path);
}

void Map::saveAs(const std::filesystem::path& path)
{
  saveTo(path);
  setLastSaveModificationCount();
  setPath(path);
  mapWasSavedNotifier(*this);
}

void Map::saveTo(const std::filesystem::path& path)
{
  ensure(m_game.get() != nullptr, "game is null");
  ensure(m_world, "world is null");

  io::Disk::withOutputStream(path, [&](auto& stream) {
    io::writeMapHeader(stream, m_game->config().name, m_world->mapFormat());

    auto writer = io::NodeWriter{*m_world, stream};
    writer.setExporting(false);
    writer.writeMap(m_taskManager);
  }) | kdl::transform_error([&](const auto& e) {
    m_logger.error() << "Could not save document: " << e.msg;
  });
}

Result<void> Map::exportAs(const io::ExportOptions& options) const
{
  return std::visit(
    kdl::overload(
      [&](const io::ObjExportOptions& objOptions) {
        return io::Disk::withOutputStream(objOptions.exportPath, [&](auto& objStream) {
          const auto mtlPath = kdl::path_replace_extension(objOptions.exportPath, ".mtl");
          return io::Disk::withOutputStream(mtlPath, [&](auto& mtlStream) {
            auto writer = io::NodeWriter{
              *m_world,
              std::make_unique<io::ObjSerializer>(
                objStream, mtlStream, mtlPath.filename().string(), objOptions)};
            writer.setExporting(true);
            writer.writeMap(m_taskManager);
          });
        });
      },
      [&](const io::MapExportOptions& mapOptions) {
        return io::Disk::withOutputStream(mapOptions.exportPath, [&](auto& stream) {
          auto writer = io::NodeWriter{*m_world, stream};
          writer.setExporting(true);
          writer.writeMap(m_taskManager);
        });
      }),
    options);
}

void Map::clear()
{
  clearRepeatableCommands();
  m_commandProcessor->clear();

  if (m_world)
  {
    mapWillBeClearedNotifier(*this);

    m_editorContext->reset();
    m_cachedSelection = std::nullopt;
    clearAssets();
    clearWorld();
    clearModificationCount();

    mapWasClearedNotifier(*this);
  }
}

bool Map::persistent() const
{
  return m_path.is_absolute() && io::Disk::pathInfo(m_path) == io::PathInfo::File;
}

std::string Map::filename() const
{
  return m_path.empty() ? "" : m_path.filename().string();
}

const std::filesystem::path& Map::path() const
{
  return m_path;
}

bool Map::modified() const
{
  return m_modificationCount != m_lastSaveModificationCount;
}

size_t Map::modificationCount() const
{
  return m_modificationCount;
}

void Map::incModificationCount(const size_t delta)
{
  m_modificationCount += delta;
  modificationStateDidChangeNotifier();
}

void Map::decModificationCount(const size_t delta)
{
  assert(m_modificationCount >= delta);
  m_modificationCount -= delta;
  modificationStateDidChangeNotifier();
}

void Map::setPath(const std::filesystem::path& path)
{
  m_path = path;
}

void Map::setLastSaveModificationCount()
{
  m_lastSaveModificationCount = m_modificationCount;
  modificationStateDidChangeNotifier();
}

void Map::clearModificationCount()
{
  m_lastSaveModificationCount = m_modificationCount = 0;
  modificationStateDidChangeNotifier();
}

} // namespace tb::mdl
