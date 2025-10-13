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

#include "Map.h"

#include "Ensure.h"
#include "Logger.h"
#include "PreferenceManager.h"
#include "io/DiskIO.h"
#include "io/GameConfigParser.h"
#include "io/LoadMaterialCollections.h"
#include "io/MapHeader.h"
#include "io/NodeReader.h"
#include "io/NodeWriter.h"
#include "io/ObjSerializer.h"
#include "io/PathInfo.h"
#include "io/SimpleParserStatus.h"
#include "io/WorldReader.h"
#include "mdl/AssetUtils.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/Command.h"
#include "mdl/CommandProcessor.h"
#include "mdl/EditorContext.h"
#include "mdl/EmptyBrushEntityValidator.h"
#include "mdl/EmptyGroupValidator.h"
#include "mdl/EmptyPropertyKeyValidator.h"
#include "mdl/EmptyPropertyValueValidator.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/EntityDefinitionUtils.h"
#include "mdl/EntityLinkManager.h"
#include "mdl/EntityModelManager.h"
#include "mdl/EntityNode.h"
#include "mdl/Game.h"
#include "mdl/GameFactory.h"
#include "mdl/Grid.h"
#include "mdl/GroupNode.h"
#include "mdl/InvalidUVScaleValidator.h"
#include "mdl/Issue.h"
#include "mdl/LayerNode.h"
#include "mdl/LinkSourceValidator.h"
#include "mdl/LinkTargetValidator.h"
#include "mdl/LinkedGroupUtils.h"
#include "mdl/LongPropertyKeyValidator.h"
#include "mdl/LongPropertyValueValidator.h"
#include "mdl/Map.h"
#include "mdl/MapFormat.h"
#include "mdl/MapTextEncoding.h"
#include "mdl/Map_Assets.h"
#include "mdl/Map_Entities.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/Map_World.h"
#include "mdl/MaterialManager.h"
#include "mdl/MissingClassnameValidator.h"
#include "mdl/MissingDefinitionValidator.h"
#include "mdl/MissingModValidator.h"
#include "mdl/MixedBrushContentsValidator.h"
#include "mdl/ModelUtils.h"
#include "mdl/Node.h"
#include "mdl/NodeIndex.h"
#include "mdl/NodeQueries.h"
#include "mdl/NonIntegerVerticesValidator.h"
#include "mdl/PatchNode.h"
#include "mdl/PointEntityWithBrushesValidator.h"
#include "mdl/PropertyKeyWithDoubleQuotationMarksValidator.h"
#include "mdl/PropertyValueWithDoubleQuotationMarksValidator.h"
#include "mdl/PushSelection.h"
#include "mdl/RepeatStack.h"
#include "mdl/ResourceManager.h"
#include "mdl/SoftMapBoundsValidator.h"
#include "mdl/TagManager.h"
#include "mdl/Transaction.h"
#include "mdl/UndoableCommand.h"
#include "mdl/UpdateLinkedGroupsCommand.h"
#include "mdl/UpdateLinkedGroupsHelper.h"
#include "mdl/VertexHandleManager.h"
#include "mdl/WorldBoundsValidator.h"
#include "mdl/WorldNode.h"
#include "mdl/WorldNode.h" // IWYU pragma: keep

#include "kdl/path_utils.h"
#include "kdl/ranges/to.h"
#include "kdl/string_utils.h"
#include "kdl/task_manager.h"

#include <fmt/format.h>
#include <fmt/std.h>

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <memory>
#include <ranges>
#include <string>
#include <vector>


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
               | kdl::ranges::to<std::vector>();

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

auto makeInitializeNodeTagsVisitor(TagManager& tagManager)
{
  return kdl::overload(
    [&](auto&& thisLambda, WorldNode* world) {
      world->initializeTags(tagManager);
      world->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, LayerNode* layer) {
      layer->initializeTags(tagManager);
      layer->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, GroupNode* group) {
      group->initializeTags(tagManager);
      group->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, EntityNode* entity) {
      entity->initializeTags(tagManager);
      entity->visitChildren(thisLambda);
    },
    [&](BrushNode* brush) { brush->initializeTags(tagManager); },
    [&](PatchNode* patch) { patch->initializeTags(tagManager); });
}

auto makeClearNodeTagsVisitor()
{
  return kdl::overload(
    [](auto&& thisLambda, WorldNode* world) {
      world->clearTags();
      world->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, LayerNode* layer) {
      layer->clearTags();
      layer->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, GroupNode* group) {
      group->clearTags();
      group->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, EntityNode* entity) {
      entity->clearTags();
      entity->visitChildren(thisLambda);
    },
    [](BrushNode* brush) { brush->clearTags(); },
    [](PatchNode* patch) { patch->clearTags(); });
}

auto makeSetMaterialsVisitor(MaterialManager& manager)
{
  return kdl::overload(
    [](auto&& thisLambda, WorldNode* worldNode) { worldNode->visitChildren(thisLambda); },
    [](auto&& thisLambda, LayerNode* layerNode) { layerNode->visitChildren(thisLambda); },
    [](auto&& thisLambda, GroupNode* groupNode) { groupNode->visitChildren(thisLambda); },
    [](auto&& thisLambda, EntityNode* entityNode) {
      entityNode->visitChildren(thisLambda);
    },
    [&](BrushNode* brushNode) {
      const auto& brush = brushNode->brush();
      for (size_t i = 0u; i < brush.faceCount(); ++i)
      {
        const auto& face = brush.face(i);
        auto* material = manager.material(face.attributes().materialName());
        brushNode->setFaceMaterial(i, material);
      }
    },
    [&](PatchNode* patchNode) {
      auto* material = manager.material(patchNode->patch().materialName());
      patchNode->setMaterial(material);
    });
}

auto makeUnsetMaterialsVisitor()
{
  return kdl::overload(
    [](auto&& thisLambda, WorldNode* worldNode) { worldNode->visitChildren(thisLambda); },
    [](auto&& thisLambda, LayerNode* layerNode) { layerNode->visitChildren(thisLambda); },
    [](auto&& thisLambda, GroupNode* groupNode) { groupNode->visitChildren(thisLambda); },
    [](auto&& thisLambda, EntityNode* entityNode) {
      entityNode->visitChildren(thisLambda);
    },
    [](BrushNode* brushNode) {
      const auto& brush = brushNode->brush();
      for (size_t i = 0u; i < brush.faceCount(); ++i)
      {
        brushNode->setFaceMaterial(i, nullptr);
      }
    },
    [](PatchNode* patchNode) { patchNode->setMaterial(nullptr); });
}

auto makeSetEntityDefinitionsVisitor(EntityDefinitionManager& manager)
{
  // this helper lambda must be captured by value
  const auto setEntityDefinition = [&](auto* node) {
    const auto* definition = manager.definition(node);
    node->setDefinition(definition);
  };

  return kdl::overload(
    [=](auto&& thisLambda, WorldNode* worldNode) {
      setEntityDefinition(worldNode);
      worldNode->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, LayerNode* layerNode) { layerNode->visitChildren(thisLambda); },
    [](auto&& thisLambda, GroupNode* groupNode) { groupNode->visitChildren(thisLambda); },
    [=](EntityNode* entityNode) { setEntityDefinition(entityNode); },
    [](BrushNode*) {},
    [](PatchNode*) {});
}

auto makeUnsetEntityDefinitionsVisitor()
{
  return kdl::overload(
    [](auto&& thisLambda, WorldNode* worldNode) {
      worldNode->setDefinition(nullptr);
      worldNode->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, LayerNode* layerNode) { layerNode->visitChildren(thisLambda); },
    [](auto&& thisLambda, GroupNode* groupNode) { groupNode->visitChildren(thisLambda); },
    [](EntityNode* entityNode) { entityNode->setDefinition(nullptr); },
    [](BrushNode*) {},
    [](PatchNode*) {});
}

auto makeSetEntityModelsVisitor(EntityModelManager& manager, Logger& logger)
{
  return kdl::overload(
    [](auto&& thisLambda, WorldNode* world) { world->visitChildren(thisLambda); },
    [](auto&& thisLambda, LayerNode* layer) { layer->visitChildren(thisLambda); },
    [](auto&& thisLambda, GroupNode* group) { group->visitChildren(thisLambda); },
    [&](EntityNode* entityNode) {
      const auto modelSpec =
        safeGetModelSpecification(logger, entityNode->entity().classname(), [&]() {
          return entityNode->entity().modelSpecification();
        });
      const auto* model = manager.model(modelSpec.path);
      entityNode->setModel(model);
    },
    [](BrushNode*) {},
    [](PatchNode*) {});
}

auto makeUnsetEntityModelsVisitor()
{
  return kdl::overload(
    [](auto&& thisLambda, WorldNode* world) { world->visitChildren(thisLambda); },
    [](auto&& thisLambda, LayerNode* layer) { layer->visitChildren(thisLambda); },
    [](auto&& thisLambda, GroupNode* group) { group->visitChildren(thisLambda); },
    [](EntityNode* entity) { entity->setModel(nullptr); },
    [](BrushNode*) {},
    [](PatchNode*) {});
}

std::vector<GroupNode*> collectGroupsWithPendingChanges(Node& node)
{
  auto result = std::vector<GroupNode*>{};

  node.accept(kdl::overload(
    [](auto&& thisLambda, const WorldNode* worldNode) {
      worldNode->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, const LayerNode* layerNode) {
      layerNode->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, GroupNode* groupNode) {
      if (groupNode->hasPendingChanges())
      {
        result.push_back(groupNode);
      }
      groupNode->visitChildren(thisLambda);
    },
    [](const EntityNode*) {},
    [](const BrushNode*) {},
    [](const PatchNode*) {}));

  return result;
}

bool updateLinkedGroups(Map& map)
{
  if (map.isCurrentDocumentStateObservable())
  {
    if (const auto allChangedLinkedGroups = collectGroupsWithPendingChanges(*map.world());
        !allChangedLinkedGroups.empty())
    {
      setHasPendingChanges(allChangedLinkedGroups, false);

      auto command = std::make_unique<UpdateLinkedGroupsCommand>(allChangedLinkedGroups);
      const auto result = map.executeAndStore(std::move(command));
      return result->success();
    }
  }

  return true;
}

class ThrowExceptionCommand : public UndoableCommand
{
public:
  ThrowExceptionCommand()
    : UndoableCommand{"Throw Exception", false}
  {
  }

private:
  std::unique_ptr<CommandResult> doPerformDo(Map&) override
  {
    throw CommandProcessorException{};
  }

  std::unique_ptr<CommandResult> doPerformUndo(Map&) override
  {
    return std::make_unique<CommandResult>(true);
  }
};

} // namespace

const vm::bbox3d Map::DefaultWorldBounds(-32768.0, 32768.0);
const std::string Map::DefaultDocumentName("unnamed.map");

Map::Map(kdl::task_manager& taskManager, Logger& logger)
  : m_logger{logger}
  , m_taskManager{taskManager}
  , m_resourceManager{std::make_unique<ResourceManager>()}
  , m_entityDefinitionManager{std::make_unique<EntityDefinitionManager>()}
  , m_entityModelManager{std::make_unique<EntityModelManager>(
      [&](auto resourceLoader) {
        auto resource =
          std::make_shared<EntityModelDataResource>(std::move(resourceLoader));
        m_resourceManager->addResource(resource);
        return resource;
      },
      m_logger)}
  , m_materialManager{std::make_unique<MaterialManager>(m_logger)}
  , m_tagManager{std::make_unique<TagManager>()}
  , m_editorContext{std::make_unique<EditorContext>()}
  , m_grid{std::make_unique<Grid>(4)}
  , m_worldBounds{DefaultWorldBounds}
  , m_nodeIndex{std::make_unique<NodeIndex>()}
  , m_entityLinkManager{std::make_unique<EntityLinkManager>(*m_nodeIndex)}
  , m_vertexHandles{std::make_unique<VertexHandleManager>()}
  , m_edgeHandles{std::make_unique<EdgeHandleManager>()}
  , m_faceHandles{std::make_unique<FaceHandleManager>()}
  , m_currentMaterialName{BrushFaceAttributes::NoMaterialName}
  , m_repeatStack{std::make_unique<RepeatStack>()}
  , m_commandProcessor{std::make_unique<CommandProcessor>(*this)}
{
  connectObservers();
}

Map::~Map()
{
  clearWorld();
}

Logger& Map::logger()
{
  return m_logger;
}

kdl::task_manager& Map::taskManager()
{
  return m_taskManager;
}

EntityDefinitionManager& Map::entityDefinitionManager()
{
  return *m_entityDefinitionManager;
}

const EntityDefinitionManager& Map::entityDefinitionManager() const
{
  return *m_entityDefinitionManager;
}

EntityModelManager& Map::entityModelManager()
{
  return *m_entityModelManager;
}

const EntityModelManager& Map::entityModelManager() const
{
  return *m_entityModelManager;
}

MaterialManager& Map::materialManager()
{
  return *m_materialManager;
}

const MaterialManager& Map::materialManager() const
{
  return *m_materialManager;
}

TagManager& Map::tagManager()
{
  return *m_tagManager;
}

const TagManager& Map::tagManager() const
{
  return *m_tagManager;
}

EditorContext& Map::editorContext()
{
  return *m_editorContext;
}

const EditorContext& Map::editorContext() const
{
  return *m_editorContext;
}

Grid& Map::grid()
{
  return *m_grid;
}

const Grid& Map::grid() const
{
  return *m_grid;
}

const Game* Map::game() const
{
  return m_game.get();
}

const vm::bbox3d& Map::worldBounds() const
{
  return m_worldBounds;
}

WorldNode* Map::world() const
{
  return m_world.get();
}

MapTextEncoding Map::encoding() const
{
  return MapTextEncoding::Quake;
}

VertexHandleManager& Map::vertexHandles()
{
  return *m_vertexHandles;
}

const VertexHandleManager& Map::vertexHandles() const
{
  return *m_vertexHandles;
}

EdgeHandleManager& Map::edgeHandles()
{
  return *m_edgeHandles;
}

const EdgeHandleManager& Map::edgeHandles() const
{
  return *m_edgeHandles;
}

FaceHandleManager& Map::faceHandles()
{
  return *m_faceHandles;
}

const FaceHandleManager& Map::faceHandles() const
{
  return *m_faceHandles;
}

const std::string& Map::currentMaterialName() const
{
  return m_currentMaterialName;
}

void Map::setCurrentMaterialName(const std::string& currentMaterialName)
{
  if (m_currentMaterialName != currentMaterialName)
  {
    m_currentMaterialName = currentMaterialName;
    currentMaterialNameDidChangeNotifier(m_currentMaterialName);
  }
}

const EntityLinkManager& Map::entityLinkManager() const
{
  return *m_entityLinkManager;
}

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
  if (!path.is_absolute())
  {
    return Error{"Path must be absolute"};
  }

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

Result<void> Map::save()
{
  return saveAs(m_path);
}

Result<void> Map::saveAs(const std::filesystem::path& path)
{
  return saveTo(path).transform([&]() {
    setLastSaveModificationCount();
    setPath(path);
    mapWasSavedNotifier(*this);
  });
}

Result<void> Map::saveTo(const std::filesystem::path& path)
{
  if (!path.is_absolute())
  {
    return Error{"Path must be absolute"};
  }

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

  return Result<void>{};
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

    m_nodeIndex->clear();
    m_entityLinkManager->clear();
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

void Map::setWorld(
  const vm::bbox3d& worldBounds,
  std::unique_ptr<WorldNode> worldNode,
  std::unique_ptr<Game> game,
  const std::filesystem::path& path)
{
  m_worldBounds = worldBounds;
  m_world = std::move(worldNode);
  m_game = std::move(game);

  entityModelManager().setGame(m_game.get(), taskManager());
  editorContext().setCurrentLayer(world()->defaultLayer());

  updateGameSearchPaths();
  setPath(path);

  loadAssets();
  registerValidators();
  registerSmartTags();
}

void Map::clearWorld()
{
  m_world.reset();
  editorContext().reset();
}

const Selection& Map::selection() const
{
  if (!m_cachedSelection)
  {
    m_cachedSelection = m_world ? computeSelection(*m_world.get()) : Selection{};
  }
  return *m_cachedSelection;
}

const vm::bbox3d Map::referenceBounds() const
{
  if (const auto bounds = selectionBounds())
  {
    return *bounds;
  }
  if (const auto bounds = lastSelectionBounds())
  {
    return *bounds;
  }
  return vm::bbox3d{16.0};
}

const std::optional<vm::bbox3d>& Map::lastSelectionBounds() const
{
  return m_lastSelectionBounds;
}

const std::optional<vm::bbox3d>& Map::selectionBounds() const
{
  if (!m_cachedSelectionBounds && selection().hasNodes())
  {
    m_cachedSelectionBounds = computeLogicalBounds(selection().nodes);
  }
  return m_cachedSelectionBounds;
}


void Map::registerSmartTags()
{
  ensure(game() != nullptr, "game is null");

  m_tagManager->clearSmartTags();
  m_tagManager->registerSmartTags(game()->config().smartTags);
}

const std::vector<SmartTag>& Map::smartTags() const
{
  return m_tagManager->smartTags();
}

bool Map::isRegisteredSmartTag(const std::string& name) const
{
  return m_tagManager->isRegisteredSmartTag(name);
}

const SmartTag& Map::smartTag(const std::string& name) const
{
  return m_tagManager->smartTag(name);
}

bool Map::isRegisteredSmartTag(const size_t index) const
{
  return m_tagManager->isRegisteredSmartTag(index);
}

const SmartTag& Map::smartTag(const size_t index) const
{
  return m_tagManager->smartTag(index);
}

void Map::initializeAllNodeTags()
{
  m_world->accept(makeInitializeNodeTagsVisitor(*m_tagManager));
}

void Map::initializeNodeTags(const std::vector<Node*>& nodes)
{
  Node::visitAll(nodes, makeInitializeNodeTagsVisitor(*m_tagManager));
}

void Map::clearNodeTags(const std::vector<Node*>& nodes)
{
  Node::visitAll(nodes, makeClearNodeTagsVisitor());
}

void Map::updateNodeTags(const std::vector<Node*>& nodes)
{
  for (auto* node : nodes)
  {
    node->updateTags(*m_tagManager);
  }
}

void Map::updateFaceTags(const std::vector<BrushFaceHandle>& faceHandles)
{
  for (const auto& faceHandle : faceHandles)
  {
    BrushNode* node = faceHandle.node();
    node->updateFaceTags(faceHandle.faceIndex(), *m_tagManager);
  }
}

void Map::updateAllFaceTags()
{
  m_world->accept(kdl::overload(
    [](auto&& thisLambda, WorldNode* world) { world->visitChildren(thisLambda); },
    [](auto&& thisLambda, LayerNode* layer) { layer->visitChildren(thisLambda); },
    [](auto&& thisLambda, GroupNode* group) { group->visitChildren(thisLambda); },
    [](auto&& thisLambda, EntityNode* entity) { entity->visitChildren(thisLambda); },
    [&](BrushNode* brush) { brush->initializeTags(*m_tagManager); },
    [](PatchNode*) {}));
}

void Map::updateFaceTagsAfterResourcesWhereProcessed(
  const std::vector<ResourceId>& resourceIds)
{
  // Some textures contain embedded default values for surface flags and such, so we must
  // update the face tags after the resources have been processed.

  const auto materials = m_materialManager->findMaterialsByTextureResourceId(resourceIds);
  const auto materialSet =
    std::unordered_set<const Material*>{materials.begin(), materials.end()};

  m_world->accept(kdl::overload(
    [](auto&& thisLambda, WorldNode* world) { world->visitChildren(thisLambda); },
    [](auto&& thisLambda, LayerNode* layer) { layer->visitChildren(thisLambda); },
    [](auto&& thisLambda, GroupNode* group) { group->visitChildren(thisLambda); },
    [](auto&& thisLambda, EntityNode* entity) { entity->visitChildren(thisLambda); },
    [&](BrushNode* brushNode) {
      const auto& faces = brushNode->brush().faces();
      for (size_t i = 0; i < faces.size(); ++i)
      {
        {
          const auto& face = faces[i];
          if (materialSet.contains(face.material()))
          {
            brushNode->updateFaceTags(i, *m_tagManager);
          }
        }
      }
    },
    [](PatchNode*) {}));
}

void Map::registerValidators()
{
  ensure(m_world, "world is null");
  ensure(m_game != nullptr, "game is null");

  m_world->registerValidator(std::make_unique<MissingClassnameValidator>());
  m_world->registerValidator(std::make_unique<MissingDefinitionValidator>());
  m_world->registerValidator(std::make_unique<MissingModValidator>(*m_game));
  m_world->registerValidator(std::make_unique<EmptyGroupValidator>());
  m_world->registerValidator(std::make_unique<EmptyBrushEntityValidator>());
  m_world->registerValidator(std::make_unique<PointEntityWithBrushesValidator>());
  m_world->registerValidator(std::make_unique<LinkSourceValidator>(*m_entityLinkManager));
  m_world->registerValidator(std::make_unique<LinkTargetValidator>(*m_entityLinkManager));
  m_world->registerValidator(std::make_unique<NonIntegerVerticesValidator>());
  m_world->registerValidator(std::make_unique<MixedBrushContentsValidator>());
  m_world->registerValidator(std::make_unique<WorldBoundsValidator>(worldBounds()));
  m_world->registerValidator(std::make_unique<SoftMapBoundsValidator>(*m_game, *m_world));
  m_world->registerValidator(std::make_unique<EmptyPropertyKeyValidator>());
  m_world->registerValidator(std::make_unique<EmptyPropertyValueValidator>());
  m_world->registerValidator(
    std::make_unique<LongPropertyKeyValidator>(m_game->config().maxPropertyLength));
  m_world->registerValidator(
    std::make_unique<LongPropertyValueValidator>(m_game->config().maxPropertyLength));
  m_world->registerValidator(
    std::make_unique<PropertyKeyWithDoubleQuotationMarksValidator>());
  m_world->registerValidator(
    std::make_unique<PropertyValueWithDoubleQuotationMarksValidator>());
  m_world->registerValidator(std::make_unique<InvalidUVScaleValidator>());
}

void Map::setIssueHidden(const Issue& issue, const bool hidden)
{
  if (issue.hidden() != hidden)
  {
    issue.node().setIssueHidden(issue.type(), hidden);
  }
}


void Map::loadAssets()
{
  loadEntityDefinitions();
  setEntityDefinitions();
  setEntityModels();
  loadMaterials();
  setMaterials();
}

void Map::clearAssets()
{
  clearEntityDefinitions();
  clearEntityModels();
  clearMaterials();
}

void Map::loadEntityDefinitions()
{
  if (const auto spec = entityDefinitionFile(*this))
  {
    auto status = io::SimpleParserStatus{m_logger};
    const auto path = game()->findEntityDefinitionFile(*spec, externalSearchPaths(*this));

    game()->loadEntityDefinitions(status, path)
      | kdl::transform([&](auto entityDefinitions) {
          m_logger.info() << fmt::format(
            "Loaded entity definition file {}", path.filename());

          addOrSetDefaultEntityLinkProperties(entityDefinitions);
          entityDefinitionManager().setDefinitions(std::move(entityDefinitions));
        })
      | kdl::transform_error([&](auto e) {
          switch (spec->type)
          {
          case EntityDefinitionFileSpec::Type::Builtin:
            m_logger.error() << "Could not load builtin entity definition file '"
                             << spec->path << "': " << e.msg;
            break;
          case EntityDefinitionFileSpec::Type::External:
            m_logger.error() << "Could not load external entity definition file '"
                             << spec->path << "': " << e.msg;
            break;
          }
        });
  }
  else
  {
    entityDefinitionManager().clear();
  }
}

void Map::clearEntityDefinitions()
{
  unsetEntityDefinitions();
  m_entityDefinitionManager->clear();
}

void Map::reloadMaterials()
{
  clearMaterials();
  loadMaterials();
}

void Map::loadMaterials()
{
  if (const auto* wadStr = m_world->entity().property(EntityPropertyKeys::Wad))
  {
    const auto wadPaths = kdl::str_split(*wadStr, ";")
                          | kdl::ranges::to<std::vector<std::filesystem::path>>();
    m_game->reloadWads(path(), wadPaths, m_logger);
  }
  m_materialManager->reload(
    m_game->gameFileSystem(),
    m_game->config().materialConfig,
    [&](auto resourceLoader) {
      auto resource = std::make_shared<TextureResource>(std::move(resourceLoader));
      m_resourceManager->addResource(resource);
      return resource;
    },
    m_taskManager);
}

void Map::clearMaterials()
{
  unsetMaterials();
  materialManager().clear();
}

void Map::setMaterials()
{
  m_world->accept(makeSetMaterialsVisitor(*m_materialManager));
  materialUsageCountsDidChangeNotifier();
}

void Map::setMaterials(const std::vector<Node*>& nodes)
{
  Node::visitAll(nodes, makeSetMaterialsVisitor(*m_materialManager));
  materialUsageCountsDidChangeNotifier();
}

void Map::setMaterials(const std::vector<BrushFaceHandle>& faceHandles)
{
  for (const auto& faceHandle : faceHandles)
  {
    BrushNode* node = faceHandle.node();
    const BrushFace& face = faceHandle.face();
    auto* material = m_materialManager->material(face.attributes().materialName());
    node->setFaceMaterial(faceHandle.faceIndex(), material);
  }
  materialUsageCountsDidChangeNotifier();
}

void Map::unsetMaterials()
{
  m_world->accept(makeUnsetMaterialsVisitor());
  materialUsageCountsDidChangeNotifier();
}

void Map::unsetMaterials(const std::vector<Node*>& nodes)
{
  Node::visitAll(nodes, makeUnsetMaterialsVisitor());
  materialUsageCountsDidChangeNotifier();
}

void Map::setEntityDefinitions()
{
  m_world->accept(makeSetEntityDefinitionsVisitor(*m_entityDefinitionManager));
}

void Map::setEntityDefinitions(const std::vector<Node*>& nodes)
{
  Node::visitAll(nodes, makeSetEntityDefinitionsVisitor(*m_entityDefinitionManager));
}

void Map::unsetEntityDefinitions()
{
  m_world->accept(makeUnsetEntityDefinitionsVisitor());
}

void Map::unsetEntityDefinitions(const std::vector<Node*>& nodes)
{
  Node::visitAll(nodes, makeUnsetEntityDefinitionsVisitor());
}

void Map::clearEntityModels()
{
  unsetEntityModels();
  m_entityModelManager->clear();
}

void Map::setEntityModels()
{
  m_world->accept(makeSetEntityModelsVisitor(*m_entityModelManager, m_logger));
}

void Map::setEntityModels(const std::vector<Node*>& nodes)
{
  Node::visitAll(nodes, makeSetEntityModelsVisitor(*m_entityModelManager, m_logger));
}

void Map::unsetEntityModels()
{
  m_world->accept(makeUnsetEntityModelsVisitor());
}

void Map::unsetEntityModels(const std::vector<Node*>& nodes)
{
  Node::visitAll(nodes, makeUnsetEntityModelsVisitor());
}

void Map::updateGameSearchPaths()
{
  m_game->setAdditionalSearchPaths(
    enabledMods(*this) | std::views::transform([](const auto& mod) {
      return std::filesystem::path{mod};
    }) | kdl::ranges::to<std::vector>(),
    m_logger);
}

void Map::initializeNodeIndex()
{
  ensure(m_world, "world node is set");
  addToNodeIndex({world()}, true);
}

void Map::addToNodeIndex(const std::vector<Node*>& nodes, const bool recurse)
{
  for (auto* node : nodes)
  {
    m_nodeIndex->addNode(*node);

    if (recurse)
    {
      addToNodeIndex(node->children(), true);
    }
  }
}

void Map::removeFromNodeIndex(const std::vector<Node*>& nodes, const bool recurse)
{
  for (auto* node : nodes)
  {
    m_nodeIndex->removeNode(*node);

    if (recurse)
    {
      removeFromNodeIndex(node->children(), true);
    }
  }
}

void Map::initializeEntityLinks()
{
  ensure(m_world, "world node is set");
  addEntityLinks({world()}, true);
}

void Map::addEntityLinks(const std::vector<Node*>& nodes, const bool recurse)
{
  for (auto* node : nodes)
  {
    node->accept(kdl::overload(
      [&](WorldNode* worldNode) { m_entityLinkManager->addEntityNode(*worldNode); },
      [](LayerNode*) {},
      [](GroupNode*) {},
      [&](EntityNode* entityNode) { m_entityLinkManager->addEntityNode(*entityNode); },
      [](BrushNode*) {},
      [](PatchNode*) {}));

    if (recurse)
    {
      addEntityLinks(node->children(), true);
    }
  }
}

void Map::removeEntityLinks(const std::vector<Node*>& nodes, const bool recurse)
{
  for (auto* node : nodes)
  {
    node->accept(kdl::overload(
      [&](WorldNode* worldNode) { m_entityLinkManager->removeEntityNode(*worldNode); },
      [](LayerNode*) {},
      [](GroupNode*) {},
      [&](EntityNode* entityNode) { m_entityLinkManager->removeEntityNode(*entityNode); },
      [](BrushNode*) {},
      [](PatchNode*) {}));


    if (recurse)
    {
      removeEntityLinks(node->children(), true);
    }
  }
}

void Map::processResourcesSync(const ProcessContext& processContext)
{
  auto allProcessedResourceIds = std::vector<ResourceId>{};
  while (m_resourceManager->needsProcessing())
  {
    auto processedResourceIds = m_resourceManager->process(
      [](auto task) {
        auto promise = std::promise<std::unique_ptr<TaskResult>>{};
        promise.set_value(task());
        return promise.get_future();
      },
      processContext);

    allProcessedResourceIds = kdl::vec_concat(
      std::move(allProcessedResourceIds), std::move(processedResourceIds));
  }

  if (!allProcessedResourceIds.empty())
  {
    resourcesWereProcessedNotifier.notify(
      kdl::vec_sort_and_remove_duplicates(std::move(allProcessedResourceIds)));
  }
}

void Map::processResourcesAsync(const ProcessContext& processContext)
{
  using namespace std::chrono_literals;

  const auto processedResourceIds = m_resourceManager->process(
    [&](auto task) { return m_taskManager.run_task(std::move(task)); },
    processContext,
    20ms);

  if (!processedResourceIds.empty())
  {
    resourcesWereProcessedNotifier.notify(processedResourceIds);
  }
}

bool Map::needsResourceProcessing() const
{
  return m_resourceManager->needsProcessing();
}

bool Map::canUndoCommand() const
{
  return m_commandProcessor->canUndo();
}

bool Map::canRedoCommand() const
{
  return m_commandProcessor->canRedo();
}

const std::string& Map::undoCommandName() const
{
  return m_commandProcessor->undoCommandName();
}

const std::string& Map::redoCommandName() const
{
  return m_commandProcessor->redoCommandName();
}

void Map::undoCommand()
{
  m_commandProcessor->undo();
  updateLinkedGroups(*this);

  // Undo/redo in the repeat system is not supported for now, so just clear the repeat
  // stack
  m_repeatStack->clear();
}

void Map::redoCommand()
{
  m_commandProcessor->redo();
  updateLinkedGroups(*this);

  // Undo/redo in the repeat system is not supported for now, so just clear the repeat
  // stack
  m_repeatStack->clear();
}

bool Map::isCommandCollationEnabled() const
{
  return m_commandProcessor->isCollationEnabled();
}

void Map::setIsCommandCollationEnabled(const bool isCommandCollationEnabled)
{
  m_commandProcessor->setIsCollationEnabled(isCommandCollationEnabled);
}

void Map::pushRepeatableCommand(RepeatableCommand command)
{
  m_repeatStack->push(std::move(command));
}

bool Map::canRepeatCommands() const
{
  return m_repeatStack->size() > 0u;
}

void Map::repeatCommands()
{
  m_repeatStack->repeat();
}

void Map::clearRepeatableCommands()
{
  m_repeatStack->clear();
}

void Map::startTransaction(std::string name, const TransactionScope scope)
{
  logger().debug() << "Starting transaction '" + name + "'";
  m_commandProcessor->startTransaction(std::move(name), scope);
  m_repeatStack->startTransaction();
}

void Map::rollbackTransaction()
{
  logger().debug() << "Rolling back transaction";
  m_commandProcessor->rollbackTransaction();
  m_repeatStack->rollbackTransaction();
}

bool Map::commitTransaction()
{
  logger().debug() << "Committing transaction";

  if (!updateLinkedGroups(*this))
  {
    cancelTransaction();
    return false;
  }

  m_commandProcessor->commitTransaction();
  m_repeatStack->commitTransaction();
  return true;
}

void Map::cancelTransaction()
{
  m_logger.debug() << "Cancelling transaction";
  m_commandProcessor->rollbackTransaction();
  m_repeatStack->rollbackTransaction();
  m_commandProcessor->commitTransaction();
  m_repeatStack->commitTransaction();
}

bool Map::isCurrentDocumentStateObservable() const
{
  return m_commandProcessor->isCurrentDocumentStateObservable();
}

bool Map::throwExceptionDuringCommand()
{
  const auto result = executeAndStore(std::make_unique<ThrowExceptionCommand>());
  return result->success();
}

std::unique_ptr<CommandResult> Map::execute(std::unique_ptr<Command>&& command)
{
  return m_commandProcessor->execute(std::move(command));
}

std::unique_ptr<CommandResult> Map::executeAndStore(
  std::unique_ptr<UndoableCommand>&& command)
{
  return m_commandProcessor->executeAndStore(std::move(command));
}

void Map::connectObservers()
{
  m_notifierConnection += mapWasCreatedNotifier.connect(this, &Map::mapWasCreated);
  m_notifierConnection += mapWasLoadedNotifier.connect(this, &Map::mapWasLoaded);

  m_notifierConnection += nodesWereAddedNotifier.connect(this, &Map::nodesWereAdded);
  m_notifierConnection +=
    nodesWillBeRemovedNotifier.connect(this, &Map::nodesWillBeRemoved);
  m_notifierConnection += nodesWereRemovedNotifier.connect(this, &Map::nodesWereRemoved);
  m_notifierConnection += nodesWillChangeNotifier.connect(this, &Map::nodesWillChange);
  m_notifierConnection += nodesDidChangeNotifier.connect(this, &Map::nodesDidChange);
  m_notifierConnection +=
    brushFacesDidChangeNotifier.connect(this, &Map::brushFacesDidChange);

  m_notifierConnection +=
    selectionDidChangeNotifier.connect(this, &Map::selectionDidChange);
  m_notifierConnection +=
    selectionWillChangeNotifier.connect(this, &Map::selectionWillChange);

  m_notifierConnection += materialCollectionsWillChangeNotifier.connect(
    this, &Map::materialCollectionsWillChange);
  m_notifierConnection += materialCollectionsDidChangeNotifier.connect(
    this, &Map::materialCollectionsDidChange);

  m_notifierConnection +=
    entityDefinitionsWillChangeNotifier.connect(this, &Map::entityDefinitionsWillChange);
  m_notifierConnection +=
    entityDefinitionsDidChangeNotifier.connect(this, &Map::entityDefinitionsDidChange);

  m_notifierConnection += modsWillChangeNotifier.connect(this, &Map::modsWillChange);
  m_notifierConnection += modsDidChangeNotifier.connect(this, &Map::modsDidChange);

  auto& prefs = PreferenceManager::instance();
  m_notifierConnection +=
    prefs.preferenceDidChangeNotifier.connect(this, &Map::preferenceDidChange);
  m_notifierConnection += m_editorContext->editorContextDidChangeNotifier.connect(
    editorContextDidChangeNotifier);
  m_notifierConnection += commandDoneNotifier.connect(this, &Map::commandDone);
  m_notifierConnection += commandUndoneNotifier.connect(this, &Map::commandUndone);
  m_notifierConnection += transactionDoneNotifier.connect(this, &Map::transactionDone);
  m_notifierConnection +=
    transactionUndoneNotifier.connect(this, &Map::transactionUndone);

  m_notifierConnection +=
    resourcesWereProcessedNotifier.connect(this, &Map::resourcesWereProcessed);

  // command processing
  m_notifierConnection +=
    m_commandProcessor->commandDoNotifier.connect(commandDoNotifier);
  m_notifierConnection +=
    m_commandProcessor->commandDoneNotifier.connect(commandDoneNotifier);
  m_notifierConnection +=
    m_commandProcessor->commandDoFailedNotifier.connect(commandDoFailedNotifier);
  m_notifierConnection +=
    m_commandProcessor->commandUndoNotifier.connect(commandUndoNotifier);
  m_notifierConnection +=
    m_commandProcessor->commandUndoneNotifier.connect(commandUndoneNotifier);
  m_notifierConnection +=
    m_commandProcessor->commandUndoFailedNotifier.connect(commandUndoFailedNotifier);
  m_notifierConnection +=
    m_commandProcessor->transactionDoneNotifier.connect(transactionDoneNotifier);
  m_notifierConnection +=
    m_commandProcessor->transactionUndoneNotifier.connect(transactionUndoneNotifier);
}

void Map::mapWasCreated(Map&)
{
  initializeAllNodeTags();
  initializeNodeIndex();
  initializeEntityLinks();
}

void Map::mapWasLoaded(Map&)
{
  initializeAllNodeTags();
  initializeNodeIndex();
  initializeEntityLinks();
}

void Map::nodesWereAdded(const std::vector<Node*>& nodes)
{
  setHasPendingChanges(collectGroups(nodes), false);
  setEntityDefinitions(nodes);
  setEntityModels(nodes);
  setMaterials(nodes);
  initializeNodeTags(nodes);
  addToNodeIndex(nodes, true);
  addEntityLinks(nodes, true);

  m_cachedSelection = std::nullopt;
  m_cachedSelectionBounds = std::nullopt;
}

void Map::nodesWillBeRemoved(const std::vector<Node*>& nodes)
{
  removeEntityLinks(nodes, true);
  removeFromNodeIndex(nodes, true);
  clearNodeTags(nodes);
}

void Map::nodesWereRemoved(const std::vector<Node*>& nodes)
{
  unsetEntityModels(nodes);
  unsetEntityDefinitions(nodes);
  unsetMaterials(nodes);

  m_cachedSelection = std::nullopt;
  m_cachedSelectionBounds = std::nullopt;
}

void Map::nodesWillChange(const std::vector<Node*>& nodes)
{
  removeEntityLinks(nodes, false);
  removeFromNodeIndex(nodes, false);
}

void Map::nodesDidChange(const std::vector<Node*>& nodes)
{
  setEntityDefinitions(nodes);
  setEntityModels(nodes);
  setMaterials(nodes);
  updateNodeTags(collectNodesAndDescendants(nodes));
  addToNodeIndex(nodes, false);
  addEntityLinks(nodes, false);

  m_cachedSelectionBounds = std::nullopt;
}

void Map::brushFacesDidChange(const std::vector<BrushFaceHandle>& brushFaces)
{
  updateFaceTags(brushFaces);
}

void Map::resourcesWereProcessed(const std::vector<ResourceId>& resourceIds)
{
  updateFaceTagsAfterResourcesWhereProcessed(resourceIds);
}

void Map::selectionWillChange()
{
  if (const auto currentSelectionBounds = selectionBounds())
  {
    m_lastSelectionBounds = currentSelectionBounds;
  }
}

void Map::selectionDidChange(const SelectionChange&)
{
  m_repeatStack->clearOnNextPush();
  m_cachedSelection = std::nullopt;
  m_cachedSelectionBounds = std::nullopt;
}

void Map::materialCollectionsWillChange()
{
  unsetMaterials();
}

void Map::materialCollectionsDidChange()
{
  loadMaterials();
  setMaterials();
  updateAllFaceTags();
}

void Map::entityDefinitionsWillChange()
{
  clearEntityDefinitions();
  clearEntityModels();
}

void Map::entityDefinitionsDidChange()
{
  loadEntityDefinitions();
  setEntityDefinitions();
  setEntityModels();
}

void Map::modsWillChange()
{
  unsetEntityModels();
  unsetEntityDefinitions();
  clearEntityModels();
}

void Map::modsDidChange()
{
  updateGameSearchPaths();
  setEntityDefinitions();
  setEntityModels();
  updateAllFaceTags();
}

void Map::preferenceDidChange(const std::filesystem::path& path)
{
  if (m_game && m_game->isGamePathPreference(path))
  {
    const auto& gameFactory = GameFactory::instance();
    const auto newGamePath = gameFactory.gamePath(m_game->config().name);
    m_game->setGamePath(newGamePath, m_logger);

    clearEntityModels();
    setEntityModels();

    reloadMaterials();
    setMaterials();
  }
}

void Map::commandDone(Command& command)
{
  m_logger.debug() << "Command '" << command.name() << "' executed";
}

void Map::commandUndone(UndoableCommand& command)
{
  m_logger.debug() << "Command '" << command.name() << "' undone";
}

void Map::transactionDone(const std::string& name)
{
  m_logger.debug() << "Transaction '" << name << "' executed";
}

void Map::transactionUndone(const std::string& name)
{
  m_logger.debug() << "Transaction '" << name << "' undone";
}

} // namespace tb::mdl
