/*
 Copyright (C) 2010 Kristian Duske

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

#include "ui/MapDocument.h"

#include "Exceptions.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Uuid.h"
#include "io/BrushFaceReader.h"
#include "io/DiskIO.h"
#include "io/ExportOptions.h"
#include "io/GameConfigParser.h"
#include "io/LoadMaterialCollections.h"
#include "io/MapHeader.h"
#include "io/NodeReader.h"
#include "io/NodeWriter.h"
#include "io/ObjSerializer.h"
#include "io/PathInfo.h"
#include "io/SimpleParserStatus.h"
#include "io/SystemPaths.h"
#include "io/WorldReader.h"
#include "mdl/AssetUtils.h"
#include "mdl/BezierPatch.h"
#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushGeometry.h"
#include "mdl/BrushNode.h"
#include "mdl/ChangeBrushFaceAttributesRequest.h"
#include "mdl/EditorContext.h"
#include "mdl/EmptyBrushEntityValidator.h"
#include "mdl/EmptyGroupValidator.h"
#include "mdl/EmptyPropertyKeyValidator.h"
#include "mdl/EmptyPropertyValueValidator.h"
#include "mdl/Entity.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityDefinitionFileSpec.h"
#include "mdl/EntityDefinitionGroup.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/EntityModelManager.h"
#include "mdl/EntityNode.h"
#include "mdl/EntityProperties.h"
#include "mdl/Game.h"
#include "mdl/GameFactory.h"
#include "mdl/GroupNode.h"
#include "mdl/InvalidUVScaleValidator.h"
#include "mdl/LayerNode.h"
#include "mdl/LinkSourceValidator.h"
#include "mdl/LinkTargetValidator.h"
#include "mdl/LinkedGroupUtils.h"
#include "mdl/LockState.h"
#include "mdl/LongPropertyKeyValidator.h"
#include "mdl/LongPropertyValueValidator.h"
#include "mdl/Material.h"
#include "mdl/MaterialManager.h"
#include "mdl/MissingClassnameValidator.h"
#include "mdl/MissingDefinitionValidator.h"
#include "mdl/MissingModValidator.h"
#include "mdl/MixedBrushContentsValidator.h"
#include "mdl/ModelUtils.h"
#include "mdl/Node.h"
#include "mdl/NodeContents.h"
#include "mdl/NodeQueries.h"
#include "mdl/NonIntegerVerticesValidator.h"
#include "mdl/PatchNode.h"
#include "mdl/PointEntityWithBrushesValidator.h"
#include "mdl/Polyhedron.h"
#include "mdl/Polyhedron3.h"
#include "mdl/PropertyKeyWithDoubleQuotationMarksValidator.h"
#include "mdl/PropertyValueWithDoubleQuotationMarksValidator.h"
#include "mdl/PushSelection.h"
#include "mdl/ResourceManager.h"
#include "mdl/SoftMapBoundsValidator.h"
#include "mdl/TagManager.h"
#include "mdl/VisibilityState.h"
#include "mdl/WorldBoundsValidator.h"
#include "mdl/WorldNode.h"
#include "ui/Actions.h"
#include "ui/AddRemoveNodesCommand.h"
#include "ui/BrushVertexCommands.h"
#include "ui/CurrentGroupCommand.h"
#include "ui/Grid.h"
#include "ui/MapTextEncoding.h"
#include "ui/PasteType.h"
#include "ui/ReparentNodesCommand.h"
#include "ui/RepeatStack.h"
#include "ui/SelectionCommand.h"
#include "ui/SetCurrentLayerCommand.h"
#include "ui/SetLinkIdsCommand.h"
#include "ui/SetLockStateCommand.h"
#include "ui/SetVisibilityCommand.h"
#include "ui/SwapNodeContentsCommand.h"
#include "ui/Transaction.h"
#include "ui/TransactionScope.h"
#include "ui/UpdateLinkedGroupsCommand.h"
#include "ui/UpdateLinkedGroupsHelper.h"
#include "ui/ViewEffectsService.h"

#include "kdl/collection_utils.h"
#include "kdl/grouped_range.h"
#include "kdl/map_utils.h"
#include "kdl/overload.h"
#include "kdl/path_utils.h"
#include "kdl/range_utils.h"
#include "kdl/result.h"
#include "kdl/result_fold.h"
#include "kdl/stable_remove_duplicates.h"
#include "kdl/string_format.h"
#include "kdl/task_manager.h"
#include "kdl/vector_set.h"
#include "kdl/vector_utils.h"

#include "vm/polygon.h"
#include "vm/util.h"
#include "vm/vec.h"
#include "vm/vec_io.h"

#include <fmt/format.h>
#include <fmt/std.h>

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <map>
#include <ranges>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>


namespace tb::ui
{

namespace
{

template <typename T>
auto collectContainingGroups(const std::vector<T*>& nodes)
{
  auto result = std::vector<mdl::GroupNode*>{};
  mdl::Node::visitAll(
    nodes,
    kdl::overload(
      [](const mdl::WorldNode*) {},
      [](const mdl::LayerNode*) {},
      [&](mdl::GroupNode* groupNode) {
        if (auto* containingGroupNode = groupNode->containingGroup())
        {
          result.push_back(containingGroupNode);
        }
      },
      [&](mdl::EntityNode* entityNode) {
        if (auto* containingGroupNode = entityNode->containingGroup())
        {
          result.push_back(containingGroupNode);
        }
      },
      [&](mdl::BrushNode* brushNode) {
        if (auto* containingGroupNode = brushNode->containingGroup())
        {
          result.push_back(containingGroupNode);
        }
      },
      [&](mdl::PatchNode* patchNode) {
        if (auto* containingGroupNode = patchNode->containingGroup())
        {
          result.push_back(containingGroupNode);
        }
      }));

  return kdl::vec_sort_and_remove_duplicates(std::move(result));
}

std::vector<mdl::GroupNode*> collectGroupsOrContainers(
  const std::vector<mdl::Node*>& nodes)
{
  auto result = std::vector<mdl::GroupNode*>{};
  mdl::Node::visitAll(
    nodes,
    kdl::overload(
      [](const mdl::WorldNode*) {},
      [](const mdl::LayerNode*) {},
      [&](mdl::GroupNode* groupNode) { result.push_back(groupNode); },
      [&](mdl::EntityNode* entityNode) {
        if (auto* containingGroup = entityNode->containingGroup())
        {
          result.push_back(containingGroup);
        }
      },
      [&](mdl::BrushNode* brushNode) {
        if (auto* containingGroup = brushNode->containingGroup())
        {
          result.push_back(containingGroup);
        }
      },
      [&](mdl::PatchNode* patchNode) {
        if (auto* containingGroup = patchNode->containingGroup())
        {
          result.push_back(containingGroup);
        }
      }));
  return kdl::vec_sort_and_remove_duplicates(std::move(result));
}

/**
 * Applies the given lambda to a copy of the contents of each of the given nodes and
 * returns a vector of pairs of the original node and the modified contents.
 *
 * The lambda L needs three overloads:
 * - bool operator()(mdl::Entity&);
 * - bool operator()(mdl::Brush&);
 * - bool operator()(mdl::BezierPatch&);
 *
 * The given node contents should be modified in place and the lambda should return true
 * if it was applied successfully and false otherwise.
 *
 * Returns a vector of pairs which map each node to its modified contents if the lambda
 * succeeded for every given node, or an empty optional otherwise.
 */
template <typename N, typename L>
std::optional<std::vector<std::pair<mdl::Node*, mdl::NodeContents>>> applyToNodeContents(
  const std::vector<N*>& nodes, L lambda)
{
  using NodeContentType =
    std::variant<mdl::Layer, mdl::Group, mdl::Entity, mdl::Brush, mdl::BezierPatch>;

  auto newNodes = std::vector<std::pair<mdl::Node*, mdl::NodeContents>>{};
  newNodes.reserve(nodes.size());

  bool success = true;
  std::transform(
    std::begin(nodes), std::end(nodes), std::back_inserter(newNodes), [&](auto* node) {
      NodeContentType nodeContents = node->accept(kdl::overload(
        [](const mdl::WorldNode* worldNode) -> NodeContentType {
          return worldNode->entity();
        },
        [](const mdl::LayerNode* layerNode) -> NodeContentType {
          return layerNode->layer();
        },
        [](const mdl::GroupNode* groupNode) -> NodeContentType {
          return groupNode->group();
        },
        [](const mdl::EntityNode* entityNode) -> NodeContentType {
          return entityNode->entity();
        },
        [](const mdl::BrushNode* brushNode) -> NodeContentType {
          return brushNode->brush();
        },
        [](const mdl::PatchNode* patchNode) -> NodeContentType {
          return patchNode->patch();
        }));

      success = success && std::visit(lambda, nodeContents);
      return std::make_pair(node, mdl::NodeContents(std::move(nodeContents)));
    });

  return success ? std::make_optional(newNodes) : std::nullopt;
}

/**
 * Applies the given lambda to a copy of the contents of each of the given nodes and
 * swaps the node contents if the given lambda succeeds for all node contents.
 *
 * The lambda L needs three overloads:
 * - bool operator()(mdl::Entity&);
 * - bool operator()(mdl::Brush&);
 * - bool operator()(mdl::BezierPatch&);
 *
 * The given node contents should be modified in place and the lambda should return true
 * if it was applied successfully and false otherwise.
 *
 * For each linked group in the given list of linked groups, its changes are distributed
 * to the connected members of its link set.
 *
 * Returns true if the given lambda could be applied successfully to all node contents
 * and false otherwise. If the lambda fails, then no node contents will be swapped, and
 * the original nodes remain unmodified.
 */
template <typename N, typename L>
bool applyAndSwap(
  MapDocument& document,
  const std::string& commandName,
  const std::vector<N*>& nodes,
  std::vector<mdl::GroupNode*> changedLinkedGroups,
  L lambda)
{
  if (nodes.empty())
  {
    return true;
  }

  if (auto newNodes = applyToNodeContents(nodes, std::move(lambda)))
  {
    return document.swapNodeContents(
      commandName, std::move(*newNodes), std::move(changedLinkedGroups));
  }

  return false;
}

/**
 * Applies the given lambda to a copy of each of the given faces.
 *
 * Specifically, each brush node of the given faces has its contents copied and the
 * lambda applied to the copied faces. If the lambda succeeds for each face, the node
 * contents are subsequently swapped.
 *
 * The lambda L needs to accept brush faces:
 * - bool operator()(mdl::BrushFace&);
 *
 * The given node contents should be modified in place and the lambda should return true
 * if it was applied successfully and false otherwise.
 *
 * For each linked group in the given list of linked groups, its changes are distributed
 * to the connected members of its link set.
 *
 * Returns true if the given lambda could be applied successfully to each face and false
 * otherwise. If the lambda fails, then no node contents will be swapped, and the
 * original nodes remain unmodified.
 */
template <typename L>
bool applyAndSwap(
  MapDocument& document,
  const std::string& commandName,
  const std::vector<mdl::BrushFaceHandle>& faces,
  L lambda)
{
  if (faces.empty())
  {
    return true;
  }

  auto brushes = std::unordered_map<mdl::BrushNode*, mdl::Brush>{};

  bool success = true;
  std::for_each(std::begin(faces), std::end(faces), [&](const auto& faceHandle) {
    auto* brushNode = faceHandle.node();
    auto it = brushes.find(brushNode);
    if (it == std::end(brushes))
    {
      it = brushes.emplace(brushNode, brushNode->brush()).first;
    }

    auto& brush = it->second;
    success = success && lambda(brush.face(faceHandle.faceIndex()));
  });

  if (success)
  {
    auto newNodes = std::vector<std::pair<mdl::Node*, mdl::NodeContents>>{};
    newNodes.reserve(brushes.size());

    for (auto& [brushNode, brush] : brushes)
    {
      newNodes.emplace_back(brushNode, mdl::NodeContents(std::move(brush)));
    }

    auto changedLinkedGroups = collectContainingGroups(
      kdl::vec_transform(newNodes, [](const auto& p) { return p.first; }));
    document.swapNodeContents(
      commandName, std::move(newNodes), std::move(changedLinkedGroups));
  }

  return success;
}
} // namespace

const vm::bbox3d MapDocument::DefaultWorldBounds(-32768.0, 32768.0);
const std::string MapDocument::DefaultDocumentName("unnamed.map");

MapDocument::MapDocument(kdl::task_manager& taskManager)
  : m_taskManager{taskManager}
  , m_resourceManager{std::make_unique<mdl::ResourceManager>()}
  , m_entityDefinitionManager{std::make_unique<mdl::EntityDefinitionManager>()}
  , m_entityModelManager{std::make_unique<mdl::EntityModelManager>(
      [&](auto resourceLoader) {
        auto resource =
          std::make_shared<mdl::EntityModelDataResource>(std::move(resourceLoader));
        m_resourceManager->addResource(resource);
        return resource;
      },
      logger())}
  , m_materialManager{std::make_unique<mdl::MaterialManager>(logger())}
  , m_tagManager{std::make_unique<mdl::TagManager>()}
  , m_editorContext{std::make_unique<mdl::EditorContext>()}
  , m_grid{std::make_unique<Grid>(4)}
  , m_repeatStack{std::make_unique<RepeatStack>()}
{
  connectObservers();
}

MapDocument::~MapDocument()
{
  if (isPointFileLoaded())
  {
    unloadPointFile();
  }
  if (isPortalFileLoaded())
  {
    unloadPortalFile();
  }
  clearWorld();
}

kdl::task_manager& MapDocument::taskManager()
{
  return m_taskManager;
}

Logger& MapDocument::logger()
{
  return *this;
}

std::shared_ptr<mdl::Game> MapDocument::game() const
{
  return m_game;
}

const vm::bbox3d& MapDocument::worldBounds() const
{
  return m_worldBounds;
}

mdl::WorldNode* MapDocument::world() const
{
  return m_world.get();
}

bool MapDocument::isGamePathPreference(const std::filesystem::path& path) const
{
  return m_game.get() != nullptr && m_game->isGamePathPreference(path);
}

mdl::LayerNode* MapDocument::currentLayer() const
{
  ensure(m_currentLayer != nullptr, "currentLayer is null");
  return m_currentLayer;
}

/**
 * Sets the current layer immediately, without adding a Command to the undo stack.
 */
mdl::LayerNode* MapDocument::performSetCurrentLayer(mdl::LayerNode* currentLayer)
{
  ensure(currentLayer != nullptr, "currentLayer is null");

  mdl::LayerNode* oldCurrentLayer = m_currentLayer;
  m_currentLayer = currentLayer;
  currentLayerDidChangeNotifier(m_currentLayer);

  return oldCurrentLayer;
}

void MapDocument::setCurrentLayer(mdl::LayerNode* currentLayer)
{
  ensure(m_currentLayer != nullptr, "old currentLayer is null");
  ensure(currentLayer != nullptr, "new currentLayer is null");

  auto transaction = Transaction{*this, "Set Current Layer"};

  while (currentGroup() != nullptr)
  {
    closeGroup();
  }

  const auto descendants = mdl::collectDescendants({m_currentLayer});
  downgradeShownToInherit(descendants);
  downgradeUnlockedToInherit(descendants);

  executeAndStore(SetCurrentLayerCommand::set(currentLayer));
  transaction.commit();
}

bool MapDocument::canSetCurrentLayer(mdl::LayerNode* currentLayer) const
{
  return m_currentLayer != currentLayer;
}

mdl::GroupNode* MapDocument::currentGroup() const
{
  return m_editorContext->currentGroup();
}

mdl::Node* MapDocument::currentGroupOrWorld() const
{
  mdl::Node* result = currentGroup();
  if (result == nullptr)
  {
    result = m_world.get();
  }
  return result;
}

mdl::Node* MapDocument::parentForNodes(const std::vector<mdl::Node*>& nodes) const
{
  if (nodes.empty())
  {
    // No reference nodes, so return either the current group (if open) or current layer
    mdl::Node* result = currentGroup();
    if (result == nullptr)
    {
      result = currentLayer();
    }
    return result;
  }

  mdl::GroupNode* parentGroup = mdl::findContainingGroup(nodes.at(0));
  if (parentGroup != nullptr)
  {
    return parentGroup;
  }

  mdl::LayerNode* parentLayer = mdl::findContainingLayer(nodes.at(0));
  ensure(parentLayer != nullptr, "no parent layer");
  return parentLayer;
}

mdl::EditorContext& MapDocument::editorContext() const
{
  return *m_editorContext;
}

mdl::EntityDefinitionManager& MapDocument::entityDefinitionManager()
{
  return *m_entityDefinitionManager;
}

mdl::EntityModelManager& MapDocument::entityModelManager()
{
  return *m_entityModelManager;
}

mdl::MaterialManager& MapDocument::materialManager()
{
  return *m_materialManager;
}

Grid& MapDocument::grid() const
{
  return *m_grid;
}

mdl::PointTrace* MapDocument::pointFile()
{
  return m_pointFile ? &m_pointFile->trace : nullptr;
}

const mdl::PortalFile* MapDocument::portalFile() const
{
  return m_portalFile ? &m_portalFile->portalFile : nullptr;
}

void MapDocument::setViewEffectsService(ViewEffectsService* viewEffectsService)
{
  m_viewEffectsService = viewEffectsService;
}

void MapDocument::createTagActions()
{
  const auto& actionManager = ActionManager::instance();
  m_tagActions = actionManager.createTagActions(m_tagManager->smartTags());
}

void MapDocument::clearTagActions()
{
  m_tagActions.clear();
}

void MapDocument::createEntityDefinitionActions()
{
  const auto& actionManager = ActionManager::instance();
  m_entityDefinitionActions =
    actionManager.createEntityDefinitionActions(m_entityDefinitionManager->definitions());
}

namespace
{
Result<std::unique_ptr<mdl::WorldNode>> loadMap(
  const mdl::GameConfig& config,
  const mdl::MapFormat mapFormat,
  const vm::bbox3d& worldBounds,
  const std::filesystem::path& path,
  kdl::task_manager& taskManager,
  Logger& logger)
{
  const auto entityPropertyConfig = mdl::EntityPropertyConfig{
    config.entityConfig.scaleExpression, config.entityConfig.setDefaultProperties};

  auto parserStatus = io::SimpleParserStatus{logger};
  return io::Disk::openFile(path) | kdl::and_then([&](auto file) {
           auto fileReader = file->reader().buffer();
           if (mapFormat == mdl::MapFormat::Unknown)
           {
             // Try all formats listed in the game config
             const auto possibleFormats =
               config.fileFormats | std::views::transform([](const auto& formatConfig) {
                 return mdl::formatFromName(formatConfig.format);
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

Result<std::unique_ptr<mdl::WorldNode>> newMap(
  const mdl::GameConfig& config,
  const mdl::MapFormat format,
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

  auto worldEntity = mdl::Entity{};
  if (!config.forceEmptyNewMap)
  {
    if (
      format == mdl::MapFormat::Valve || format == mdl::MapFormat::Quake2_Valve
      || format == mdl::MapFormat::Quake3_Valve)
    {
      worldEntity.addOrUpdateProperty(mdl::EntityPropertyKeys::ValveVersion, "220");
    }

    if (config.materialConfig.property)
    {
      worldEntity.addOrUpdateProperty(*config.materialConfig.property, "");
    }
  }

  auto entityPropertyConfig = mdl::EntityPropertyConfig{
    config.entityConfig.scaleExpression, config.entityConfig.setDefaultProperties};
  auto worldNode = std::make_unique<mdl::WorldNode>(
    std::move(entityPropertyConfig), std::move(worldEntity), format);

  if (!config.forceEmptyNewMap)
  {
    const auto builder = mdl::BrushBuilder{
      worldNode->mapFormat(), worldBounds, config.faceAttribsConfig.defaults};
    builder.createCuboid({128.0, 128.0, 32.0}, mdl::BrushFaceAttributes::NoMaterialName)
      | kdl::transform([&](auto b) {
          worldNode->defaultLayer()->addChild(new mdl::BrushNode{std::move(b)});
        })
      | kdl::transform_error(
        [&](auto e) { logger.error() << "Could not create default brush: " << e.msg; });
  }

  return worldNode;
}

void setWorldDefaultProperties(
  mdl::WorldNode& world, mdl::EntityDefinitionManager& entityDefinitionManager)
{
  const auto definition =
    entityDefinitionManager.definition(mdl::EntityPropertyValues::WorldspawnClassname);

  if (definition && world.entityPropertyConfig().setDefaultProperties)
  {
    auto entity = world.entity();
    mdl::setDefaultProperties(*definition, entity, mdl::SetDefaultPropertyMode::SetAll);
    world.setEntity(std::move(entity));
  }
}
} // namespace

Result<void> MapDocument::newDocument(
  const mdl::MapFormat mapFormat,
  const vm::bbox3d& worldBounds,
  std::shared_ptr<mdl::Game> game)
{
  info("Creating new document");

  clearDocument();

  return newMap(game->config(), mapFormat, m_worldBounds, m_taskManager, logger())
         | kdl::transform([&](auto worldNode) {
             setWorld(worldBounds, std::move(worldNode), game, DefaultDocumentName);
             setWorldDefaultProperties(*m_world, *m_entityDefinitionManager);
             clearModificationCount();
             documentWasNewedNotifier(this);
           });
}

Result<void> MapDocument::loadDocument(
  const mdl::MapFormat mapFormat,
  const vm::bbox3d& worldBounds,
  std::shared_ptr<mdl::Game> game,
  const std::filesystem::path& path)
{
  info(fmt::format("Loading document from {}", path));

  clearDocument();

  return loadMap(game->config(), mapFormat, worldBounds, path, m_taskManager, logger())
         | kdl::transform([&](auto worldNode) {
             setWorld(worldBounds, std::move(worldNode), game, path);
             documentWasLoadedNotifier(this);
           });
}

void MapDocument::saveDocument()
{
  doSaveDocument(m_path);
}

void MapDocument::saveDocumentAs(const std::filesystem::path& path)
{
  doSaveDocument(path);
}

void MapDocument::saveDocumentTo(const std::filesystem::path& path)
{
  ensure(m_game.get() != nullptr, "game is null");
  ensure(m_world, "world is null");

  io::Disk::withOutputStream(path, [&](auto& stream) {
    io::writeMapHeader(stream, m_game->config().name, m_world->mapFormat());

    auto writer = io::NodeWriter{*m_world, stream};
    writer.setExporting(false);
    writer.writeMap(m_taskManager);
  }) | kdl::transform_error([&](const auto& e) {
    error() << "Could not save document: " << e.msg;
  });
}

Result<void> MapDocument::exportDocumentAs(const io::ExportOptions& options)
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

void MapDocument::doSaveDocument(const std::filesystem::path& path)
{
  saveDocumentTo(path);
  setLastSaveModificationCount();
  setPath(path);
  documentWasSavedNotifier(this);
}

void MapDocument::clearDocument()
{
  clearRepeatableCommands();
  doClearCommandProcessor();

  if (m_world)
  {
    documentWillBeClearedNotifier(this);

    m_editorContext->reset();
    clearSelection();
    unloadAssets();
    clearTagActions();
    clearWorld();
    clearModificationCount();

    documentWasClearedNotifier(this);
  }
}

MapTextEncoding MapDocument::encoding() const
{
  return MapTextEncoding::Quake;
}

std::string MapDocument::serializeSelectedNodes()
{
  std::stringstream stream;
  auto writer = io::NodeWriter{*m_world, stream};
  writer.writeNodes(selectedNodes().nodes(), m_taskManager);
  return stream.str();
}

std::string MapDocument::serializeSelectedBrushFaces()
{
  std::stringstream stream;
  auto writer = io::NodeWriter{*m_world, stream};
  writer.writeBrushFaces(
    m_selectedBrushFaces | std::views::transform([](const auto& h) { return h.face(); })
      | kdl::to_vector,
    m_taskManager);
  return stream.str();
}

PasteType MapDocument::paste(const std::string& str)
{
  auto parserStatus = io::SimpleParserStatus{logger()};

  // Try parsing as entities, then as brushes, in all compatible formats
  return io::NodeReader::read(
           str,
           m_world->mapFormat(),
           m_worldBounds,
           m_world->entityPropertyConfig(),
           parserStatus,
           m_taskManager)
         | kdl::transform([&](auto nodes) {
             return pasteNodes(nodes) ? PasteType::Node : PasteType::Failed;
           })
         | kdl::or_else([&](const auto& nodeError) {
             // Try parsing as brush faces
             auto reader = io::BrushFaceReader{str, m_world->mapFormat()};
             return reader.read(m_worldBounds, parserStatus)
                    | kdl::transform([&](const auto& faces) {
                        return !faces.empty() && pasteBrushFaces(faces)
                                 ? PasteType::BrushFace
                                 : PasteType::Failed;
                      })
                    | kdl::transform_error([&](const auto& faceError) {
                        error() << "Could not parse clipboard contents as nodes: "
                                << nodeError.msg;
                        error() << "Could not parse clipboard contents as faces: "
                                << faceError.msg;
                        return PasteType::Failed;
                      });
           })
         | kdl::value();
}

namespace
{

auto extractNodesToPaste(const std::vector<mdl::Node*>& nodes, mdl::Node* parent)
{
  auto nodesToDetach = std::vector<mdl::Node*>{};
  auto nodesToDelete = std::vector<mdl::Node*>{};
  auto nodesToAdd = std::map<mdl::Node*, std::vector<mdl::Node*>>{};

  for (auto* node : nodes)
  {
    node->accept(kdl::overload(
      [&](auto&& thisLambda, mdl::WorldNode* world) {
        world->visitChildren(thisLambda);
        nodesToDelete.push_back(world);
      },
      [&](auto&& thisLambda, mdl::LayerNode* layer) {
        layer->visitChildren(thisLambda);
        nodesToDetach.push_back(layer);
        nodesToDelete.push_back(layer);
      },
      [&](mdl::GroupNode* group) {
        nodesToDetach.push_back(group);
        nodesToAdd[parent].push_back(group);
      },
      [&](auto&& thisLambda, mdl::EntityNode* entityNode) {
        if (mdl::isWorldspawn(entityNode->entity().classname()))
        {
          entityNode->visitChildren(thisLambda);
          nodesToDetach.push_back(entityNode);
          nodesToDelete.push_back(entityNode);
        }
        else
        {
          nodesToDetach.push_back(entityNode);
          nodesToAdd[parent].push_back(entityNode);
        }
      },
      [&](mdl::BrushNode* brush) {
        nodesToDetach.push_back(brush);
        nodesToAdd[parent].push_back(brush);
      },
      [&](mdl::PatchNode* patch) {
        nodesToDetach.push_back(patch);
        nodesToAdd[parent].push_back(patch);
      }));
  }

  for (auto* node : nodesToDetach)
  {
    if (auto* nodeParent = node->parent())
    {
      nodeParent->removeChild(node);
    }
  }
  kdl::vec_clear_and_delete(nodesToDelete);

  return nodesToAdd;
}

std::vector<mdl::IdType> allPersistentGroupIds(const mdl::Node& root)
{
  auto result = std::vector<mdl::IdType>{};
  root.accept(kdl::overload(
    [](auto&& thisLambda, const mdl::WorldNode* worldNode) {
      worldNode->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, const mdl::LayerNode* layerNode) {
      layerNode->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, const mdl::GroupNode* groupNode) {
      if (const auto persistentId = groupNode->persistentId())
      {
        result.push_back(*persistentId);
      }
      groupNode->visitChildren(thisLambda);
    },
    [](const mdl::EntityNode*) {},
    [](const mdl::BrushNode*) {},
    [](const mdl::PatchNode*) {}));
  return result;
}

void fixRedundantPersistentIds(
  const std::map<mdl::Node*, std::vector<mdl::Node*>>& nodesToAdd,
  const std::vector<mdl::IdType>& existingPersistentGroupIds)
{
  auto persistentGroupIds = kdl::vector_set{existingPersistentGroupIds};
  for (auto& [newParent, nodesToAddToParent] : nodesToAdd)
  {
    for (auto* node : nodesToAddToParent)
    {
      node->accept(kdl::overload(
        [&](auto&& thisLambda, mdl::WorldNode* worldNode) {
          worldNode->visitChildren(thisLambda);
        },
        [&](auto&& thisLambda, mdl::LayerNode* layerNode) {
          layerNode->visitChildren(thisLambda);
        },
        [&](auto&& thisLambda, mdl::GroupNode* groupNode) {
          if (const auto persistentGroupId = groupNode->persistentId())
          {
            if (!persistentGroupIds.insert(*persistentGroupId).second)
            {
              // a group with this ID is already in the map or being pasted
              groupNode->resetPersistentId();
            }
          }
          groupNode->visitChildren(thisLambda);
        },
        [](mdl::EntityNode*) {},
        [](mdl::BrushNode*) {},
        [](mdl::PatchNode*) {}));
    }
  }
}

void fixRecursiveLinkedGroups(
  const std::map<mdl::Node*, std::vector<mdl::Node*>>& nodesToAdd, Logger& logger)
{
  for (auto& [newParent, nodesToAddToParent] : nodesToAdd)
  {
    const auto linkedGroupIds =
      kdl::vec_sort(mdl::collectParentLinkedGroupIds(*newParent));
    for (auto* node : nodesToAddToParent)
    {
      node->accept(kdl::overload(
        [&](auto&& thisLambda, mdl::WorldNode* worldNode) {
          worldNode->visitChildren(thisLambda);
        },
        [&](auto&& thisLambda, mdl::LayerNode* layerNode) {
          layerNode->visitChildren(thisLambda);
        },
        [&](auto&& thisLambda, mdl::GroupNode* groupNode) {
          const auto& linkId = groupNode->linkId();
          if (std::binary_search(linkedGroupIds.begin(), linkedGroupIds.end(), linkId))
          {
            logger.warn() << "Unlinking recursive linked group with ID '" << linkId
                          << "'";

            auto group = groupNode->group();
            group.setTransformation(vm::mat4x4d::identity());
            groupNode->setGroup(std::move(group));
            groupNode->setLinkId(generateUuid());
          }
          groupNode->visitChildren(thisLambda);
        },
        [](mdl::EntityNode*) {},
        [](mdl::BrushNode*) {},
        [](mdl::PatchNode*) {}));
    }
  }
}

void copyAndSetLinkIds(
  const std::map<mdl::Node*, std::vector<mdl::Node*>>& nodesToAdd,
  mdl::WorldNode& worldNode,
  Logger& logger)
{
  // Recursively collect all groups to add
  const auto groupsToAdd = kdl::vec_sort(
    mdl::collectGroups(kdl::vec_flatten(kdl::map_values(nodesToAdd))),
    mdl::compareGroupNodesByLinkId);

  const auto groupsByLinkId = kdl::make_grouped_range(
    groupsToAdd,
    [](const auto* lhs, const auto* rhs) { return lhs->linkId() == rhs->linkId(); });

  for (const auto& linkedGroupsToAdd : groupsByLinkId)
  {
    const auto& linkId = linkedGroupsToAdd.front()->linkId();
    const auto existingLinkedNodes = mdl::collectNodesWithLinkId({&worldNode}, linkId);

    if (existingLinkedNodes.size() == 1)
    {
      // Unlink the added nodes because we don't want to create linked duplicates
      mdl::resetLinkIds({linkedGroupsToAdd.front()});

      if (std::next(linkedGroupsToAdd.begin()) != linkedGroupsToAdd.end())
      {
        // But keep the added linked groups mutually linked
        mdl::copyAndSetLinkIds(
          *linkedGroupsToAdd.front(),
          std::vector(std::next(linkedGroupsToAdd.begin()), linkedGroupsToAdd.end()));
      }
    }
    else if (existingLinkedNodes.size() > 1)
    {
      // Keep the pasted nodes linked to their originals, but validate the structure
      if (
        auto* existingLinkedGroup =
          dynamic_cast<mdl::GroupNode*>(existingLinkedNodes.front()))
      {
        const auto errors = mdl::copyAndSetLinkIds(
          *existingLinkedGroup,
          std::vector(linkedGroupsToAdd.begin(), linkedGroupsToAdd.end()));
        for (const auto& error : errors)
        {
          logger.warn() << "Could not paste linked groups: " + error.msg;
        }
      }
    }
  }
}

} // namespace

bool MapDocument::pasteNodes(const std::vector<mdl::Node*>& nodes)
{
  const auto nodesToAdd = extractNodesToPaste(nodes, parentForNodes());
  fixRedundantPersistentIds(nodesToAdd, allPersistentGroupIds(*m_world.get()));
  fixRecursiveLinkedGroups(nodesToAdd, *this);
  copyAndSetLinkIds(nodesToAdd, *m_world, *this);

  auto transaction = Transaction{*this, "Paste Nodes"};

  const auto addedNodes = addNodes(nodesToAdd);
  if (addedNodes.empty())
  {
    transaction.cancel();
    return false;
  }

  deselectAll();
  selectNodes(mdl::collectSelectableNodes(addedNodes, editorContext()));
  transaction.commit();

  return true;
}

bool MapDocument::pasteBrushFaces(const std::vector<mdl::BrushFace>& faces)
{
  assert(!faces.empty());
  return setFaceAttributesExceptContentFlags(faces.back().attributes());
}

void MapDocument::loadPointFile(std::filesystem::path path)
{
  static_assert(
    !std::is_reference_v<decltype(path)>,
    "path must be passed by value because reloadPointFile() passes m_pointFilePath");

  if (isPointFileLoaded())
  {
    unloadPointFile();
  }

  io::Disk::withInputStream(path, [&](auto& stream) {
    return mdl::loadPointFile(stream) | kdl::transform([&](auto trace) {
             info() << "Loaded point file " << path;
             m_pointFile = PointFile{std::move(trace), std::move(path)};
             pointFileWasLoadedNotifier();
           });
  }) | kdl::transform_error([&](auto e) {
    error() << "Couldn't load portal file " << path << ": " << e.msg;
    m_pointFile = {};
  });
}

bool MapDocument::isPointFileLoaded() const
{
  return m_pointFile != std::nullopt;
}

bool MapDocument::canReloadPointFile() const
{
  return isPointFileLoaded();
}

void MapDocument::reloadPointFile()
{
  assert(isPointFileLoaded());
  loadPointFile(m_pointFile->path);
}

void MapDocument::unloadPointFile()
{
  assert(isPointFileLoaded());
  m_pointFile = std::nullopt;

  info() << "Unloaded point file";
  pointFileWasUnloadedNotifier();
}

void MapDocument::loadPortalFile(std::filesystem::path path)
{
  static_assert(
    !std::is_reference_v<decltype(path)>,
    "path must be passed by value because reloadPortalFile() passes m_portalFilePath");

  if (!mdl::canLoadPortalFile(path))
  {
    return;
  }

  if (isPortalFileLoaded())
  {
    unloadPortalFile();
  }


  io::Disk::withInputStream(path, [&](auto& stream) {
    return mdl::loadPortalFile(stream) | kdl::transform([&](auto portalFile) {
             info() << "Loaded portal file " << path;
             m_portalFile = {std::move(portalFile), std::move(path)};
             portalFileWasLoadedNotifier();
           });
  }) | kdl::transform_error([&](auto e) {
    error() << "Couldn't load portal file " << path << ": " << e.msg;
    m_portalFile = std::nullopt;
  });
}

bool MapDocument::isPortalFileLoaded() const
{
  return m_portalFile != std::nullopt;
}

bool MapDocument::canReloadPortalFile() const
{
  return m_portalFile && mdl::canLoadPortalFile(m_portalFile->path);
}

void MapDocument::reloadPortalFile()
{
  assert(isPortalFileLoaded());
  loadPortalFile(m_portalFile->path);
}

void MapDocument::unloadPortalFile()
{
  assert(isPortalFileLoaded());
  m_portalFile = std::nullopt;

  info() << "Unloaded portal file";
  portalFileWasUnloadedNotifier();
}

bool MapDocument::hasSelection() const
{
  return hasSelectedNodes() || hasSelectedBrushFaces();
}

bool MapDocument::hasSelectedNodes() const
{
  return !m_selectedNodes.empty();
}

bool MapDocument::hasSelectedBrushFaces() const
{
  return !m_selectedBrushFaces.empty();
}

bool MapDocument::hasAnySelectedBrushFaces() const
{
  return hasSelectedBrushFaces() || selectedNodes().hasBrushes();
}

std::vector<mdl::EntityNodeBase*> MapDocument::allSelectedEntityNodes() const
{
  if (!hasSelection())
  {
    return m_world ? std::vector<mdl::EntityNodeBase*>({m_world.get()})
                   : std::vector<mdl::EntityNodeBase*>{};
  }

  auto result = std::vector<mdl::EntityNodeBase*>{};
  for (auto* node : m_selectedNodes)
  {
    node->accept(kdl::overload(
      [&](auto&& thisLambda, mdl::WorldNode* world) {
        result.push_back(world);
        world->visitChildren(thisLambda);
      },
      [&](auto&& thisLambda, mdl::LayerNode* layer) { layer->visitChildren(thisLambda); },
      [&](auto&& thisLambda, mdl::GroupNode* group) { group->visitChildren(thisLambda); },
      [&](mdl::EntityNode* entity) { result.push_back(entity); },
      [&](mdl::BrushNode* brush) { result.push_back(brush->entity()); },
      [&](mdl::PatchNode* patch) { result.push_back(patch->entity()); }));
  }

  result = kdl::vec_sort_and_remove_duplicates(std::move(result));

  // Don't select worldspawn together with any other entities
  return result.size() == 1
           ? result
           : kdl::vec_filter(std::move(result), [](const auto* entityNode) {
               return entityNode->entity().classname()
                      != mdl::EntityPropertyValues::WorldspawnClassname;
             });
}

std::vector<mdl::BrushNode*> MapDocument::allSelectedBrushNodes() const
{
  auto brushes = std::vector<mdl::BrushNode*>{};
  for (auto* node : m_selectedNodes.nodes())
  {
    node->accept(kdl::overload(
      [](auto&& thisLambda, mdl::WorldNode* world) { world->visitChildren(thisLambda); },
      [](auto&& thisLambda, mdl::LayerNode* layer) { layer->visitChildren(thisLambda); },
      [](auto&& thisLambda, mdl::GroupNode* group) { group->visitChildren(thisLambda); },
      [](auto&& thisLambda, mdl::EntityNode* entity) {
        entity->visitChildren(thisLambda);
      },
      [&](mdl::BrushNode* brush) { brushes.push_back(brush); },
      [&](mdl::PatchNode*) {}));
  }
  return brushes;
}

bool MapDocument::hasAnySelectedBrushNodes() const
{
  // This is just an optimization of `!allSelectedBrushNodes().empty()`
  // that stops after finding the first brush
  const auto visitChildrenAndExitEarly = [](auto&& thisLambda, const auto* node) {
    for (const auto* child : node->children())
    {
      if (child->accept(thisLambda))
      {
        return true;
      }
    }
    return false;
  };

  for (const auto* node : m_selectedNodes.nodes())
  {
    const auto hasBrush = node->accept(kdl::overload(
      [&](auto&& thisLambda, const mdl::WorldNode* world) -> bool {
        return visitChildrenAndExitEarly(thisLambda, world);
      },
      [&](auto&& thisLambda, const mdl::LayerNode* layer) -> bool {
        return visitChildrenAndExitEarly(thisLambda, layer);
      },
      [&](auto&& thisLambda, const mdl::GroupNode* group) -> bool {
        return visitChildrenAndExitEarly(thisLambda, group);
      },
      [&](auto&& thisLambda, const mdl::EntityNode* entity) -> bool {
        return visitChildrenAndExitEarly(thisLambda, entity);
      },
      [](const mdl::BrushNode*) -> bool { return true; },
      [](const mdl::PatchNode*) -> bool { return false; }));
    if (hasBrush)
    {
      return true;
    }
  }

  return false;
}

const mdl::NodeCollection& MapDocument::selectedNodes() const
{
  return m_selectedNodes;
}

std::vector<mdl::BrushFaceHandle> MapDocument::allSelectedBrushFaces() const
{
  if (hasSelectedBrushFaces())
  {
    return selectedBrushFaces();
  }

  const auto faces = mdl::collectBrushFaces(m_selectedNodes.nodes());
  return mdl::faceSelectionWithLinkedGroupConstraints(*m_world.get(), faces)
    .facesToSelect;
}

std::vector<mdl::BrushFaceHandle> MapDocument::selectedBrushFaces() const
{
  return m_selectedBrushFaces;
}

VertexHandleManager& MapDocument::vertexHandles()
{
  return m_vertexHandles;
}

EdgeHandleManager& MapDocument::edgeHandles()
{
  return m_edgeHandles;
}

FaceHandleManager& MapDocument::faceHandles()
{
  return m_faceHandles;
}

const vm::bbox3d& MapDocument::referenceBounds() const
{
  return hasSelectedNodes() ? selectionBounds() : lastSelectionBounds();
}

const vm::bbox3d& MapDocument::lastSelectionBounds() const
{
  return m_lastSelectionBounds;
}

const vm::bbox3d& MapDocument::selectionBounds() const
{
  if (!m_selectionBoundsValid)
  {
    validateSelectionBounds();
  }
  return m_selectionBounds;
}

const std::string& MapDocument::currentMaterialName() const
{
  return m_currentMaterialName;
}

void MapDocument::setCurrentMaterialName(const std::string& currentMaterialName)
{
  if (m_currentMaterialName != currentMaterialName)
  {
    m_currentMaterialName = currentMaterialName;
    currentMaterialNameDidChangeNotifier(m_currentMaterialName);
  }
}

void MapDocument::selectAllNodes()
{
  m_repeatStack->clearOnNextPush();
  executeAndStore(SelectionCommand::selectAllNodes());
}

void MapDocument::selectSiblings()
{
  const auto& nodes = selectedNodes().nodes();
  if (nodes.empty())
  {
    return;
  }

  auto visited = std::unordered_set<mdl::Node*>{};
  auto nodesToSelect = std::vector<mdl::Node*>{};

  for (auto* node : nodes)
  {
    auto* parent = node->parent();
    if (visited.insert(parent).second)
    {
      nodesToSelect = kdl::vec_concat(
        std::move(nodesToSelect),
        mdl::collectSelectableNodes(parent->children(), editorContext()));
    }
  }

  auto transaction = Transaction{*this, "Select Siblings"};
  deselectAll();
  selectNodes(nodesToSelect);
  transaction.commit();
}

void MapDocument::selectTouching(const bool del)
{
  const auto nodes = kdl::vec_filter(
    mdl::collectTouchingNodes(
      std::vector<mdl::Node*>{m_world.get()}, m_selectedNodes.brushes()),
    [&](mdl::Node* node) { return m_editorContext->selectable(node); });

  auto transaction = Transaction{*this, "Select Touching"};
  if (del)
  {
    deleteObjects();
  }
  else
  {
    deselectAll();
  }
  selectNodes(nodes);
  transaction.commit();
}

void MapDocument::selectInside(const bool del)
{
  const auto nodes = kdl::vec_filter(
    mdl::collectContainedNodes(
      std::vector<mdl::Node*>{m_world.get()}, m_selectedNodes.brushes()),
    [&](mdl::Node* node) { return m_editorContext->selectable(node); });

  auto transaction = Transaction{*this, "Select Inside"};
  if (del)
  {
    deleteObjects();
  }
  else
  {
    deselectAll();
  }
  selectNodes(nodes);
  transaction.commit();
}

void MapDocument::selectInverse()
{
  // This only selects nodes that have no selected children (or parents).
  // This is because if a brush entity only 1 selected child and 1 unselected,
  // we treat it as partially selected and don't want to try to select the entity if the
  // selection is inverted, which would reselect both children.

  auto nodesToSelect = std::vector<mdl::Node*>{};
  const auto collectNode = [&](auto* node) {
    if (
      !node->transitivelySelected() && !node->descendantSelected()
      && m_editorContext->selectable(node))
    {
      nodesToSelect.push_back(node);
    }
  };

  currentGroupOrWorld()->accept(kdl::overload(
    [](auto&& thisLambda, mdl::WorldNode* world) { world->visitChildren(thisLambda); },
    [](auto&& thisLambda, mdl::LayerNode* layer) { layer->visitChildren(thisLambda); },
    [&](auto&& thisLambda, mdl::GroupNode* group) {
      collectNode(group);
      group->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, mdl::EntityNode* entity) {
      collectNode(entity);
      entity->visitChildren(thisLambda);
    },
    [&](mdl::BrushNode* brush) { collectNode(brush); },
    [&](mdl::PatchNode* patch) { collectNode(patch); }));

  auto transaction = Transaction{*this, "Select Inverse"};
  deselectAll();
  selectNodes(nodesToSelect);
  transaction.commit();
}

void MapDocument::selectNodesWithFilePosition(const std::vector<size_t>& positions)
{
  auto nodesToSelect = std::vector<mdl::Node*>{};
  const auto hasFilePosition = [&](const auto* node) {
    return std::any_of(positions.begin(), positions.end(), [&](const auto position) {
      return node->containsLine(position);
    });
  };

  m_world->accept(kdl::overload(
    [&](auto&& thisLambda, mdl::WorldNode* worldNode) {
      worldNode->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, mdl::LayerNode* layerNode) {
      layerNode->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, mdl::GroupNode* groupNode) {
      if (hasFilePosition(groupNode))
      {
        if (m_editorContext->selectable(groupNode))
        {
          nodesToSelect.push_back(groupNode);
        }
        else
        {
          groupNode->visitChildren(thisLambda);
        }
      }
    },
    [&](auto&& thisLambda, mdl::EntityNode* entityNode) {
      if (hasFilePosition(entityNode))
      {
        if (m_editorContext->selectable(entityNode))
        {
          nodesToSelect.push_back(entityNode);
        }
        else
        {
          const auto previousCount = nodesToSelect.size();
          entityNode->visitChildren(thisLambda);
          if (previousCount == nodesToSelect.size())
          {
            // no child was selected, select all children
            nodesToSelect = kdl::vec_concat(
              std::move(nodesToSelect),
              mdl::collectSelectableNodes(entityNode->children(), *m_editorContext));
          }
        }
      }
    },
    [&](mdl::BrushNode* brushNode) {
      if (hasFilePosition(brushNode) && m_editorContext->selectable(brushNode))
      {
        nodesToSelect.push_back(brushNode);
      }
    },
    [&](mdl::PatchNode* patchNode) {
      if (hasFilePosition(patchNode) && m_editorContext->selectable(patchNode))
      {
        nodesToSelect.push_back(patchNode);
      }
    }));

  auto transaction = Transaction{*this, "Select by Line Number"};
  deselectAll();
  selectNodes(nodesToSelect);
  transaction.commit();
}

void MapDocument::selectNodes(const std::vector<mdl::Node*>& nodes)
{
  m_repeatStack->clearOnNextPush();
  executeAndStore(SelectionCommand::select(nodes));
}

void MapDocument::selectBrushFaces(const std::vector<mdl::BrushFaceHandle>& handles)
{
  m_repeatStack->clearOnNextPush();
  executeAndStore(SelectionCommand::select(handles));
  if (!handles.empty())
  {
    setCurrentMaterialName(handles.back().face().attributes().materialName());
  }
}

void MapDocument::convertToFaceSelection()
{
  m_repeatStack->clearOnNextPush();
  executeAndStore(SelectionCommand::convertToFaces());
}

void MapDocument::selectFacesWithMaterial(const mdl::Material* material)
{
  const auto faces = kdl::vec_filter(
    mdl::collectSelectableBrushFaces(
      std::vector<mdl::Node*>{m_world.get()}, *m_editorContext),
    [&](const auto& faceHandle) { return faceHandle.face().material() == material; });

  auto transaction = Transaction{*this, "Select Faces with Material"};
  deselectAll();
  selectBrushFaces(faces);
  transaction.commit();
}

void MapDocument::selectBrushesWithMaterial(const mdl::Material* material)
{
  const auto selectableNodes =
    mdl::collectSelectableNodes(std::vector<mdl::Node*>{m_world.get()}, *m_editorContext);
  const auto brushes =
    selectableNodes | std::views::filter([&](const auto& node) {
      return std::ranges::any_of(
        mdl::collectSelectableBrushFaces({node}, *m_editorContext),
        [&](const auto& faceHandle) { return faceHandle.face().material() == material; });
    })
    | kdl::to_vector;

  auto transaction = Transaction{*this, "Select Brushes with Material"};
  deselectAll();
  selectNodes(brushes);
  transaction.commit();
}

void MapDocument::selectTall(const vm::axis::type cameraAxis)
{
  const auto cameraAbsDirection = vm::vec3d::axis(cameraAxis);
  // we can't make a brush that is exactly as large as worldBounds
  const auto tallBounds = worldBounds().expand(-1.0);

  const auto min = vm::dot(tallBounds.min, cameraAbsDirection);
  const auto max = vm::dot(tallBounds.max, cameraAbsDirection);

  const auto minPlane = vm::plane3d{min, cameraAbsDirection};
  const auto maxPlane = vm::plane3d{max, cameraAbsDirection};

  const auto& selectionBrushNodes = selectedNodes().brushes();
  assert(!selectionBrushNodes.empty());

  const auto brushBuilder = mdl::BrushBuilder{world()->mapFormat(), worldBounds()};

  kdl::vec_transform(
    selectionBrushNodes,
    [&](const auto* selectionBrushNode) {
      const auto& selectionBrush = selectionBrushNode->brush();

      auto tallVertices = std::vector<vm::vec3d>{};
      tallVertices.reserve(2 * selectionBrush.vertexCount());

      for (const auto* vertex : selectionBrush.vertices())
      {
        tallVertices.push_back(minPlane.project_point(vertex->position()));
        tallVertices.push_back(maxPlane.project_point(vertex->position()));
      }

      return brushBuilder.createBrush(
               tallVertices, mdl::BrushFaceAttributes::NoMaterialName)
             | kdl::transform([](auto brush) {
                 return std::make_unique<mdl::BrushNode>(std::move(brush));
               });
    })
    | kdl::fold | kdl::transform([&](const auto& tallBrushes) {
        // delete the original selection brushes before searching for the objects to
        // select
        auto transaction = Transaction{*this, "Select Tall"};
        deleteObjects();

        const auto nodesToSelect = kdl::vec_filter(
          mdl::collectContainedNodes(
            {world()},
            kdl::vec_transform(tallBrushes, [](const auto& b) { return b.get(); })),
          [&](const auto* node) { return editorContext().selectable(node); });
        selectNodes(nodesToSelect);

        transaction.commit();
      })
    | kdl::transform_error(
      [&](auto e) { logger().error() << "Could not create selection brush: " << e.msg; });
}

void MapDocument::deselectAll()
{
  if (hasSelection())
  {
    m_repeatStack->clearOnNextPush();
    executeAndStore(SelectionCommand::deselectAll());
  }
}

void MapDocument::deselectNodes(const std::vector<mdl::Node*>& nodes)
{
  m_repeatStack->clearOnNextPush();
  executeAndStore(SelectionCommand::deselect(nodes));
}

void MapDocument::deselectBrushFaces(const std::vector<mdl::BrushFaceHandle>& handles)
{
  m_repeatStack->clearOnNextPush();
  executeAndStore(SelectionCommand::deselect(handles));
}

void MapDocument::updateLastSelectionBounds()
{
  const auto currentSelectionBounds = selectionBounds();
  if (currentSelectionBounds.is_valid() && !currentSelectionBounds.is_empty())
  {
    m_lastSelectionBounds = selectionBounds();
  }
}

void MapDocument::invalidateSelectionBounds()
{
  m_selectionBoundsValid = false;
}

void MapDocument::validateSelectionBounds() const
{
  m_selectionBounds = computeLogicalBounds(m_selectedNodes.nodes());
  m_selectionBoundsValid = true;
}

void MapDocument::clearSelection()
{
  m_selectedNodes.clear();
  m_selectedBrushFaces.clear();
}

/**
 * Takes a { parent, children } map and adds the children to the given parents.
 * The world node tree takes ownership of the children, unless the transaction fails.
 *
 * @param nodes the nodes to add and the parents to add them to
 * @return the added nodes
 */
std::vector<mdl::Node*> MapDocument::addNodes(
  const std::map<mdl::Node*, std::vector<mdl::Node*>>& nodes)
{
  for (const auto& [parent, children] : nodes)
  {
    assert(parent == m_world.get() || parent->isDescendantOf(m_world.get()));
    unused(parent);
  }

  auto transaction = Transaction{*this, "Add Objects"};
  const auto result = executeAndStore(AddRemoveNodesCommand::add(nodes));
  if (!result->success())
  {
    transaction.cancel();
    return {};
  }

  setHasPendingChanges(collectGroupsOrContainers(kdl::map_keys(nodes)), true);

  const auto addedNodes = kdl::vec_flatten(kdl::map_values(nodes));
  ensureVisible(addedNodes);
  ensureUnlocked(addedNodes);
  if (!transaction.commit())
  {
    return {};
  }

  return addedNodes;
}

/**
 * Removes the given nodes. If this causes any groups/entities to become empty, removes
 * them as well.
 *
 * Ownership of the removed nodes is transferred to the undo system.
 */
void MapDocument::removeNodes(const std::vector<mdl::Node*>& nodes)
{
  auto removableNodes = parentChildrenMap(removeImplicitelyRemovedNodes(nodes));

  auto transaction = Transaction{*this};
  while (!removableNodes.empty())
  {
    setHasPendingChanges(collectGroupsOrContainers(kdl::map_keys(removableNodes)), true);

    closeRemovedGroups(removableNodes);
    executeAndStore(AddRemoveNodesCommand::remove(removableNodes));

    removableNodes = collectRemovableParents(removableNodes);
  }

  assertResult(transaction.commit());
}

std::map<mdl::Node*, std::vector<mdl::Node*>> MapDocument::collectRemovableParents(
  const std::map<mdl::Node*, std::vector<mdl::Node*>>& nodes) const
{
  std::map<mdl::Node*, std::vector<mdl::Node*>> result;
  for (const auto& [node, children] : nodes)
  {
    if (node->removeIfEmpty() && !node->hasChildren())
    {
      mdl::Node* parent = node->parent();
      ensure(parent != nullptr, "parent is null");
      result[parent].push_back(node);
    }
  }
  return result;
}

struct MapDocument::CompareByAncestry
{
  bool operator()(const mdl::Node* lhs, const mdl::Node* rhs) const
  {
    return lhs->isAncestorOf(rhs);
  }
};

std::vector<mdl::Node*> MapDocument::removeImplicitelyRemovedNodes(
  std::vector<mdl::Node*> nodes) const
{
  if (nodes.empty())
  {
    return nodes;
  }

  nodes = kdl::vec_sort(std::move(nodes), CompareByAncestry());

  auto result = std::vector<mdl::Node*>{};
  result.reserve(nodes.size());
  result.push_back(nodes.front());

  for (size_t i = 1; i < nodes.size(); ++i)
  {
    auto* node = nodes[i];
    if (!node->isDescendantOf(result))
    {
      result.push_back(node);
    }
  }

  return result;
}

void MapDocument::closeRemovedGroups(
  const std::map<mdl::Node*, std::vector<mdl::Node*>>& toRemove)
{
  for (const auto& [parent, nodes] : toRemove)
  {
    for (const mdl::Node* node : nodes)
    {
      if (node == currentGroup())
      {
        closeGroup();
        closeRemovedGroups(toRemove);
        return;
      }
    }
  }
}

namespace
{
auto setLinkIdsForReparentingNodes(
  const std::map<mdl::Node*, std::vector<mdl::Node*>>& nodesToReparent)
{
  auto result = std::vector<std::tuple<mdl::Node*, std::string>>{};
  for (const auto& [newParent_, nodes] : nodesToReparent)
  {
    mdl::Node::visitAll(
      nodes,
      kdl::overload(
        [](const mdl::WorldNode*) {},
        [](const mdl::LayerNode*) {},
        [](const mdl::GroupNode*) {
          // group nodes can keep their ID because they should remain in their link set
        },
        [&, newParent = newParent_](auto&& thisLambda, mdl::EntityNode* entityNode) {
          if (newParent->isAncestorOf(entityNode->parent()))
          {
            result.emplace_back(entityNode, generateUuid());
            entityNode->visitChildren(thisLambda);
          }
        },
        [&, newParent = newParent_](mdl::BrushNode* brushNode) {
          if (newParent->isAncestorOf(brushNode->parent()))
          {
            result.emplace_back(brushNode, generateUuid());
          }
        },
        [&, newParent = newParent_](mdl::PatchNode* patchNode) {
          if (newParent->isAncestorOf(patchNode->parent()))
          {
            result.emplace_back(patchNode, generateUuid());
          }
        }));
  }
  return result;
}
} // namespace

bool MapDocument::reparentNodes(
  const std::map<mdl::Node*, std::vector<mdl::Node*>>& nodesToAdd)
{
  if (!checkReparenting(nodesToAdd))
  {
    return false;
  }

  const auto nodesToRemove =
    parentChildrenMap(kdl::vec_flatten(kdl::map_values(nodesToAdd)));

  const auto changedLinkedGroups = collectGroupsOrContainers(
    kdl::vec_concat(kdl::map_keys(nodesToAdd), kdl::map_keys(nodesToRemove)));

  if (!checkLinkedGroupsToUpdate(changedLinkedGroups))
  {
    return false;
  }

  auto transaction = Transaction{*this, "Reparent Objects"};

  // This handles two main cases:
  // - creating brushes in a hidden layer, and then grouping / ungrouping them keeps
  // them visible
  // - creating brushes in a hidden layer, then moving them to a hidden layer, should
  // downgrade them
  //   to inherited and hide them
  for (auto& [newParent, nodes] : nodesToAdd)
  {
    auto* newParentLayer = mdl::findContainingLayer(newParent);

    const auto nodesToDowngrade = mdl::collectNodesAndDescendants(
      nodes,
      [&](mdl::Object* node) { return node->containingLayer() != newParentLayer; });

    downgradeUnlockedToInherit(nodesToDowngrade);
    downgradeShownToInherit(nodesToDowngrade);
  }

  // Reset link IDs of nodes being reparented, but don't recurse into nested groups
  executeAndStore(std::make_unique<SetLinkIdsCommand>(
    "Set Link ID", setLinkIdsForReparentingNodes(nodesToAdd)));

  const auto result =
    executeAndStore(ReparentNodesCommand::reparent(nodesToAdd, nodesToRemove));
  if (!result->success())
  {
    transaction.cancel();
    return false;
  }

  setHasPendingChanges(changedLinkedGroups, true);

  auto removableNodes = collectRemovableParents(nodesToRemove);
  while (!removableNodes.empty())
  {
    setHasPendingChanges(
      collectContainingGroups(kdl::vec_flatten(kdl::map_values(removableNodes))), true);

    closeRemovedGroups(removableNodes);
    executeAndStore(AddRemoveNodesCommand::remove(removableNodes));

    removableNodes = collectRemovableParents(removableNodes);
  }

  return transaction.commit();
}

bool MapDocument::checkReparenting(
  const std::map<mdl::Node*, std::vector<mdl::Node*>>& nodesToAdd) const
{
  for (const auto& [newParent, children] : nodesToAdd)
  {
    if (!newParent->canAddChildren(std::begin(children), std::end(children)))
    {
      return false;
    }
  }
  return true;
}

void MapDocument::deleteObjects()
{
  const auto nodes = m_selectedNodes.nodes();

  auto transaction = Transaction{*this, "Delete Objects"};
  deselectAll();
  removeNodes(nodes);
  assertResult(transaction.commit());
}

namespace
{

/**
 * Returns whether, for UI reasons, duplicating the given node should also cause its
 * parent to be duplicated.
 *
 * Applies when duplicating a brush inside a brush entity.
 */
bool shouldCloneParentWhenCloningNode(const mdl::Node* node)
{
  return node->parent()->accept(kdl::overload(
    [](const mdl::WorldNode*) { return false; },
    [](const mdl::LayerNode*) { return false; },
    [](const mdl::GroupNode*) { return false; },
    [&](const mdl::EntityNode*) { return true; },
    [](const mdl::BrushNode*) { return false; },
    [](const mdl::PatchNode*) { return false; }));
}

void resetLinkIdsOfNonGroupedNodes(
  const std::map<mdl::Node*, std::vector<mdl::Node*>>& nodes)
{
  for (const auto& [parent, children] : nodes)
  {
    mdl::Node::visitAll(
      children,
      kdl::overload(
        [](const mdl::WorldNode*) {},
        [](const mdl::LayerNode*) {},
        [](const mdl::GroupNode*) {},
        [](auto&& thisLambda, mdl::EntityNode* entityNode) {
          entityNode->setLinkId(generateUuid());
          entityNode->visitChildren(thisLambda);
        },
        [](mdl::BrushNode* brushNode) { brushNode->setLinkId(generateUuid()); },
        [](mdl::PatchNode* patchNode) { patchNode->setLinkId(generateUuid()); }));
  }
}

} // namespace

void MapDocument::duplicateObjects()
{
  auto nodesToAdd = std::map<mdl::Node*, std::vector<mdl::Node*>>{};
  auto nodesToSelect = std::vector<mdl::Node*>{};
  auto newParentMap = std::map<mdl::Node*, mdl::Node*>{};

  for (auto* original : selectedNodes().nodes())
  {
    auto* suggestedParent = parentForNodes({original});
    auto* clone = original->cloneRecursively(m_worldBounds);

    if (shouldCloneParentWhenCloningNode(original))
    {
      // e.g. original is a brush in a brush entity, so we need to clone the entity
      // (parent) see if the parent was already cloned and if not, clone it and store it
      auto* originalParent = original->parent();
      auto* newParent = static_cast<mdl::Node*>(nullptr);
      const auto it = newParentMap.find(originalParent);
      if (it != std::end(newParentMap))
      {
        // parent was already cloned
        newParent = it->second;
      }
      else
      {
        // parent was not cloned yet
        newParent = originalParent->clone(m_worldBounds);
        newParentMap.insert({originalParent, newParent});
        nodesToAdd[suggestedParent].push_back(newParent);
      }

      // the hierarchy will look like (parent -> child): suggestedParent -> newParent ->
      // clone
      newParent->addChild(clone);
    }
    else
    {
      nodesToAdd[suggestedParent].push_back(clone);
    }

    nodesToSelect.push_back(clone);
  }

  resetLinkIdsOfNonGroupedNodes(nodesToAdd);
  copyAndSetLinkIds(nodesToAdd, *m_world, *this);

  {
    auto transaction = Transaction{*this, "Duplicate Objects"};
    deselectAll();

    if (addNodes(nodesToAdd).empty())
    {
      transaction.cancel();
      return;
    }

    selectNodes(nodesToSelect);
    if (!transaction.commit())
    {
      return;
    }
  }

  if (m_viewEffectsService)
  {
    m_viewEffectsService->flashSelection();
  }
  m_repeatStack->push([&]() { duplicateObjects(); });
}

mdl::EntityNode* MapDocument::createPointEntity(
  const mdl::EntityDefinition& definition, const vm::vec3d& delta)
{
  ensure(
    getType(definition) == mdl::EntityDefinitionType::Point,
    "definition is a point entity definition");

  auto entity = mdl::Entity{{{mdl::EntityPropertyKeys::Classname, definition.name}}};

  if (m_world->entityPropertyConfig().setDefaultProperties)
  {
    mdl::setDefaultProperties(definition, entity, mdl::SetDefaultPropertyMode::SetAll);
  }

  auto* entityNode = new mdl::EntityNode{std::move(entity)};

  auto transaction = Transaction{*this, "Create " + definition.name};
  deselectAll();
  if (addNodes({{parentForNodes(), {entityNode}}}).empty())
  {
    transaction.cancel();
    return nullptr;
  }
  selectNodes({entityNode});
  if (!translateObjects(delta))
  {
    transaction.cancel();
    return nullptr;
  }

  if (!transaction.commit())
  {
    return nullptr;
  }

  return entityNode;
}

mdl::EntityNode* MapDocument::createBrushEntity(const mdl::EntityDefinition& definition)
{
  ensure(
    getType(definition) == mdl::EntityDefinitionType::Brush,
    "definition is a brush entity definition");

  const auto brushes = selectedNodes().brushes();
  assert(!brushes.empty());

  // if all brushes belong to the same entity, and that entity is not worldspawn, copy
  // its properties
  auto entity =
    (brushes.front()->entity() != m_world.get()
     && std::all_of(
       std::next(brushes.begin()),
       brushes.end(),
       [&](const auto* brush) { return brush->entity() == brushes.front()->entity(); }))
      ? brushes.front()->entity()->entity()
      : mdl::Entity{};

  entity.addOrUpdateProperty(mdl::EntityPropertyKeys::Classname, definition.name);

  if (m_world->entityPropertyConfig().setDefaultProperties)
  {
    mdl::setDefaultProperties(definition, entity, mdl::SetDefaultPropertyMode::SetAll);
  }

  auto* entityNode = new mdl::EntityNode{std::move(entity)};

  const auto nodes = kdl::vec_static_cast<mdl::Node*>(brushes);

  auto transaction = Transaction{*this, "Create " + definition.name};
  deselectAll();
  if (addNodes({{parentForNodes(), {entityNode}}}).empty())
  {
    transaction.cancel();
    return nullptr;
  }
  if (!reparentNodes({{entityNode, nodes}}))
  {
    transaction.cancel();
    return nullptr;
  }
  selectNodes(nodes);

  if (!transaction.commit())
  {
    return nullptr;
  }

  return entityNode;
}

static std::vector<mdl::Node*> collectGroupableNodes(
  const std::vector<mdl::Node*>& selectedNodes, const mdl::EntityNodeBase* world)
{
  std::vector<mdl::Node*> result;
  const auto addNode = [&](auto&& thisLambda, auto* node) {
    if (node->entity() == world)
    {
      result.push_back(node);
    }
    else
    {
      node->visitParent(thisLambda);
    }
  };

  mdl::Node::visitAll(
    selectedNodes,
    kdl::overload(
      [](mdl::WorldNode*) {},
      [](mdl::LayerNode*) {},
      [&](mdl::GroupNode* group) { result.push_back(group); },
      [&](mdl::EntityNode* entity) { result.push_back(entity); },
      [&](auto&& thisLambda, mdl::BrushNode* brush) { addNode(thisLambda, brush); },
      [&](auto&& thisLambda, mdl::PatchNode* patch) { addNode(thisLambda, patch); }));
  return kdl::col_stable_remove_duplicates(std::move(result));
}

mdl::GroupNode* MapDocument::groupSelection(const std::string& name)
{
  if (!hasSelectedNodes())
  {
    return nullptr;
  }

  const auto nodes = collectGroupableNodes(selectedNodes().nodes(), world());
  if (nodes.empty())
  {
    return nullptr;
  }

  auto* group = new mdl::GroupNode{mdl::Group{name}};

  auto transaction = Transaction{*this, "Group Selected Objects"};
  deselectAll();
  if (
    addNodes({{parentForNodes(nodes), {group}}}).empty()
    || !reparentNodes({{group, nodes}}))
  {
    transaction.cancel();
    return nullptr;
  }
  selectNodes({group});

  if (!transaction.commit())
  {
    return nullptr;
  }

  return group;
}

void MapDocument::mergeSelectedGroupsWithGroup(mdl::GroupNode* group)
{
  if (!hasSelectedNodes() || !m_selectedNodes.hasOnlyGroups())
  {
    return;
  }

  const auto groupsToMerge = m_selectedNodes.groups();

  auto transaction = Transaction{*this, "Merge Groups"};
  deselectAll();
  for (auto groupToMerge : groupsToMerge)
  {
    if (groupToMerge != group)
    {
      const auto children = groupToMerge->children();
      if (!reparentNodes({{group, children}}))
      {
        transaction.cancel();
        return;
      }
    }
  }
  selectNodes({group});

  transaction.commit();
}

void MapDocument::ungroupSelection()
{
  if (!hasSelectedNodes())
  {
    return;
  }

  auto transaction = Transaction{*this, "Ungroup"};
  separateSelectedLinkedGroups(false);

  const auto selectedNodes = m_selectedNodes.nodes();
  auto nodesToReselect = std::vector<mdl::Node*>{};

  deselectAll();

  auto success = true;
  mdl::Node::visitAll(
    selectedNodes,
    kdl::overload(
      [](mdl::WorldNode*) {},
      [](mdl::LayerNode*) {},
      [&](mdl::GroupNode* group) {
        auto* parent = group->parent();
        const auto children = group->children();
        success = success && reparentNodes({{parent, children}});
        nodesToReselect = kdl::vec_concat(std::move(nodesToReselect), children);
      },
      [&](mdl::EntityNode* entity) { nodesToReselect.push_back(entity); },
      [&](mdl::BrushNode* brush) { nodesToReselect.push_back(brush); },
      [&](mdl::PatchNode* patch) { nodesToReselect.push_back(patch); }));

  if (!success)
  {
    transaction.cancel();
    return;
  }

  selectNodes(nodesToReselect);
  transaction.commit();
}

void MapDocument::renameGroups(const std::string& name)
{
  if (hasSelectedNodes() && m_selectedNodes.hasOnlyGroups())
  {
    const auto commandName =
      kdl::str_plural("Rename ", m_selectedNodes.groupCount(), "Group", "Groups");
    applyAndSwap(
      *this,
      commandName,
      m_selectedNodes.groups(),
      {},
      kdl::overload(
        [](mdl::Layer&) { return true; },
        [&](mdl::Group& group) {
          group.setName(name);
          return true;
        },
        [](mdl::Entity&) { return true; },
        [](mdl::Brush&) { return true; },
        [](mdl::BezierPatch&) { return true; }));
  }
}

void MapDocument::openGroup(mdl::GroupNode* group)
{
  auto transaction = Transaction{*this, "Open Group"};

  deselectAll();
  auto* previousGroup = m_editorContext->currentGroup();
  if (previousGroup == nullptr)
  {
    lock(std::vector<mdl::Node*>{m_world.get()});
  }
  else
  {
    resetLock(std::vector<mdl::Node*>{previousGroup});
  }
  unlock(std::vector<mdl::Node*>{group});
  executeAndStore(CurrentGroupCommand::push(group));

  transaction.commit();
}

void MapDocument::closeGroup()
{
  auto transaction = Transaction{*this, "Open Group"};

  deselectAll();
  auto* previousGroup = m_editorContext->currentGroup();
  resetLock(std::vector<mdl::Node*>{previousGroup});
  executeAndStore(CurrentGroupCommand::pop());

  auto* currentGroup = m_editorContext->currentGroup();
  if (currentGroup != nullptr)
  {
    unlock(std::vector<mdl::Node*>{currentGroup});
  }
  else
  {
    unlock(std::vector<mdl::Node*>{m_world.get()});
  }

  transaction.commit();
}

mdl::GroupNode* MapDocument::createLinkedDuplicate()
{
  if (!canCreateLinkedDuplicate())
  {
    return nullptr;
  }

  auto transaction = Transaction{*this, "Create Linked Duplicate"};

  auto* groupNode = m_selectedNodes.groups().front();
  auto* groupNodeClone =
    static_cast<mdl::GroupNode*>(groupNode->cloneRecursively(m_worldBounds));
  auto* suggestedParent = parentForNodes({groupNode});
  if (addNodes({{suggestedParent, {groupNodeClone}}}).empty())
  {
    transaction.cancel();
    return nullptr;
  }

  if (!transaction.commit())
  {
    return nullptr;
  }

  return groupNodeClone;
}

bool MapDocument::canCreateLinkedDuplicate() const
{
  return m_selectedNodes.hasOnlyGroups() && m_selectedNodes.groupCount() == 1u;
}

void MapDocument::selectLinkedGroups()
{
  if (!canSelectLinkedGroups())
  {
    return;
  }

  const auto linkIdsToSelect = kdl::vec_sort_and_remove_duplicates(kdl::vec_transform(
    m_selectedNodes.groups(), [](const auto* groupNode) { return groupNode->linkId(); }));
  const auto groupNodesToSelect =
    kdl::vec_flatten(kdl::vec_transform(linkIdsToSelect, [&](const auto& linkId) {
      return mdl::collectNodesWithLinkId({m_world.get()}, linkId);
    }));

  auto transaction = Transaction{*this, "Select Linked Groups"};
  deselectAll();
  selectNodes(groupNodesToSelect);
  transaction.commit();
}

bool MapDocument::canSelectLinkedGroups() const
{
  if (!m_selectedNodes.hasOnlyGroups())
  {
    return false;
  }

  const auto allLinkIds = kdl::vec_sort(kdl::vec_transform(
    mdl::collectGroups({m_world.get()}),
    [](const auto& groupNode) { return groupNode->linkId(); }));

  return kdl::all_of(m_selectedNodes.groups(), [&](const auto* groupNode) {
    const auto [iBegin, iEnd] =
      std::equal_range(allLinkIds.begin(), allLinkIds.end(), groupNode->linkId());
    return std::distance(iBegin, iEnd) > 1;
  });
}

void MapDocument::linkGroups(const std::vector<mdl::GroupNode*>& groupNodes)
{
  if (groupNodes.size() > 1)
  {
    const auto& sourceGroupNode = *groupNodes.front();
    const auto targetGroupNodes =
      kdl::vec_slice_suffix(groupNodes, groupNodes.size() - 1);
    mdl::copyAndReturnLinkIds(sourceGroupNode, targetGroupNodes)
      | kdl::transform([&](auto linkIds) {
          auto linkIdVector = kdl::vec_transform(
            std::move(linkIds), [](auto pair) -> std::tuple<mdl::Node*, std::string> {
              return {std::move(pair)};
            });

          executeAndStore(
            std::make_unique<SetLinkIdsCommand>("Set Link ID", std::move(linkIdVector)));
        })
      | kdl::transform_error(
        [&](auto e) { error() << "Could not link groups: " << e.msg; });
    ;
  }
}

namespace
{

std::vector<mdl::Node*> collectNodesToUnlink(
  const std::vector<mdl::GroupNode*>& groupNodes)
{
  auto result = std::vector<mdl::Node*>{};
  for (auto* groupNode : groupNodes)
  {
    result.push_back(groupNode);
    groupNode->visitChildren(kdl::overload(
      [](const mdl::WorldNode*) {},
      [](const mdl::LayerNode*) {},
      [](const mdl::GroupNode*) {},
      [&](mdl::EntityNode* entityNode) { result.push_back(entityNode); },
      [&](mdl::BrushNode* brushNode) { result.push_back(brushNode); },
      [&](mdl::PatchNode* patchNode) { result.push_back(patchNode); }));
  }
  return result;
}

} // namespace

void MapDocument::unlinkGroups(const std::vector<mdl::GroupNode*>& groupNodes)
{
  const auto nodesToUnlink = collectNodesToUnlink(groupNodes);

  auto linkIds = kdl::vec_transform(
    nodesToUnlink, [](auto* node) -> std::tuple<mdl::Node*, std::string> {
      return {node, generateUuid()};
    });

  executeAndStore(
    std::make_unique<SetLinkIdsCommand>("Reset Link ID", std::move(linkIds)));
}

void MapDocument::separateLinkedGroups()
{
  auto transaction = Transaction{*this, "Separate Linked Groups"};
  separateSelectedLinkedGroups(true);
  transaction.commit();
}

bool MapDocument::canSeparateLinkedGroups() const
{
  return kdl::any_of(m_selectedNodes.groups(), [&](const auto* groupNode) {
    const auto linkedGroups =
      mdl::collectNodesWithLinkId({m_world.get()}, groupNode->linkId());
    return linkedGroups.size() > 1u
           && kdl::any_of(linkedGroups, [](const auto* linkedGroupNode) {
                return !linkedGroupNode->selected();
              });
  });
}

bool MapDocument::canUpdateLinkedGroups(const std::vector<mdl::Node*>& nodes) const
{
  if (nodes.empty())
  {
    return false;
  }

  const auto changedLinkedGroups = collectContainingGroups(nodes);
  return checkLinkedGroupsToUpdate(changedLinkedGroups);
}

void MapDocument::setHasPendingChanges(
  const std::vector<mdl::GroupNode*>& groupNodes, const bool hasPendingChanges)
{
  for (auto* groupNode : groupNodes)
  {
    groupNode->setHasPendingChanges(hasPendingChanges);
  }
}

static std::vector<mdl::GroupNode*> collectGroupsWithPendingChanges(mdl::Node& node)
{
  auto result = std::vector<mdl::GroupNode*>{};

  node.accept(kdl::overload(
    [](auto&& thisLambda, const mdl::WorldNode* worldNode) {
      worldNode->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, const mdl::LayerNode* layerNode) {
      layerNode->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, mdl::GroupNode* groupNode) {
      if (groupNode->hasPendingChanges())
      {
        result.push_back(groupNode);
      }
      groupNode->visitChildren(thisLambda);
    },
    [](const mdl::EntityNode*) {},
    [](const mdl::BrushNode*) {},
    [](const mdl::PatchNode*) {}));

  return result;
}

bool MapDocument::updateLinkedGroups()
{
  if (isCurrentDocumentStateObservable())
  {
    if (const auto allChangedLinkedGroups = collectGroupsWithPendingChanges(*m_world);
        !allChangedLinkedGroups.empty())
    {
      setHasPendingChanges(allChangedLinkedGroups, false);

      auto command = std::make_unique<UpdateLinkedGroupsCommand>(allChangedLinkedGroups);
      const auto result = executeAndStore(std::move(command));
      return result->success();
    }
  }

  return true;
}

void MapDocument::separateSelectedLinkedGroups(const bool relinkGroups)
{
  const auto selectedLinkIds = kdl::vec_sort_and_remove_duplicates(kdl::vec_transform(
    m_selectedNodes.groups(), [](const auto* groupNode) { return groupNode->linkId(); }));

  auto groupsToUnlink = std::vector<mdl::GroupNode*>{};
  auto groupsToRelink = std::vector<std::vector<mdl::GroupNode*>>{};

  for (const auto& linkedGroupId : selectedLinkIds)
  {
    auto linkedGroups = mdl::collectGroupsWithLinkId({m_world.get()}, linkedGroupId);

    // partition the linked groups into selected and unselected ones
    const auto it = std::partition(
      std::begin(linkedGroups), std::end(linkedGroups), [](const auto* linkedGroupNode) {
        return linkedGroupNode->selected();
      });

    auto selectedLinkedGroups =
      std::vector<mdl::GroupNode*>(std::begin(linkedGroups), it);

    assert(!selectedLinkedGroups.empty());
    if (linkedGroups.size() - selectedLinkedGroups.size() > 0)
    {
      if (relinkGroups)
      {
        groupsToRelink.push_back(selectedLinkedGroups);
      }
      groupsToUnlink =
        kdl::vec_concat(std::move(groupsToUnlink), std::move(selectedLinkedGroups));
    }
    else if (selectedLinkedGroups.size() > 1 && !relinkGroups)
    {
      // all members of a link group are being separated, and we don't want to relink
      // them, so we need to reset their linked group IDs
      groupsToUnlink =
        kdl::vec_concat(std::move(groupsToUnlink), std::move(selectedLinkedGroups));
    }
  }

  const auto changedLinkedGroups = kdl::vec_sort_and_remove_duplicates(kdl::vec_concat(
    collectContainingGroups(groupsToUnlink),
    collectContainingGroups(kdl::vec_flatten(groupsToRelink))));

  if (checkLinkedGroupsToUpdate(changedLinkedGroups))
  {
    auto transaction = Transaction{*this, "Separate Selected Linked Groups"};

    unlinkGroups(groupsToUnlink);
    for (const auto& groupNodes : groupsToRelink)
    {
      linkGroups(groupNodes);
    }

    setHasPendingChanges(changedLinkedGroups, true);
    transaction.commit();
  }
}

void MapDocument::renameLayer(mdl::LayerNode* layerNode, const std::string& name)
{
  applyAndSwap(
    *this,
    "Rename Layer",
    std::vector<mdl::Node*>{layerNode},
    {},
    kdl::overload(
      [&](mdl::Layer& layer) {
        layer.setName(name);
        return true;
      },
      [](mdl::Group&) { return true; },
      [](mdl::Entity&) { return true; },
      [](mdl::Brush&) { return true; },
      [](mdl::BezierPatch&) { return true; }));
}

bool MapDocument::moveLayerByOne(mdl::LayerNode* layerNode, MoveDirection direction)
{
  const std::vector<mdl::LayerNode*> sorted = m_world->customLayersUserSorted();

  const auto maybeIndex = kdl::index_of(sorted, layerNode);
  if (!maybeIndex.has_value())
  {
    return false;
  }

  const int newIndex =
    static_cast<int>(*maybeIndex) + (direction == MoveDirection::Down ? 1 : -1);
  if (newIndex < 0 || newIndex >= static_cast<int>(sorted.size()))
  {
    return false;
  }

  mdl::LayerNode* neighbourNode = sorted.at(static_cast<size_t>(newIndex));
  auto layer = layerNode->layer();
  auto neighbourLayer = neighbourNode->layer();

  const int layerSortIndex = layer.sortIndex();
  const int neighbourSortIndex = neighbourLayer.sortIndex();

  // Swap the sort indices of `layer` and `neighbour`
  layer.setSortIndex(neighbourSortIndex);
  neighbourLayer.setSortIndex(layerSortIndex);

  swapNodeContents(
    "Swap Layer Positions",
    {{layerNode, mdl::NodeContents(std::move(layer))},
     {neighbourNode, mdl::NodeContents(std::move(neighbourLayer))}},
    {});

  return true;
}

void MapDocument::moveLayer(mdl::LayerNode* layer, const int offset)
{
  ensure(layer != m_world->defaultLayer(), "attempted to move default layer");

  auto transaction = Transaction{*this, "Move Layer"};

  const auto direction = (offset > 0) ? MoveDirection::Down : MoveDirection::Up;
  for (int i = 0; i < std::abs(offset); ++i)
  {
    if (!moveLayerByOne(layer, direction))
    {
      break;
    }
  }

  transaction.commit();
}

bool MapDocument::canMoveLayer(mdl::LayerNode* layer, const int offset) const
{
  ensure(layer != nullptr, "null layer");

  mdl::WorldNode* world = this->world();
  if (layer == world->defaultLayer())
  {
    return false;
  }

  const std::vector<mdl::LayerNode*> sorted = world->customLayersUserSorted();
  const auto maybeIndex = kdl::index_of(sorted, layer);
  if (!maybeIndex.has_value())
  {
    return false;
  }

  const int newIndex = static_cast<int>(*maybeIndex) + offset;
  return (newIndex >= 0 && newIndex < static_cast<int>(sorted.size()));
}

void MapDocument::moveSelectionToLayer(mdl::LayerNode* layer)
{
  const auto& selectedNodes = this->selectedNodes().nodes();

  auto nodesToMove = std::vector<mdl::Node*>{};
  auto nodesToSelect = std::vector<mdl::Node*>{};

  const auto addBrushOrPatchNode = [&](auto* node) {
    assert(node->selected());

    if (!node->containedInGroup())
    {
      auto* entity = node->entity();
      if (entity == m_world.get())
      {
        nodesToMove.push_back(node);
        nodesToSelect.push_back(node);
      }
      else
      {
        if (!kdl::vec_contains(nodesToMove, entity))
        {
          nodesToMove.push_back(entity);
          nodesToSelect = kdl::vec_concat(std::move(nodesToSelect), entity->children());
        }
      }
    }
  };

  for (auto* node : selectedNodes)
  {
    node->accept(kdl::overload(
      [](mdl::WorldNode*) {},
      [](mdl::LayerNode*) {},
      [&](mdl::GroupNode* group) {
        assert(group->selected());

        if (!group->containedInGroup())
        {
          nodesToMove.push_back(group);
          nodesToSelect.push_back(group);
        }
      },
      [&](mdl::EntityNode* entity) {
        assert(entity->selected());

        if (!entity->containedInGroup())
        {
          nodesToMove.push_back(entity);
          nodesToSelect.push_back(entity);
        }
      },
      [&](mdl::BrushNode* brush) { addBrushOrPatchNode(brush); },
      [&](mdl::PatchNode* patch) { addBrushOrPatchNode(patch); }));
  }

  if (!nodesToMove.empty())
  {
    auto transaction = Transaction{*this, "Move Nodes to " + layer->name()};
    deselectAll();
    if (!reparentNodes({{layer, nodesToMove}}))
    {
      transaction.cancel();
      return;
    }
    if (!layer->hidden() && !layer->locked())
    {
      selectNodes(nodesToSelect);
    }
    transaction.commit();
  }
}

bool MapDocument::canMoveSelectionToLayer(mdl::LayerNode* layer) const
{
  ensure(layer != nullptr, "null layer");
  const auto& nodes = selectedNodes().nodes();

  const bool isAnyNodeInGroup =
    std::any_of(std::begin(nodes), std::end(nodes), [&](auto* node) {
      return mdl::findContainingGroup(node) != nullptr;
    });
  const bool isAnyNodeInOtherLayer =
    std::any_of(std::begin(nodes), std::end(nodes), [&](auto* node) {
      return mdl::findContainingLayer(node) != layer;
    });

  return !nodes.empty() && !isAnyNodeInGroup && isAnyNodeInOtherLayer;
}

void MapDocument::hideLayers(const std::vector<mdl::LayerNode*>& layers)
{
  auto transaction = Transaction{*this, "Hide Layers"};
  hide(std::vector<mdl::Node*>{std::begin(layers), std::end(layers)});
  transaction.commit();
}

bool MapDocument::canHideLayers(const std::vector<mdl::LayerNode*>& layers) const
{
  return std::any_of(
    layers.begin(), layers.end(), [](const auto* layer) { return layer->visible(); });
}

void MapDocument::isolateLayers(const std::vector<mdl::LayerNode*>& layers)
{
  const auto allLayers = world()->allLayers();

  auto transaction = Transaction{*this, "Isolate Layers"};
  hide(std::vector<mdl::Node*>{std::begin(allLayers), std::end(allLayers)});
  show(std::vector<mdl::Node*>{std::begin(layers), std::end(layers)});
  transaction.commit();
}

bool MapDocument::canIsolateLayers(const std::vector<mdl::LayerNode*>& layers) const
{
  const auto allLayers = m_world->allLayers();
  return std::any_of(allLayers.begin(), allLayers.end(), [&](const auto* layer) {
    return kdl::vec_contains(layers, layer) != layer->visible();
  });
}

void MapDocument::isolate()
{
  auto selectedNodes = std::vector<mdl::Node*>{};
  auto unselectedNodes = std::vector<mdl::Node*>{};

  const auto collectNode = [&](auto* node) {
    if (node->transitivelySelected() || node->descendantSelected())
    {
      selectedNodes.push_back(node);
    }
    else
    {
      unselectedNodes.push_back(node);
    }
  };

  m_world->accept(kdl::overload(
    [](auto&& thisLambda, mdl::WorldNode* world) { world->visitChildren(thisLambda); },
    [](auto&& thisLambda, mdl::LayerNode* layer) { layer->visitChildren(thisLambda); },
    [&](auto&& thisLambda, mdl::GroupNode* group) {
      collectNode(group);
      group->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, mdl::EntityNode* entity) {
      collectNode(entity);
      entity->visitChildren(thisLambda);
    },
    [&](mdl::BrushNode* brush) { collectNode(brush); },
    [&](mdl::PatchNode* patch) { collectNode(patch); }));

  auto transaction = Transaction{*this, "Isolate Objects"};
  executeAndStore(SetVisibilityCommand::hide(unselectedNodes));
  executeAndStore(SetVisibilityCommand::show(selectedNodes));
  transaction.commit();
}

void MapDocument::setOmitLayerFromExport(
  mdl::LayerNode* layerNode, const bool omitFromExport)
{
  const auto commandName =
    omitFromExport ? "Omit Layer from Export" : "Include Layer in Export";

  auto layer = layerNode->layer();
  layer.setOmitFromExport(omitFromExport);
  swapNodeContents(commandName, {{layerNode, mdl::NodeContents(std::move(layer))}}, {});
}

void MapDocument::selectAllInLayers(const std::vector<mdl::LayerNode*>& layers)
{
  const auto nodes = mdl::collectSelectableNodes(
    kdl::vec_static_cast<mdl::Node*>(layers), editorContext());

  deselectAll();
  selectNodes(nodes);
}

bool MapDocument::canSelectAllInLayers(
  const std::vector<mdl::LayerNode*>& /* layers */) const
{
  return editorContext().canChangeSelection();
}

void MapDocument::hide(const std::vector<mdl::Node*> nodes)
{
  auto transaction = Transaction{*this, "Hide Objects"};

  // Deselect any selected nodes inside `nodes`
  deselectNodes(mdl::collectSelectedNodes(nodes));

  // Reset visibility of any forced shown children of `nodes`
  downgradeShownToInherit(mdl::collectDescendants(nodes));

  executeAndStore(SetVisibilityCommand::hide(nodes));
  transaction.commit();
}

void MapDocument::hideSelection()
{
  hide(m_selectedNodes.nodes());
}

void MapDocument::show(const std::vector<mdl::Node*>& nodes)
{
  executeAndStore(SetVisibilityCommand::show(nodes));
}

void MapDocument::showAll()
{
  resetVisibility(mdl::collectDescendants(m_world->allLayers()));
}

void MapDocument::ensureVisible(const std::vector<mdl::Node*>& nodes)
{
  executeAndStore(SetVisibilityCommand::ensureVisible(nodes));
}

void MapDocument::resetVisibility(const std::vector<mdl::Node*>& nodes)
{
  executeAndStore(SetVisibilityCommand::reset(nodes));
}

void MapDocument::lock(const std::vector<mdl::Node*>& nodes)
{
  auto transaction = Transaction{*this, "Lock Objects"};

  // Deselect any selected nodes or faces inside `nodes`
  deselectNodes(mdl::collectSelectedNodes(nodes));
  deselectBrushFaces(mdl::collectSelectedBrushFaces(nodes));

  // Reset lock state of any forced unlocked children of `nodes`
  downgradeUnlockedToInherit(mdl::collectDescendants(nodes));

  executeAndStore(SetLockStateCommand::lock(nodes));
  transaction.commit();
}

void MapDocument::unlock(const std::vector<mdl::Node*>& nodes)
{
  executeAndStore(SetLockStateCommand::unlock(nodes));
}

/**
 * Unlocks only those nodes from the given list whose lock state resolves to "locked"
 */
void MapDocument::ensureUnlocked(const std::vector<mdl::Node*>& nodes)
{
  auto nodesToUnlock = std::vector<mdl::Node*>{};
  std::copy_if(
    nodes.begin(), nodes.end(), std::back_inserter(nodesToUnlock), [](auto* node) {
      return node->locked();
    });
  unlock(nodesToUnlock);
}

void MapDocument::resetLock(const std::vector<mdl::Node*>& nodes)
{
  executeAndStore(SetLockStateCommand::reset(nodes));
}

/**
 * This is called to clear the forced Visibility_Shown that was set on newly created
 * nodes so they could be visible if created in a hidden layer
 */
void MapDocument::downgradeShownToInherit(const std::vector<mdl::Node*>& nodes)
{
  auto nodesToReset = std::vector<mdl::Node*>{};
  std::copy_if(
    nodes.begin(), nodes.end(), std::back_inserter(nodesToReset), [](auto* node) {
      return node->visibilityState() == mdl::VisibilityState::Shown;
    });
  resetVisibility(nodesToReset);
}

/**
 * See downgradeShownToInherit
 */
void MapDocument::downgradeUnlockedToInherit(const std::vector<mdl::Node*>& nodes)
{
  auto nodesToReset = std::vector<mdl::Node*>{};
  std::copy_if(
    nodes.begin(), nodes.end(), std::back_inserter(nodesToReset), [](auto* node) {
      return node->lockState() == mdl::LockState::Unlocked;
    });
  resetLock(nodesToReset);
}

bool MapDocument::swapNodeContents(
  const std::string& commandName,
  std::vector<std::pair<mdl::Node*, mdl::NodeContents>> nodesToSwap,
  std::vector<mdl::GroupNode*> changedLinkedGroups)
{

  if (!checkLinkedGroupsToUpdate(changedLinkedGroups))
  {
    return false;
  }

  auto transaction = Transaction{*this};
  const auto result = executeAndStore(
    std::make_unique<SwapNodeContentsCommand>(commandName, std::move(nodesToSwap)));

  if (!result->success())
  {
    transaction.cancel();
    return false;
  }

  setHasPendingChanges(changedLinkedGroups, true);
  return transaction.commit();
}

bool MapDocument::swapNodeContents(
  const std::string& commandName,
  std::vector<std::pair<mdl::Node*, mdl::NodeContents>> nodesToSwap)
{
  auto changedLinkedGroups = collectContainingGroups(
    kdl::vec_transform(nodesToSwap, [](const auto& p) { return p.first; }));

  return swapNodeContents(
    commandName, std::move(nodesToSwap), std::move(changedLinkedGroups));
}

bool MapDocument::transformObjects(
  const std::string& commandName, const vm::mat4x4d& transformation)
{
  auto nodesToTransform = std::vector<mdl::Node*>{};
  auto entitiesToTransform = std::unordered_map<mdl::EntityNodeBase*, size_t>{};

  for (auto* node : m_selectedNodes)
  {
    node->accept(kdl::overload(
      [&](auto&& thisLambda, mdl::WorldNode* worldNode) {
        worldNode->visitChildren(thisLambda);
      },
      [&](auto&& thisLambda, mdl::LayerNode* layerNode) {
        layerNode->visitChildren(thisLambda);
      },
      [&](auto&& thisLambda, mdl::GroupNode* groupNode) {
        nodesToTransform.push_back(groupNode);
        groupNode->visitChildren(thisLambda);
      },
      [&](auto&& thisLambda, mdl::EntityNode* entityNode) {
        if (!entityNode->hasChildren())
        {
          nodesToTransform.push_back(entityNode);
        }
        else
        {
          entityNode->visitChildren(thisLambda);
        }
      },
      [&](mdl::BrushNode* brushNode) {
        nodesToTransform.push_back(brushNode);
        entitiesToTransform[brushNode->entity()]++;
      },
      [&](mdl::PatchNode* patchNode) {
        nodesToTransform.push_back(patchNode);
        entitiesToTransform[patchNode->entity()]++;
      }));
  }

  // add entities if all of their children are transformed
  for (const auto& [entityNode, transformedChildCount] : entitiesToTransform)
  {
    if (
      transformedChildCount == entityNode->childCount()
      && !mdl::isWorldspawn(entityNode->entity().classname()))
    {
      nodesToTransform.push_back(entityNode);
    }
  }

  using TransformResult = Result<std::pair<mdl::Node*, mdl::NodeContents>>;

  const auto alignmentLock = pref(Preferences::AlignmentLock);
  const auto updateAngleProperty =
    m_world->entityPropertyConfig().updateAnglePropertyAfterTransform;

  auto tasks =
    nodesToTransform | std::views::transform([&](auto& node) {
      return std::function{[&]() {
        return node->accept(kdl::overload(
          [&](mdl::WorldNode*) -> TransformResult {
            ensure(false, "Unexpected world node");
          },
          [&](mdl::LayerNode*) -> TransformResult {
            ensure(false, "Unexpected layer node");
          },
          [&](mdl::GroupNode* groupNode) -> TransformResult {
            auto group = groupNode->group();
            group.transform(transformation);
            return std::make_pair(groupNode, mdl::NodeContents{std::move(group)});
          },
          [&](mdl::EntityNode* entityNode) -> TransformResult {
            auto entity = entityNode->entity();
            entity.transform(transformation, updateAngleProperty);
            return std::make_pair(entityNode, mdl::NodeContents{std::move(entity)});
          },
          [&](mdl::BrushNode* brushNode) -> TransformResult {
            const auto* containingGroup = brushNode->containingGroup();
            const bool lockAlignment =
            alignmentLock
            || (containingGroup && containingGroup->closed() && mdl::collectLinkedNodes({m_world.get()}, *brushNode).size() > 1);

            auto brush = brushNode->brush();
            return brush.transform(m_worldBounds, transformation, lockAlignment)
                   | kdl::and_then([&]() -> TransformResult {
                       return std::make_pair(
                         brushNode, mdl::NodeContents{std::move(brush)});
                     });
          },
          [&](mdl::PatchNode* patchNode) -> TransformResult {
            auto patch = patchNode->patch();
            patch.transform(transformation);
            return std::make_pair(patchNode, mdl::NodeContents{std::move(patch)});
          }));
      }};
    });

  return m_taskManager.run_tasks_and_wait(tasks) | kdl::fold
         | kdl::and_then([&](auto nodesToUpdate) -> Result<bool> {
             const auto success = swapNodeContents(
               commandName,
               std::move(nodesToUpdate),
               collectContainingGroups(m_selectedNodes.nodes()));

             if (success)
             {
               m_repeatStack->push([&, commandName, transformation]() {
                 transformObjects(commandName, transformation);
               });
             }
             return success;
           })
         | kdl::value_or(false);
}

bool MapDocument::translateObjects(const vm::vec3d& delta)
{
  return transformObjects("Translate Objects", vm::translation_matrix(delta));
}

bool MapDocument::rotateObjects(
  const vm::vec3d& center, const vm::vec3d& axis, const double angle)
{
  const auto transformation = vm::translation_matrix(center)
                              * vm::rotation_matrix(axis, angle)
                              * vm::translation_matrix(-center);
  return transformObjects("Rotate Objects", transformation);
}

bool MapDocument::scaleObjects(const vm::bbox3d& oldBBox, const vm::bbox3d& newBBox)
{
  const auto transformation = vm::scale_bbox_matrix(oldBBox, newBBox);
  return transformObjects("Scale Objects", transformation);
}

bool MapDocument::scaleObjects(const vm::vec3d& center, const vm::vec3d& scaleFactors)
{
  const auto transformation = vm::translation_matrix(center)
                              * vm::scaling_matrix(scaleFactors)
                              * vm::translation_matrix(-center);
  return transformObjects("Scale Objects", transformation);
}

bool MapDocument::shearObjects(
  const vm::bbox3d& box, const vm::vec3d& sideToShear, const vm::vec3d& delta)
{
  const auto transformation = vm::shear_bbox_matrix(box, sideToShear, delta);
  return transformObjects("Scale Objects", transformation);
}

bool MapDocument::flipObjects(const vm::vec3d& center, const vm::axis::type axis)
{
  const auto transformation = vm::translation_matrix(center)
                              * vm::mirror_matrix<double>(axis)
                              * vm::translation_matrix(-center);
  return transformObjects("Flip Objects", transformation);
}

bool MapDocument::createBrush(const std::vector<vm::vec3d>& points)
{
  const auto builder = mdl::BrushBuilder{
    m_world->mapFormat(), m_worldBounds, m_game->config().faceAttribsConfig.defaults};

  return builder.createBrush(points, currentMaterialName())
         | kdl::and_then([&](auto b) -> Result<void> {
             auto* brushNode = new mdl::BrushNode{std::move(b)};

             auto transaction = Transaction{*this, "Create Brush"};
             deselectAll();
             if (addNodes({{parentForNodes(), {brushNode}}}).empty())
             {
               transaction.cancel();
               return Error{"Could not add brush to document"};
             }
             selectNodes({brushNode});
             if (!transaction.commit())
             {
               return Error{"Could not add brush to document"};
             }

             return kdl::void_success;
           })
         | kdl::if_error([&](auto e) { error() << "Could not create brush: " << e.msg; })
         | kdl::is_success();
}

bool MapDocument::csgConvexMerge()
{
  if (!hasSelectedBrushFaces() && !selectedNodes().hasOnlyBrushes())
  {
    return false;
  }

  auto points = std::vector<vm::vec3d>{};

  if (hasSelectedBrushFaces())
  {
    for (const auto& handle : selectedBrushFaces())
    {
      for (const auto* vertex : handle.face().vertices())
      {
        points.push_back(vertex->position());
      }
    }
  }
  else if (selectedNodes().hasOnlyBrushes())
  {
    for (const auto* brushNode : selectedNodes().brushes())
    {
      for (const auto* vertex : brushNode->brush().vertices())
      {
        points.push_back(vertex->position());
      }
    }
  }

  auto polyhedron = mdl::Polyhedron3{std::move(points)};
  if (!polyhedron.polyhedron() || !polyhedron.closed())
  {
    return false;
  }

  const auto builder = mdl::BrushBuilder{
    m_world->mapFormat(), m_worldBounds, m_game->config().faceAttribsConfig.defaults};
  return builder.createBrush(polyhedron, currentMaterialName())
         | kdl::transform([&](auto b) {
             b.cloneFaceAttributesFrom(kdl::vec_transform(
               selectedNodes().brushes(),
               [](const auto* brushNode) { return &brushNode->brush(); }));

             // The nodelist is either empty or contains only brushes.
             const auto toRemove = selectedNodes().nodes();

             // We could be merging brushes that have different parents; use the parent
             // of the first brush.
             auto* parentNode = static_cast<mdl::Node*>(nullptr);
             if (!selectedNodes().brushes().empty())
             {
               parentNode = selectedNodes().brushes().front()->parent();
             }
             else if (!selectedBrushFaces().empty())
             {
               parentNode = selectedBrushFaces().front().node()->parent();
             }
             else
             {
               parentNode = parentForNodes();
             }

             auto* brushNode = new mdl::BrushNode{std::move(b)};

             auto transaction = Transaction{*this, "CSG Convex Merge"};
             deselectAll();
             if (addNodes({{parentNode, {brushNode}}}).empty())
             {
               transaction.cancel();
               return;
             }
             removeNodes(toRemove);
             selectNodes({brushNode});
             transaction.commit();
           })
         | kdl::if_error([&](auto e) { error() << "Could not create brush: " << e.msg; })
         | kdl::is_success();
}

bool MapDocument::csgSubtract()
{
  const auto subtrahendNodes = std::vector<mdl::BrushNode*>{selectedNodes().brushes()};
  if (subtrahendNodes.empty())
  {
    return false;
  }

  auto transaction = Transaction{*this, "CSG Subtract"};
  // Select touching, but don't delete the subtrahends yet
  selectTouching(false);

  const auto minuendNodes = std::vector<mdl::BrushNode*>{selectedNodes().brushes()};
  const auto subtrahends = kdl::vec_transform(
    subtrahendNodes, [](const auto* subtrahendNode) { return &subtrahendNode->brush(); });

  auto toAdd = std::map<mdl::Node*, std::vector<mdl::Node*>>{};
  auto toRemove =
    std::vector<mdl::Node*>{std::begin(subtrahendNodes), std::end(subtrahendNodes)};

  return kdl::vec_transform(
           minuendNodes,
           [&](auto* minuendNode) {
             const auto& minuend = minuendNode->brush();
             auto currentSubtractionResults = minuend.subtract(
               m_world->mapFormat(), m_worldBounds, currentMaterialName(), subtrahends);

             return kdl::vec_filter(
                      std::move(currentSubtractionResults),
                      [](const auto r) { return r | kdl::is_success(); })
                    | kdl::fold | kdl::transform([&](auto currentBrushes) {
                        if (!currentBrushes.empty())
                        {
                          auto resultNodes = kdl::vec_transform(
                            std::move(currentBrushes),
                            [&](auto b) { return new mdl::BrushNode{std::move(b)}; });
                          auto& toAddForParent = toAdd[minuendNode->parent()];
                          toAddForParent = kdl::vec_concat(
                            std::move(toAddForParent), std::move(resultNodes));
                        }

                        toRemove.push_back(minuendNode);
                      });
           })
         | kdl::fold | kdl::transform([&]() {
             deselectAll();
             const auto added = addNodes(toAdd);
             removeNodes(toRemove);
             selectNodes(added);

             return transaction.commit();
           })
         | kdl::transform_error([&](const auto& e) {
             error() << "Could not subtract brushes: " << e;
             transaction.cancel();
             return false;
           })
         | kdl::value();
}

bool MapDocument::csgIntersect()
{
  const auto brushes = selectedNodes().brushes();
  if (brushes.size() < 2u)
  {
    return false;
  }

  auto intersection = brushes.front()->brush();

  bool valid = true;
  for (auto it = std::next(std::begin(brushes)), end = std::end(brushes);
       it != end && valid;
       ++it)
  {
    auto* brushNode = *it;
    const auto& brush = brushNode->brush();
    valid = intersection.intersect(m_worldBounds, brush) | kdl::if_error([&](auto e) {
              error() << "Could not intersect brushes: " << e.msg;
            })
            | kdl::is_success();
  }

  const auto toRemove = std::vector<mdl::Node*>{std::begin(brushes), std::end(brushes)};

  auto transaction = Transaction{*this, "CSG Intersect"};
  deselectNodes(toRemove);

  if (valid)
  {
    auto* intersectionNode = new mdl::BrushNode{std::move(intersection)};
    if (addNodes({{parentForNodes(toRemove), {intersectionNode}}}).empty())
    {
      transaction.cancel();
      return false;
    }
    removeNodes(toRemove);
    selectNodes({intersectionNode});
  }
  else
  {
    removeNodes(toRemove);
  }

  return transaction.commit();
}

bool MapDocument::csgHollow()
{
  const auto brushNodes = selectedNodes().brushes();
  if (brushNodes.empty())
  {
    return false;
  }

  bool didHollowAnything = false;
  auto toAdd = std::map<mdl::Node*, std::vector<mdl::Node*>>{};
  auto toRemove = std::vector<mdl::Node*>{};

  for (auto* brushNode : brushNodes)
  {
    const auto& originalBrush = brushNode->brush();

    auto shrunkenBrush = originalBrush;
    shrunkenBrush.expand(m_worldBounds, -double(m_grid->actualSize()), true)
      | kdl::and_then([&]() {
          didHollowAnything = true;

          return originalBrush.subtract(
                   m_world->mapFormat(),
                   m_worldBounds,
                   currentMaterialName(),
                   shrunkenBrush)
                 | kdl::fold | kdl::transform([&](auto fragments) {
                     auto fragmentNodes =
                       kdl::vec_transform(std::move(fragments), [](auto&& b) {
                         return new mdl::BrushNode{std::forward<decltype(b)>(b)};
                       });

                     auto& toAddForParent = toAdd[brushNode->parent()];
                     toAddForParent =
                       kdl::vec_concat(std::move(toAddForParent), fragmentNodes);
                     toRemove.push_back(brushNode);
                   });
        })
      | kdl::transform_error(
        [&](const auto& e) { error() << "Could not hollow brush: " << e; });
  }

  if (!didHollowAnything)
  {
    return false;
  }

  auto transaction = Transaction{*this, "CSG Hollow"};
  deselectAll();
  const auto added = addNodes(toAdd);
  if (added.empty())
  {
    transaction.cancel();
    return false;
  }
  removeNodes(toRemove);
  selectNodes(added);

  return transaction.commit();
}

bool MapDocument::clipBrushes(
  const vm::vec3d& p1, const vm::vec3d& p2, const vm::vec3d& p3)
{
  return kdl::vec_transform(
           m_selectedNodes.brushes(),
           [&](const mdl::BrushNode* originalBrush) {
             auto clippedBrush = originalBrush->brush();
             return mdl::BrushFace::create(
                      p1,
                      p2,
                      p3,
                      mdl::BrushFaceAttributes{currentMaterialName()},
                      m_world->mapFormat())
                    | kdl::and_then([&](mdl::BrushFace&& clipFace) {
                        return clippedBrush.clip(m_worldBounds, std::move(clipFace));
                      })
                    | kdl::and_then([&]() -> Result<std::pair<mdl::Node*, mdl::Brush>> {
                        return std::make_pair(
                          originalBrush->parent(), std::move(clippedBrush));
                      });
           })
         | kdl::fold | kdl::and_then([&](auto&& clippedBrushAndParents) -> Result<void> {
             auto toAdd = std::map<mdl::Node*, std::vector<mdl::Node*>>{};
             const auto toRemove =
               kdl::vec_static_cast<mdl::Node*>(m_selectedNodes.brushes());

             for (auto& [parentNode, clippedBrush] : clippedBrushAndParents)
             {
               toAdd[parentNode].push_back(new mdl::BrushNode{std::move(clippedBrush)});
             }

             auto transaction = Transaction{*this, "Clip Brushes"};
             deselectAll();
             removeNodes(toRemove);

             const auto addedNodes = addNodes(toAdd);
             if (addedNodes.empty())
             {
               transaction.cancel();
               return Error{"Could not replace brushes in document"};
             }
             selectNodes(addedNodes);
             if (!transaction.commit())
             {
               return Error{"Could not replace brushes in document"};
             }
             return kdl::void_success;
           })
         | kdl::if_error(
           [&](const auto& e) { error() << "Could not clip brushes: " << e; })
         | kdl::is_success();
}

bool MapDocument::setProperty(
  const std::string& key, const std::string& value, const bool defaultToProtected)
{
  const auto entityNodes = allSelectedEntityNodes();
  return applyAndSwap(
    *this,
    "Set Property",
    entityNodes,
    collectContainingGroups(entityNodes),
    kdl::overload(
      [](mdl::Layer&) { return true; },
      [](mdl::Group&) { return true; },
      [&](mdl::Entity& entity) {
        entity.addOrUpdateProperty(key, value, defaultToProtected);
        return true;
      },
      [](mdl::Brush&) { return true; },
      [](mdl::BezierPatch&) { return true; }));
}

bool MapDocument::renameProperty(const std::string& oldKey, const std::string& newKey)
{
  const auto entityNodes = allSelectedEntityNodes();
  return applyAndSwap(
    *this,
    "Rename Property",
    entityNodes,
    collectContainingGroups(entityNodes),
    kdl::overload(
      [](mdl::Layer&) { return true; },
      [](mdl::Group&) { return true; },
      [&](mdl::Entity& entity) {
        entity.renameProperty(oldKey, newKey);
        return true;
      },
      [](mdl::Brush&) { return true; },
      [](mdl::BezierPatch&) { return true; }));
}

bool MapDocument::removeProperty(const std::string& key)
{
  const auto entityNodes = allSelectedEntityNodes();
  return applyAndSwap(
    *this,
    "Remove Property",
    entityNodes,
    collectContainingGroups(entityNodes),
    kdl::overload(
      [](mdl::Layer&) { return true; },
      [](mdl::Group&) { return true; },
      [&](mdl::Entity& entity) {
        entity.removeProperty(key);
        return true;
      },
      [](mdl::Brush&) { return true; },
      [](mdl::BezierPatch&) { return true; }));
}

bool MapDocument::convertEntityColorRange(
  const std::string& key, mdl::ColorRange::Type range)
{
  const auto entityNodes = allSelectedEntityNodes();
  return applyAndSwap(
    *this,
    "Convert Color",
    entityNodes,
    collectContainingGroups(entityNodes),
    kdl::overload(
      [](mdl::Layer&) { return true; },
      [](mdl::Group&) { return true; },
      [&](mdl::Entity& entity) {
        if (const auto* oldValue = entity.property(key))
        {
          entity.addOrUpdateProperty(key, mdl::convertEntityColor(*oldValue, range));
        }
        return true;
      },
      [](mdl::Brush&) { return true; },
      [](mdl::BezierPatch&) { return true; }));
}

bool MapDocument::updateSpawnflag(
  const std::string& key, const size_t flagIndex, const bool setFlag)
{
  const auto entityNodes = allSelectedEntityNodes();
  return applyAndSwap(
    *this,
    setFlag ? "Set Spawnflag" : "Unset Spawnflag",
    entityNodes,
    collectContainingGroups(entityNodes),
    kdl::overload(
      [](mdl::Layer&) { return true; },
      [](mdl::Group&) { return true; },
      [&](mdl::Entity& entity) {
        const auto* strValue = entity.property(key);
        int intValue = strValue ? kdl::str_to_int(*strValue).value_or(0) : 0;
        const int flagValue = (1 << flagIndex);

        intValue = setFlag ? intValue | flagValue : intValue & ~flagValue;
        entity.addOrUpdateProperty(key, kdl::str_to_string(intValue));

        return true;
      },
      [](mdl::Brush&) { return true; },
      [](mdl::BezierPatch&) { return true; }));
}

namespace
{
/**
 * Search the given linked groups for an entity node at the given node path, and return
 * its unprotected value for the given property key.
 */
std::optional<std::string> findUnprotectedPropertyValue(
  const std::string& key, const std::vector<mdl::EntityNodeBase*>& linkedEntities)
{
  for (const auto* entityNode : linkedEntities)
  {
    if (!kdl::vec_contains(entityNode->entity().protectedProperties(), key))
    {
      if (const auto* value = entityNode->entity().property(key))
      {
        return *value;
      }
    }
  }
  return std::nullopt;
}

/**
 * Find the unprotected property value of the given key in the corresponding linked
 * nodes of the given entity nodes. This value is used to restore the original value
 * when a property is set from protected to unprotected.
 */
std::optional<std::string> findUnprotectedPropertyValue(
  const std::string& key,
  const mdl::EntityNodeBase& entityNode,
  mdl::WorldNode& worldNode)
{
  const auto linkedNodes = mdl::collectLinkedNodes({&worldNode}, entityNode);
  if (linkedNodes.size() > 1)
  {
    if (const auto value = findUnprotectedPropertyValue(key, linkedNodes))
    {
      return value;
    }
  }

  return std::nullopt;
}
} // namespace

bool MapDocument::setProtectedProperty(const std::string& key, const bool value)
{
  const auto entityNodes = allSelectedEntityNodes();

  auto nodesToUpdate = std::vector<std::pair<mdl::Node*, mdl::NodeContents>>{};
  for (auto* entityNode : entityNodes)
  {
    auto entity = entityNode->entity();
    auto protectedProperties = entity.protectedProperties();
    if (value && !kdl::vec_contains(protectedProperties, key))
    {
      protectedProperties.push_back(key);
    }
    else if (!value && kdl::vec_contains(protectedProperties, key))
    {
      if (
        const auto newValue =
          findUnprotectedPropertyValue(key, *entityNode, *m_world.get()))
      {
        entity.addOrUpdateProperty(key, *newValue);
      }

      protectedProperties = kdl::vec_erase(std::move(protectedProperties), key);
    }
    entity.setProtectedProperties(std::move(protectedProperties));
    nodesToUpdate.emplace_back(entityNode, std::move(entity));
  }

  return swapNodeContents(
    "Set Protected Property", nodesToUpdate, collectContainingGroups(entityNodes));
}

bool MapDocument::clearProtectedProperties()
{
  const auto entityNodes = allSelectedEntityNodes();

  auto nodesToUpdate = std::vector<std::pair<mdl::Node*, mdl::NodeContents>>{};
  for (auto* entityNode : entityNodes)
  {
    if (entityNode->entity().protectedProperties().empty())
    {
      continue;
    }

    const auto linkedEntities = mdl::collectLinkedNodes({m_world.get()}, *entityNode);
    if (linkedEntities.size() <= 1)
    {
      continue;
    }

    auto entity = entityNode->entity();
    for (const auto& key : entity.protectedProperties())
    {
      if (const auto newValue = findUnprotectedPropertyValue(key, linkedEntities))
      {
        entity.addOrUpdateProperty(key, *newValue);
      }
    }

    entity.setProtectedProperties({});
    nodesToUpdate.emplace_back(entityNode, std::move(entity));
  }

  return swapNodeContents(
    "Clear Protected Properties", nodesToUpdate, collectContainingGroups(entityNodes));
}

bool MapDocument::canClearProtectedProperties() const
{
  const auto entityNodes = allSelectedEntityNodes();
  if (
    entityNodes.empty()
    || (entityNodes.size() == 1u && entityNodes.front() == m_world.get()))
  {
    return false;
  }

  return canUpdateLinkedGroups(kdl::vec_static_cast<mdl::Node*>(entityNodes));
}

void MapDocument::setDefaultProperties(const mdl::SetDefaultPropertyMode mode)
{
  const auto entityNodes = allSelectedEntityNodes();
  applyAndSwap(
    *this,
    "Reset Default Properties",
    entityNodes,
    collectContainingGroups(entityNodes),
    kdl::overload(
      [](mdl::Layer&) { return true; },
      [](mdl::Group&) { return true; },
      [&](mdl::Entity& entity) {
        if (const auto* definition = entity.definition())
        {
          mdl::setDefaultProperties(*definition, entity, mode);
        }
        return true;
      },
      [](mdl::Brush&) { return true; },
      [](mdl::BezierPatch&) { return true; }));
}

bool MapDocument::extrudeBrushes(
  const std::vector<vm::polygon3d>& faces, const vm::vec3d& delta)
{
  const auto nodes = m_selectedNodes.nodes();
  return applyAndSwap(
    *this,
    "Resize Brushes",
    nodes,
    collectContainingGroups(nodes),
    kdl::overload(
      [](mdl::Layer&) { return true; },
      [](mdl::Group&) { return true; },
      [](mdl::Entity&) { return true; },
      [&](mdl::Brush& brush) {
        const auto faceIndex = brush.findFace(faces);
        if (!faceIndex)
        {
          // we allow resizing only some of the brushes
          return true;
        }

        return brush.moveBoundary(
                 m_worldBounds, *faceIndex, delta, pref(Preferences::AlignmentLock))
               | kdl::transform([&]() { return m_worldBounds.contains(brush.bounds()); })
               | kdl::transform_error([&](auto e) {
                   error() << "Could not resize brush: " << e.msg;
                   return false;
                 })
               | kdl::value();
      },
      [](mdl::BezierPatch&) { return true; }));
}

bool MapDocument::setFaceAttributes(const mdl::BrushFaceAttributes& attributes)
{
  mdl::ChangeBrushFaceAttributesRequest request;
  request.setAll(attributes);
  return setFaceAttributes(request);
}

bool MapDocument::setFaceAttributesExceptContentFlags(
  const mdl::BrushFaceAttributes& attributes)
{
  mdl::ChangeBrushFaceAttributesRequest request;
  request.setAllExceptContentFlags(attributes);
  return setFaceAttributes(request);
}

bool MapDocument::setFaceAttributes(const mdl::ChangeBrushFaceAttributesRequest& request)
{
  return applyAndSwap(
    *this, request.name(), allSelectedBrushFaces(), [&](mdl::BrushFace& brushFace) {
      request.evaluate(brushFace);
      return true;
    });
}

bool MapDocument::copyUVFromFace(
  const mdl::UVCoordSystemSnapshot& coordSystemSnapshot,
  const mdl::BrushFaceAttributes& attribs,
  const vm::plane3d& sourceFacePlane,
  const mdl::WrapStyle wrapStyle)
{
  return applyAndSwap(
    *this, "Copy UV Alignment", m_selectedBrushFaces, [&](mdl::BrushFace& face) {
      face.copyUVCoordSystemFromFace(
        coordSystemSnapshot, attribs, sourceFacePlane, wrapStyle);
      return true;
    });
}

bool MapDocument::translateUV(
  const vm::vec3f& cameraUp, const vm::vec3f& cameraRight, const vm::vec2f& delta)
{
  return applyAndSwap(*this, "Move UV", m_selectedBrushFaces, [&](mdl::BrushFace& face) {
    face.moveUV(vm::vec3d(cameraUp), vm::vec3d(cameraRight), delta);
    return true;
  });
}

bool MapDocument::rotateUV(const float angle)
{
  return applyAndSwap(
    *this, "Rotate UV", m_selectedBrushFaces, [&](mdl::BrushFace& face) {
      face.rotateUV(angle);
      return true;
    });
}

bool MapDocument::shearUV(const vm::vec2f& factors)
{
  return applyAndSwap(*this, "Shear UV", m_selectedBrushFaces, [&](mdl::BrushFace& face) {
    face.shearUV(factors);
    return true;
  });
}

bool MapDocument::flipUV(
  const vm::vec3f& cameraUp,
  const vm::vec3f& cameraRight,
  const vm::direction cameraRelativeFlipDirection)
{
  const bool isHFlip =
    (cameraRelativeFlipDirection == vm::direction::left
     || cameraRelativeFlipDirection == vm::direction::right);
  return applyAndSwap(
    *this,
    isHFlip ? "Flip UV Horizontally" : "Flip UV Vertically",
    m_selectedBrushFaces,
    [&](mdl::BrushFace& face) {
      face.flipUV(
        vm::vec3d(cameraUp), vm::vec3d(cameraRight), cameraRelativeFlipDirection);
      return true;
    });
}

bool MapDocument::snapVertices(const double snapTo)
{
  size_t succeededBrushCount = 0;
  size_t failedBrushCount = 0;

  const auto allSelectedBrushes = allSelectedBrushNodes();
  const bool applyAndSwapSuccess = applyAndSwap(
    *this,
    "Snap Brush Vertices",
    allSelectedBrushes,
    collectContainingGroups(allSelectedBrushes),
    kdl::overload(
      [](mdl::Layer&) { return true; },
      [](mdl::Group&) { return true; },
      [](mdl::Entity&) { return true; },
      [&](mdl::Brush& originalBrush) {
        if (originalBrush.canSnapVertices(m_worldBounds, snapTo))
        {
          originalBrush.snapVertices(m_worldBounds, snapTo, pref(Preferences::UVLock))
            | kdl::transform([&]() { succeededBrushCount += 1; })
            | kdl::transform_error([&](auto e) {
                error() << "Could not snap vertices: " << e.msg;
                failedBrushCount += 1;
              });
        }
        else
        {
          failedBrushCount += 1;
        }
        return true;
      },
      [](mdl::BezierPatch&) { return true; }));

  if (!applyAndSwapSuccess)
  {
    return false;
  }
  if (succeededBrushCount > 0)
  {
    info(kdl::str_to_string(
      "Snapped vertices of ",
      succeededBrushCount,
      " ",
      kdl::str_plural(succeededBrushCount, "brush", "brushes")));
  }
  if (failedBrushCount > 0)
  {
    info(kdl::str_to_string(
      "Failed to snap vertices of ",
      failedBrushCount,
      " ",
      kdl::str_plural(failedBrushCount, "brush", "brushes")));
  }

  return true;
}

MapDocument::TransformVerticesResult MapDocument::transformVertices(
  std::vector<vm::vec3d> vertexPositions, const vm::mat4x4d& transform)
{
  auto newVertexPositions = std::vector<vm::vec3d>{};
  auto newNodes = applyToNodeContents(
    m_selectedNodes.nodes(),
    kdl::overload(
      [](mdl::Layer&) { return true; },
      [](mdl::Group&) { return true; },
      [](mdl::Entity&) { return true; },
      [&](mdl::Brush& brush) {
        const auto verticesToMove = kdl::vec_filter(
          vertexPositions, [&](const auto& vertex) { return brush.hasVertex(vertex); });
        if (verticesToMove.empty())
        {
          return true;
        }

        if (!brush.canTransformVertices(m_worldBounds, verticesToMove, transform))
        {
          return false;
        }

        return brush.transformVertices(
                 m_worldBounds, verticesToMove, transform, pref(Preferences::UVLock))
               | kdl::transform([&]() {
                   auto newPositions =
                     brush.findClosestVertexPositions(transform * verticesToMove);
                   newVertexPositions = kdl::vec_concat(
                     std::move(newVertexPositions), std::move(newPositions));
                 })
               | kdl::if_error(
                 [&](auto e) { error() << "Could not move brush vertices: " << e.msg; })
               | kdl::is_success();
      },
      [](mdl::BezierPatch&) { return true; }));

  if (newNodes)
  {
    kdl::vec_sort_and_remove_duplicates(newVertexPositions);

    const auto commandName =
      kdl::str_plural(vertexPositions.size(), "Move Brush Vertex", "Move Brush Vertices");
    auto transaction = Transaction{*this, commandName};

    const auto changedLinkedGroups = collectContainingGroups(
      kdl::vec_transform(*newNodes, [](const auto& p) { return p.first; }));

    const auto result = executeAndStore(std::make_unique<BrushVertexCommand>(
      commandName,
      std::move(*newNodes),
      std::move(vertexPositions),
      std::move(newVertexPositions)));

    if (!result->success())
    {
      transaction.cancel();
      return TransformVerticesResult{false, false};
    }

    setHasPendingChanges(changedLinkedGroups, true);

    if (!transaction.commit())
    {
      return TransformVerticesResult{false, false};
    }

    const auto* moveVerticesResult =
      dynamic_cast<BrushVertexCommandResult*>(result.get());
    ensure(
      moveVerticesResult != nullptr,
      "command processor returned unexpected command result type");

    return {moveVerticesResult->success(), moveVerticesResult->hasRemainingVertices()};
  }

  return TransformVerticesResult{false, false};
}

bool MapDocument::transformEdges(
  std::vector<vm::segment3d> edgePositions, const vm::mat4x4d& transform)
{
  auto newEdgePositions = std::vector<vm::segment3d>{};
  auto newNodes = applyToNodeContents(
    m_selectedNodes.nodes(),
    kdl::overload(
      [](mdl::Layer&) { return true; },
      [](mdl::Group&) { return true; },
      [](mdl::Entity&) { return true; },
      [&](mdl::Brush& brush) {
        const auto edgesToMove = kdl::vec_filter(
          edgePositions, [&](const auto& edge) { return brush.hasEdge(edge); });
        if (edgesToMove.empty())
        {
          return true;
        }

        if (!brush.canTransformEdges(m_worldBounds, edgesToMove, transform))
        {
          return false;
        }

        return brush.transformEdges(
                 m_worldBounds, edgesToMove, transform, pref(Preferences::UVLock))
               | kdl::transform([&]() {
                   auto newPositions = brush.findClosestEdgePositions(kdl::vec_transform(
                     edgesToMove,
                     [&](const auto& edge) { return edge.transform(transform); }));
                   newEdgePositions = kdl::vec_concat(
                     std::move(newEdgePositions), std::move(newPositions));
                 })
               | kdl::if_error(
                 [&](auto e) { error() << "Could not move brush edges: " << e.msg; })
               | kdl::is_success();
      },
      [](mdl::BezierPatch&) { return true; }));

  if (newNodes)
  {
    kdl::vec_sort_and_remove_duplicates(newEdgePositions);

    const auto commandName =
      kdl::str_plural(edgePositions.size(), "Move Brush Edge", "Move Brush Edges");
    auto transaction = Transaction{*this, commandName};

    const auto changedLinkedGroups = collectContainingGroups(
      kdl::vec_transform(*newNodes, [](const auto& p) { return p.first; }));

    const auto result = executeAndStore(std::make_unique<BrushEdgeCommand>(
      commandName,
      std::move(*newNodes),
      std::move(edgePositions),
      std::move(newEdgePositions)));

    if (!result->success())
    {
      transaction.cancel();
      return false;
    }

    setHasPendingChanges(changedLinkedGroups, true);
    return transaction.commit();
  }

  return false;
}

bool MapDocument::transformFaces(
  std::vector<vm::polygon3d> facePositions, const vm::mat4x4d& transform)
{
  auto newFacePositions = std::vector<vm::polygon3d>{};
  auto newNodes = applyToNodeContents(
    m_selectedNodes.nodes(),
    kdl::overload(
      [](mdl::Layer&) { return true; },
      [](mdl::Group&) { return true; },
      [](mdl::Entity&) { return true; },
      [&](mdl::Brush& brush) {
        const auto facesToMove = kdl::vec_filter(
          facePositions, [&](const auto& face) { return brush.hasFace(face); });
        if (facesToMove.empty())
        {
          return true;
        }

        if (!brush.canTransformFaces(m_worldBounds, facesToMove, transform))
        {
          return false;
        }

        return brush.transformFaces(
                 m_worldBounds, facesToMove, transform, pref(Preferences::UVLock))
               | kdl::transform([&]() {
                   auto newPositions = brush.findClosestFacePositions(kdl::vec_transform(
                     facesToMove,
                     [&](const auto& face) { return face.transform(transform); }));
                   newFacePositions = kdl::vec_concat(
                     std::move(newFacePositions), std::move(newPositions));
                 })
               | kdl::if_error(
                 [&](auto e) { error() << "Could not move brush faces: " << e.msg; })
               | kdl::is_success();
      },
      [](mdl::BezierPatch&) { return true; }));

  if (newNodes)
  {
    kdl::vec_sort_and_remove_duplicates(newFacePositions);

    const auto commandName =
      kdl::str_plural(facePositions.size(), "Move Brush Face", "Move Brush Faces");
    auto transaction = Transaction{*this, commandName};

    auto changedLinkedGroups = collectContainingGroups(
      kdl::vec_transform(*newNodes, [](const auto& p) { return p.first; }));

    const auto result = executeAndStore(std::make_unique<BrushFaceCommand>(
      commandName,
      std::move(*newNodes),
      std::move(facePositions),
      std::move(newFacePositions)));

    if (!result->success())
    {
      transaction.cancel();
      return false;
    }

    setHasPendingChanges(changedLinkedGroups, true);
    return transaction.commit();
  }

  return false;
}

bool MapDocument::addVertex(const vm::vec3d& vertexPosition)
{
  auto newNodes = applyToNodeContents(
    m_selectedNodes.nodes(),
    kdl::overload(
      [](mdl::Layer&) { return true; },
      [](mdl::Group&) { return true; },
      [](mdl::Entity&) { return true; },
      [&](mdl::Brush& brush) {
        if (!brush.canAddVertex(m_worldBounds, vertexPosition))
        {
          return false;
        }

        return brush.addVertex(m_worldBounds, vertexPosition)
               | kdl::if_error(
                 [&](auto e) { error() << "Could not add brush vertex: " << e.msg; })
               | kdl::is_success();
      },
      [](mdl::BezierPatch&) { return true; }));

  if (newNodes)
  {
    const auto commandName = "Add Brush Vertex";
    auto transaction = Transaction{*this, commandName};

    const auto changedLinkedGroups = collectContainingGroups(
      kdl::vec_transform(*newNodes, [](const auto& p) { return p.first; }));

    const auto result = executeAndStore(std::make_unique<BrushVertexCommand>(
      commandName,
      std::move(*newNodes),
      std::vector<vm::vec3d>{},
      std::vector<vm::vec3d>{vertexPosition}));

    if (!result->success())
    {
      transaction.cancel();
      return false;
    }

    setHasPendingChanges(changedLinkedGroups, true);
    return transaction.commit();
  }

  return false;
}

bool MapDocument::removeVertices(
  const std::string& commandName, std::vector<vm::vec3d> vertexPositions)
{
  auto newNodes = applyToNodeContents(
    m_selectedNodes.nodes(),
    kdl::overload(
      [](mdl::Layer&) { return true; },
      [](mdl::Group&) { return true; },
      [](mdl::Entity&) { return true; },
      [&](mdl::Brush& brush) {
        const auto verticesToRemove = kdl::vec_filter(
          vertexPositions, [&](const auto& vertex) { return brush.hasVertex(vertex); });
        if (verticesToRemove.empty())
        {
          return true;
        }

        if (!brush.canRemoveVertices(m_worldBounds, verticesToRemove))
        {
          return false;
        }

        return brush.removeVertices(m_worldBounds, verticesToRemove)
               | kdl::if_error(
                 [&](auto e) { error() << "Could not remove brush vertices: " << e.msg; })
               | kdl::is_success();
      },
      [](mdl::BezierPatch&) { return true; }));

  if (newNodes)
  {
    auto transaction = Transaction{*this, commandName};

    auto changedLinkedGroups = collectContainingGroups(
      kdl::vec_transform(*newNodes, [](const auto& p) { return p.first; }));

    const auto result = executeAndStore(std::make_unique<BrushVertexCommand>(
      commandName,
      std::move(*newNodes),
      std::move(vertexPositions),
      std::vector<vm::vec3d>{}));

    if (!result->success())
    {
      transaction.cancel();
      return false;
    }

    setHasPendingChanges(changedLinkedGroups, true);
    return transaction.commit();
  }

  return false;
}

void MapDocument::printVertices()
{
  if (hasSelectedBrushFaces())
  {
    for (const auto& handle : m_selectedBrushFaces)
    {
      std::stringstream str;
      str.precision(17);
      for (const mdl::BrushVertex* vertex : handle.face().vertices())
      {
        str << "(" << vertex->position() << ") ";
      }
      info(str.str());
    }
  }
  else if (selectedNodes().hasBrushes())
  {
    for (const mdl::BrushNode* brushNode : selectedNodes().brushes())
    {
      const mdl::Brush& brush = brushNode->brush();

      std::stringstream str;
      str.precision(17);
      for (const mdl::BrushVertex* vertex : brush.vertices())
      {
        str << vertex->position() << " ";
      }
      info(str.str());
    }
  }
}

namespace
{

class ThrowExceptionCommand : public UndoableCommand
{
public:
  using Ptr = std::shared_ptr<ThrowExceptionCommand>;

public:
  ThrowExceptionCommand()
    : UndoableCommand("Throw Exception", false)
  {
  }

private:
  std::unique_ptr<CommandResult> doPerformDo(MapDocumentCommandFacade&) override
  {
    throw CommandProcessorException();
  }

  std::unique_ptr<CommandResult> doPerformUndo(MapDocumentCommandFacade&) override
  {
    return std::make_unique<CommandResult>(true);
  }
};

} // namespace

bool MapDocument::throwExceptionDuringCommand()
{
  const auto result = executeAndStore(std::make_unique<ThrowExceptionCommand>());
  return result->success();
}

bool MapDocument::canUndoCommand() const
{
  return doCanUndoCommand();
}

bool MapDocument::canRedoCommand() const
{
  return doCanRedoCommand();
}

const std::string& MapDocument::undoCommandName() const
{
  return doGetUndoCommandName();
}

const std::string& MapDocument::redoCommandName() const
{
  return doGetRedoCommandName();
}

void MapDocument::undoCommand()
{
  doUndoCommand();
  updateLinkedGroups();

  // Undo/redo in the repeat system is not supported for now, so just clear the repeat
  // stack
  m_repeatStack->clear();
}

void MapDocument::redoCommand()
{
  doRedoCommand();
  updateLinkedGroups();

  // Undo/redo in the repeat system is not supported for now, so just clear the repeat
  // stack
  m_repeatStack->clear();
}

bool MapDocument::canRepeatCommands() const
{
  return m_repeatStack->size() > 0u;
}

void MapDocument::repeatCommands()
{
  m_repeatStack->repeat();
}

void MapDocument::clearRepeatableCommands()
{
  m_repeatStack->clear();
}

void MapDocument::startTransaction(std::string name, const TransactionScope scope)
{
  debug("Starting transaction '" + name + "'");
  doStartTransaction(std::move(name), scope);
  m_repeatStack->startTransaction();
}

void MapDocument::rollbackTransaction()
{
  debug("Rolling back transaction");
  doRollbackTransaction();
  m_repeatStack->rollbackTransaction();
}

bool MapDocument::commitTransaction()
{
  debug("Committing transaction");

  if (!updateLinkedGroups())
  {
    cancelTransaction();
    return false;
  }

  doCommitTransaction();
  m_repeatStack->commitTransaction();
  return true;
}

void MapDocument::cancelTransaction()
{
  debug("Cancelling transaction");
  doRollbackTransaction();
  m_repeatStack->rollbackTransaction();
  doCommitTransaction();
  m_repeatStack->commitTransaction();
}

std::unique_ptr<CommandResult> MapDocument::execute(std::unique_ptr<Command>&& command)
{
  return doExecute(std::move(command));
}

std::unique_ptr<CommandResult> MapDocument::executeAndStore(
  std::unique_ptr<UndoableCommand>&& command)
{
  return doExecuteAndStore(std::move(command));
}

void MapDocument::processResourcesSync(const mdl::ProcessContext& processContext)
{
  auto allProcessedResourceIds = std::vector<mdl::ResourceId>{};
  while (m_resourceManager->needsProcessing())
  {
    auto processedResourceIds = m_resourceManager->process(
      [](auto task) {
        auto promise = std::promise<std::unique_ptr<mdl::TaskResult>>{};
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

void MapDocument::processResourcesAsync(const mdl::ProcessContext& processContext)
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

bool MapDocument::needsResourceProcessing()
{
  return m_resourceManager->needsProcessing();
}

void MapDocument::pick(const vm::ray3d& pickRay, mdl::PickResult& pickResult) const
{
  if (m_world)
  {
    m_world->pick(*m_editorContext, pickRay, pickResult);
  }
}

std::vector<mdl::Node*> MapDocument::findNodesContaining(const vm::vec3d& point) const
{
  auto result = std::vector<mdl::Node*>{};
  if (m_world)
  {
    m_world->findNodesContaining(point, result);
  }
  return result;
}

void MapDocument::setWorld(
  const vm::bbox3d& worldBounds,
  std::unique_ptr<mdl::WorldNode> worldNode,
  std::shared_ptr<mdl::Game> game,
  const std::filesystem::path& path)
{
  m_worldBounds = worldBounds;
  m_world = std::move(worldNode);
  m_game = game;

  m_entityModelManager->setGame(game.get(), m_taskManager);
  performSetCurrentLayer(m_world->defaultLayer());

  updateGameSearchPaths();
  setPath(path);

  loadAssets();
  registerValidators();
  registerSmartTags();
  createTagActions();
}

void MapDocument::clearWorld()
{
  m_world.reset();
  m_currentLayer = nullptr;
}

mdl::EntityDefinitionFileSpec MapDocument::entityDefinitionFile() const
{
  return m_world ? m_game->extractEntityDefinitionFile(m_world->entity())
                 : mdl::EntityDefinitionFileSpec{};
}

std::vector<mdl::EntityDefinitionFileSpec> MapDocument::allEntityDefinitionFiles() const
{
  return m_game->allEntityDefinitionFiles();
}

void MapDocument::setEntityDefinitionFile(const mdl::EntityDefinitionFileSpec& spec)
{
  // to avoid backslashes being misinterpreted as escape sequences
  const std::string formatted = kdl::str_replace_every(spec.asString(), "\\", "/");

  auto entity = m_world->entity();
  entity.addOrUpdateProperty(mdl::EntityPropertyKeys::EntityDefinitions, formatted);
  swapNodeContents(
    "Set Entity Definitions", {{world(), mdl::NodeContents(std::move(entity))}}, {});
}

void MapDocument::setEntityDefinitions(std::vector<mdl::EntityDefinition> definitions)
{
  m_entityDefinitionManager->setDefinitions(std::move(definitions));
}

void MapDocument::reloadMaterialCollections()
{
  const auto nodes = std::vector<mdl::Node*>{m_world.get()};
  NotifyBeforeAndAfter notifyNodes(
    nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
  NotifyBeforeAndAfter notifyMaterialCollections(
    materialCollectionsWillChangeNotifier, materialCollectionsDidChangeNotifier);

  info("Reloading material collections");
  unloadMaterials();
  // materialCollectionsDidChange will load the collections again
}

void MapDocument::reloadEntityDefinitions()
{
  const auto nodes = std::vector<mdl::Node*>{m_world.get()};
  NotifyBeforeAndAfter notifyNodes(
    nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
  NotifyBeforeAndAfter notifyEntityDefinitions(
    entityDefinitionsWillChangeNotifier, entityDefinitionsDidChangeNotifier);

  info("Reloading entity definitions");
}

std::vector<std::filesystem::path> MapDocument::enabledMaterialCollections() const
{
  if (m_world)
  {
    if (
      const auto* materialCollectionStr =
        m_world->entity().property(mdl::EntityPropertyKeys::EnabledMaterialCollections))
    {
      const auto strs = kdl::str_split(*materialCollectionStr, ";");
      return kdl::vec_sort_and_remove_duplicates(
        strs | std::views::transform([](const auto& str) {
          return std::filesystem::path{str};
        })
        | kdl::to_vector);
    }

    // Otherwise, enable all texture collections
    return kdl::vec_sort_and_remove_duplicates(
      m_materialManager->collections()
      | std::views::transform([](const auto& collection) { return collection.path(); })
      | kdl::to_vector);
  }
  return {};
}

std::vector<std::filesystem::path> MapDocument::disabledMaterialCollections() const
{
  if (m_world)
  {
    auto materialCollections = kdl::vec_sort_and_remove_duplicates(kdl::vec_transform(
      m_materialManager->collections(),
      [](const auto& collection) { return collection.path(); }));

    return kdl::set_difference(materialCollections, enabledMaterialCollections());
  }
  return {};
}

void MapDocument::setEnabledMaterialCollections(
  const std::vector<std::filesystem::path>& enabledMaterialCollections)
{
  const auto enabledMaterialCollectionStr = kdl::str_join(
    kdl::vec_transform(
      kdl::vec_sort_and_remove_duplicates(enabledMaterialCollections),
      [](const auto& path) { return path.string(); }),
    ";");

  auto transaction = Transaction{*this, "Set enabled material collections"};

  const auto pushSelection = mdl::PushSelection{this};
  deselectAll();

  const auto success = setProperty(
    mdl::EntityPropertyKeys::EnabledMaterialCollections, enabledMaterialCollectionStr);
  transaction.finish(success);
}

void MapDocument::loadAssets()
{
  loadEntityDefinitions();
  setEntityDefinitions();
  loadEntityModels();
  loadMaterials();
  setMaterials();
}

void MapDocument::unloadAssets()
{
  unloadEntityDefinitions();
  unloadEntityModels();
  unloadMaterials();
}

void MapDocument::loadEntityDefinitions()
{
  const auto spec = entityDefinitionFile();
  const auto path = m_game->findEntityDefinitionFile(spec, externalSearchPaths());
  auto status = io::SimpleParserStatus{logger()};

  m_entityDefinitionManager->loadDefinitions(path, *m_game, status)
    | kdl::transform([&]() {
        info(fmt::format("Loaded entity definition file {}", path.filename()));
        createEntityDefinitionActions();
      })
    | kdl::transform_error([&](auto e) {
        if (spec.builtin())
        {
          error() << "Could not load builtin entity definition file '" << spec.path()
                  << "': " << e.msg;
        }
        else
        {
          error() << "Could not load external entity definition file '" << spec.path()
                  << "': " << e.msg;
        }
      });
}

void MapDocument::unloadEntityDefinitions()
{
  unsetEntityDefinitions();
  m_entityDefinitionManager->clear();
  m_entityDefinitionActions.clear();
}

void MapDocument::loadEntityModels()
{
  setEntityModels();
}

void MapDocument::unloadEntityModels()
{
  clearEntityModels();
}

void MapDocument::reloadMaterials()
{
  unloadMaterials();
  loadMaterials();
}

void MapDocument::loadMaterials()
{
  if (const auto* wadStr = m_world->entity().property(mdl::EntityPropertyKeys::Wad))
  {
    const auto wadPaths = kdl::vec_transform(
      kdl::str_split(*wadStr, ";"),
      [](const auto& str) { return std::filesystem::path{str}; });
    m_game->reloadWads(path(), wadPaths, logger());
  }
  m_materialManager->reload(
    m_game->gameFileSystem(),
    m_game->config().materialConfig,
    [&](auto resourceLoader) {
      auto resource = std::make_shared<mdl::TextureResource>(std::move(resourceLoader));
      m_resourceManager->addResource(resource);
      return resource;
    },
    m_taskManager);
}

void MapDocument::unloadMaterials()
{
  unsetMaterials();
  m_materialManager->clear();
}

static auto makeSetMaterialsVisitor(mdl::MaterialManager& manager)
{
  return kdl::overload(
    [](auto&& thisLambda, mdl::WorldNode* world) { world->visitChildren(thisLambda); },
    [](auto&& thisLambda, mdl::LayerNode* layer) { layer->visitChildren(thisLambda); },
    [](auto&& thisLambda, mdl::GroupNode* group) { group->visitChildren(thisLambda); },
    [](auto&& thisLambda, mdl::EntityNode* entity) { entity->visitChildren(thisLambda); },
    [&](mdl::BrushNode* brushNode) {
      const mdl::Brush& brush = brushNode->brush();
      for (size_t i = 0u; i < brush.faceCount(); ++i)
      {
        const mdl::BrushFace& face = brush.face(i);
        mdl::Material* material = manager.material(face.attributes().materialName());
        brushNode->setFaceMaterial(i, material);
      }
    },
    [&](mdl::PatchNode* patchNode) {
      auto* material = manager.material(patchNode->patch().materialName());
      patchNode->setMaterial(material);
    });
}

static auto makeUnsetMaterialsVisitor()
{
  return kdl::overload(
    [](auto&& thisLambda, mdl::WorldNode* world) { world->visitChildren(thisLambda); },
    [](auto&& thisLambda, mdl::LayerNode* layer) { layer->visitChildren(thisLambda); },
    [](auto&& thisLambda, mdl::GroupNode* group) { group->visitChildren(thisLambda); },
    [](auto&& thisLambda, mdl::EntityNode* entity) { entity->visitChildren(thisLambda); },
    [](mdl::BrushNode* brushNode) {
      const mdl::Brush& brush = brushNode->brush();
      for (size_t i = 0u; i < brush.faceCount(); ++i)
      {
        brushNode->setFaceMaterial(i, nullptr);
      }
    },
    [](mdl::PatchNode* patchNode) { patchNode->setMaterial(nullptr); });
}

void MapDocument::setMaterials()
{
  m_world->accept(makeSetMaterialsVisitor(*m_materialManager));
  materialUsageCountsDidChangeNotifier();
}

void MapDocument::setMaterials(const std::vector<mdl::Node*>& nodes)
{
  mdl::Node::visitAll(nodes, makeSetMaterialsVisitor(*m_materialManager));
  materialUsageCountsDidChangeNotifier();
}

void MapDocument::setMaterials(const std::vector<mdl::BrushFaceHandle>& faceHandles)
{
  for (const auto& faceHandle : faceHandles)
  {
    mdl::BrushNode* node = faceHandle.node();
    const mdl::BrushFace& face = faceHandle.face();
    auto* material = m_materialManager->material(face.attributes().materialName());
    node->setFaceMaterial(faceHandle.faceIndex(), material);
  }
  materialUsageCountsDidChangeNotifier();
}

void MapDocument::unsetMaterials()
{
  m_world->accept(makeUnsetMaterialsVisitor());
  materialUsageCountsDidChangeNotifier();
}

void MapDocument::unsetMaterials(const std::vector<mdl::Node*>& nodes)
{
  mdl::Node::visitAll(nodes, makeUnsetMaterialsVisitor());
  materialUsageCountsDidChangeNotifier();
}

static auto makeSetEntityDefinitionsVisitor(mdl::EntityDefinitionManager& manager)
{
  // this helper lambda must be captured by value
  const auto setEntityDefinition = [&](auto* node) {
    const auto* definition = manager.definition(node);
    node->setDefinition(definition);
  };

  return kdl::overload(
    [=](auto&& thisLambda, mdl::WorldNode* world) {
      setEntityDefinition(world);
      world->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, mdl::LayerNode* layer) { layer->visitChildren(thisLambda); },
    [](auto&& thisLambda, mdl::GroupNode* group) { group->visitChildren(thisLambda); },
    [=](mdl::EntityNode* entity) { setEntityDefinition(entity); },
    [](mdl::BrushNode*) {},
    [](mdl::PatchNode*) {});
}

static auto makeUnsetEntityDefinitionsVisitor()
{
  return kdl::overload(
    [](auto&& thisLambda, mdl::WorldNode* world) {
      world->setDefinition(nullptr);
      world->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, mdl::LayerNode* layer) { layer->visitChildren(thisLambda); },
    [](auto&& thisLambda, mdl::GroupNode* group) { group->visitChildren(thisLambda); },
    [](mdl::EntityNode* entity) { entity->setDefinition(nullptr); },
    [](mdl::BrushNode*) {},
    [](mdl::PatchNode*) {});
}

void MapDocument::setEntityDefinitions()
{
  m_world->accept(makeSetEntityDefinitionsVisitor(*m_entityDefinitionManager));
}

void MapDocument::setEntityDefinitions(const std::vector<mdl::Node*>& nodes)
{
  mdl::Node::visitAll(nodes, makeSetEntityDefinitionsVisitor(*m_entityDefinitionManager));
}

void MapDocument::unsetEntityDefinitions()
{
  m_world->accept(makeUnsetEntityDefinitionsVisitor());
}

void MapDocument::unsetEntityDefinitions(const std::vector<mdl::Node*>& nodes)
{
  mdl::Node::visitAll(nodes, makeUnsetEntityDefinitionsVisitor());
}

void MapDocument::reloadEntityDefinitionsInternal()
{
  unloadEntityDefinitions();
  clearEntityModels();
  loadEntityDefinitions();
  setEntityDefinitions();
  setEntityModels();
}

void MapDocument::clearEntityModels()
{
  unsetEntityModels();
  m_entityModelManager->clear();
}

static auto makeSetEntityModelsVisitor(mdl::EntityModelManager& manager, Logger& logger)
{
  return kdl::overload(
    [](auto&& thisLambda, mdl::WorldNode* world) { world->visitChildren(thisLambda); },
    [](auto&& thisLambda, mdl::LayerNode* layer) { layer->visitChildren(thisLambda); },
    [](auto&& thisLambda, mdl::GroupNode* group) { group->visitChildren(thisLambda); },
    [&](mdl::EntityNode* entityNode) {
      const auto modelSpec =
        mdl::safeGetModelSpecification(logger, entityNode->entity().classname(), [&]() {
          return entityNode->entity().modelSpecification();
        });
      const auto* model = manager.model(modelSpec.path);
      entityNode->setModel(model);
    },
    [](mdl::BrushNode*) {},
    [](mdl::PatchNode*) {});
}

static auto makeUnsetEntityModelsVisitor()
{
  return kdl::overload(
    [](auto&& thisLambda, mdl::WorldNode* world) { world->visitChildren(thisLambda); },
    [](auto&& thisLambda, mdl::LayerNode* layer) { layer->visitChildren(thisLambda); },
    [](auto&& thisLambda, mdl::GroupNode* group) { group->visitChildren(thisLambda); },
    [](mdl::EntityNode* entity) { entity->setModel(nullptr); },
    [](mdl::BrushNode*) {},
    [](mdl::PatchNode*) {});
}

void MapDocument::setEntityModels()
{
  m_world->accept(makeSetEntityModelsVisitor(*m_entityModelManager, *this));
}

void MapDocument::setEntityModels(const std::vector<mdl::Node*>& nodes)
{
  mdl::Node::visitAll(nodes, makeSetEntityModelsVisitor(*m_entityModelManager, *this));
}

void MapDocument::unsetEntityModels()
{
  m_world->accept(makeUnsetEntityModelsVisitor());
}

void MapDocument::unsetEntityModels(const std::vector<mdl::Node*>& nodes)
{
  mdl::Node::visitAll(nodes, makeUnsetEntityModelsVisitor());
}

std::vector<std::filesystem::path> MapDocument::externalSearchPaths() const
{
  std::vector<std::filesystem::path> searchPaths;
  if (!m_path.empty() && m_path.is_absolute())
  {
    searchPaths.push_back(m_path.parent_path());
  }

  const std::filesystem::path gamePath = m_game->gamePath();
  if (!gamePath.empty())
  {
    searchPaths.push_back(gamePath);
  }

  searchPaths.push_back(io::SystemPaths::appDirectory());
  return searchPaths;
}

void MapDocument::updateGameSearchPaths()
{
  m_game->setAdditionalSearchPaths(
    kdl::vec_transform(
      mods(), [](const auto& mod) { return std::filesystem::path{mod}; }),
    logger());
}

std::vector<std::string> MapDocument::mods() const
{
  return m_game->extractEnabledMods(m_world->entity());
}

void MapDocument::setMods(const std::vector<std::string>& mods)
{
  auto entity = m_world->entity();
  if (mods.empty())
  {
    entity.removeProperty(mdl::EntityPropertyKeys::Mods);
  }
  else
  {
    const std::string newValue = kdl::str_join(mods, ";");
    entity.addOrUpdateProperty(mdl::EntityPropertyKeys::Mods, newValue);
  }
  swapNodeContents(
    "Set Enabled Mods", {{world(), mdl::NodeContents(std::move(entity))}}, {});
}

std::string MapDocument::defaultMod() const
{
  return m_game->defaultMod();
}

/**
 * Note if bounds.source is SoftMapBoundsType::Game, bounds.bounds is ignored.
 */
void MapDocument::setSoftMapBounds(const mdl::Game::SoftMapBounds& bounds)
{
  auto entity = world()->entity();
  switch (bounds.source)
  {
  case mdl::Game::SoftMapBoundsType::Map:
    if (!bounds.bounds.has_value())
    {
      // Set the worldspawn key EntityPropertyKeys::SoftMaxMapSize's value to the empty
      // string to indicate that we are overriding the Game's bounds with unlimited.
      entity.addOrUpdateProperty(
        mdl::EntityPropertyKeys::SoftMapBounds,
        mdl::EntityPropertyValues::NoSoftMapBounds);
    }
    else
    {
      entity.addOrUpdateProperty(
        mdl::EntityPropertyKeys::SoftMapBounds,
        io::serializeSoftMapBoundsString(*bounds.bounds));
    }
    break;
  case mdl::Game::SoftMapBoundsType::Game:
    // Unset the map's setting
    entity.removeProperty(mdl::EntityPropertyKeys::SoftMapBounds);
    break;
    switchDefault();
  }
  swapNodeContents(
    "Set Soft Map Bounds", {{world(), mdl::NodeContents(std::move(entity))}}, {});
}

mdl::Game::SoftMapBounds MapDocument::softMapBounds() const
{
  if (!m_world)
  {
    return {mdl::Game::SoftMapBoundsType::Game, std::nullopt};
  }
  return m_game->extractSoftMapBounds(m_world->entity());
}

void MapDocument::setIssueHidden(const mdl::Issue& issue, const bool hidden)
{
  doSetIssueHidden(issue, hidden);
}

void MapDocument::registerValidators()
{
  ensure(m_world, "world is null");
  ensure(m_game.get() != nullptr, "game is null");

  m_world->registerValidator(std::make_unique<mdl::MissingClassnameValidator>());
  m_world->registerValidator(std::make_unique<mdl::MissingDefinitionValidator>());
  m_world->registerValidator(std::make_unique<mdl::MissingModValidator>(m_game));
  m_world->registerValidator(std::make_unique<mdl::EmptyGroupValidator>());
  m_world->registerValidator(std::make_unique<mdl::EmptyBrushEntityValidator>());
  m_world->registerValidator(std::make_unique<mdl::PointEntityWithBrushesValidator>());
  m_world->registerValidator(std::make_unique<mdl::LinkSourceValidator>());
  m_world->registerValidator(std::make_unique<mdl::LinkTargetValidator>());
  m_world->registerValidator(std::make_unique<mdl::NonIntegerVerticesValidator>());
  m_world->registerValidator(std::make_unique<mdl::MixedBrushContentsValidator>());
  m_world->registerValidator(std::make_unique<mdl::WorldBoundsValidator>(worldBounds()));
  m_world->registerValidator(
    std::make_unique<mdl::SoftMapBoundsValidator>(m_game, *m_world));
  m_world->registerValidator(std::make_unique<mdl::EmptyPropertyKeyValidator>());
  m_world->registerValidator(std::make_unique<mdl::EmptyPropertyValueValidator>());
  m_world->registerValidator(
    std::make_unique<mdl::LongPropertyKeyValidator>(m_game->config().maxPropertyLength));
  m_world->registerValidator(std::make_unique<mdl::LongPropertyValueValidator>(
    m_game->config().maxPropertyLength));
  m_world->registerValidator(
    std::make_unique<mdl::PropertyKeyWithDoubleQuotationMarksValidator>());
  m_world->registerValidator(
    std::make_unique<mdl::PropertyValueWithDoubleQuotationMarksValidator>());
  m_world->registerValidator(std::make_unique<mdl::InvalidUVScaleValidator>());
}

void MapDocument::registerSmartTags()
{
  ensure(m_game.get() != nullptr, "game is null");

  m_tagManager->clearSmartTags();
  m_tagManager->registerSmartTags(m_game->config().smartTags);
}

const std::vector<mdl::SmartTag>& MapDocument::smartTags() const
{
  return m_tagManager->smartTags();
}

bool MapDocument::isRegisteredSmartTag(const std::string& name) const
{
  return m_tagManager->isRegisteredSmartTag(name);
}

const mdl::SmartTag& MapDocument::smartTag(const std::string& name) const
{
  return m_tagManager->smartTag(name);
}

bool MapDocument::isRegisteredSmartTag(const size_t index) const
{
  return m_tagManager->isRegisteredSmartTag(index);
}

const mdl::SmartTag& MapDocument::smartTag(const size_t index) const
{
  return m_tagManager->smartTag(index);
}

static auto makeInitializeNodeTagsVisitor(mdl::TagManager& tagManager)
{
  return kdl::overload(
    [&](auto&& thisLambda, mdl::WorldNode* world) {
      world->initializeTags(tagManager);
      world->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, mdl::LayerNode* layer) {
      layer->initializeTags(tagManager);
      layer->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, mdl::GroupNode* group) {
      group->initializeTags(tagManager);
      group->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, mdl::EntityNode* entity) {
      entity->initializeTags(tagManager);
      entity->visitChildren(thisLambda);
    },
    [&](mdl::BrushNode* brush) { brush->initializeTags(tagManager); },
    [&](mdl::PatchNode* patch) { patch->initializeTags(tagManager); });
}

static auto makeClearNodeTagsVisitor()
{
  return kdl::overload(
    [](auto&& thisLambda, mdl::WorldNode* world) {
      world->clearTags();
      world->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, mdl::LayerNode* layer) {
      layer->clearTags();
      layer->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, mdl::GroupNode* group) {
      group->clearTags();
      group->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, mdl::EntityNode* entity) {
      entity->clearTags();
      entity->visitChildren(thisLambda);
    },
    [](mdl::BrushNode* brush) { brush->clearTags(); },
    [](mdl::PatchNode* patch) { patch->clearTags(); });
}

void MapDocument::initializeAllNodeTags(MapDocument* document)
{
  assert(document == this);
  unused(document);
  m_world->accept(makeInitializeNodeTagsVisitor(*m_tagManager));
}

void MapDocument::initializeNodeTags(const std::vector<mdl::Node*>& nodes)
{
  mdl::Node::visitAll(nodes, makeInitializeNodeTagsVisitor(*m_tagManager));
}

void MapDocument::clearNodeTags(const std::vector<mdl::Node*>& nodes)
{
  mdl::Node::visitAll(nodes, makeClearNodeTagsVisitor());
}

void MapDocument::updateNodeTags(const std::vector<mdl::Node*>& nodes)
{
  for (auto* node : nodes)
  {
    node->updateTags(*m_tagManager);
  }
}

void MapDocument::updateFaceTags(const std::vector<mdl::BrushFaceHandle>& faceHandles)
{
  for (const auto& faceHandle : faceHandles)
  {
    mdl::BrushNode* node = faceHandle.node();
    node->updateFaceTags(faceHandle.faceIndex(), *m_tagManager);
  }
}

void MapDocument::updateAllFaceTags()
{
  m_world->accept(kdl::overload(
    [](auto&& thisLambda, mdl::WorldNode* world) { world->visitChildren(thisLambda); },
    [](auto&& thisLambda, mdl::LayerNode* layer) { layer->visitChildren(thisLambda); },
    [](auto&& thisLambda, mdl::GroupNode* group) { group->visitChildren(thisLambda); },
    [](auto&& thisLambda, mdl::EntityNode* entity) { entity->visitChildren(thisLambda); },
    [&](mdl::BrushNode* brush) { brush->initializeTags(*m_tagManager); },
    [](mdl::PatchNode*) {}));
}

void MapDocument::updateFaceTagsAfterResourcesWhereProcessed(
  const std::vector<mdl::ResourceId>& resourceIds)
{
  // Some textures contain embedded default values for surface flags and such, so we must
  // update the face tags after the resources have been processed.

  const auto materials = m_materialManager->findMaterialsByTextureResourceId(resourceIds);
  const auto materialSet =
    std::unordered_set<const mdl::Material*>{materials.begin(), materials.end()};

  m_world->accept(kdl::overload(
    [](auto&& thisLambda, mdl::WorldNode* world) { world->visitChildren(thisLambda); },
    [](auto&& thisLambda, mdl::LayerNode* layer) { layer->visitChildren(thisLambda); },
    [](auto&& thisLambda, mdl::GroupNode* group) { group->visitChildren(thisLambda); },
    [](auto&& thisLambda, mdl::EntityNode* entity) { entity->visitChildren(thisLambda); },
    [&](mdl::BrushNode* brushNode) {
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
    [](mdl::PatchNode*) {}));
}

bool MapDocument::persistent() const
{
  return m_path.is_absolute() && io::Disk::pathInfo(m_path) == io::PathInfo::File;
}

std::string MapDocument::filename() const
{
  if (m_path.empty())
  {
    return "";
  }
  return m_path.filename().string();
}

const std::filesystem::path& MapDocument::path() const
{
  return m_path;
}

void MapDocument::setPath(const std::filesystem::path& path)
{
  m_path = path;
}

bool MapDocument::modified() const
{
  return m_modificationCount != m_lastSaveModificationCount;
}

size_t MapDocument::modificationCount() const
{
  return m_modificationCount;
}

void MapDocument::setLastSaveModificationCount()
{
  m_lastSaveModificationCount = m_modificationCount;
  documentModificationStateDidChangeNotifier();
}

void MapDocument::clearModificationCount()
{
  m_lastSaveModificationCount = m_modificationCount = 0;
  documentModificationStateDidChangeNotifier();
}

void MapDocument::connectObservers()
{
  m_notifierConnection += materialCollectionsWillChangeNotifier.connect(
    this, &MapDocument::materialCollectionsWillChange);
  m_notifierConnection += materialCollectionsDidChangeNotifier.connect(
    this, &MapDocument::materialCollectionsDidChange);

  m_notifierConnection += entityDefinitionsWillChangeNotifier.connect(
    this, &MapDocument::entityDefinitionsWillChange);
  m_notifierConnection += entityDefinitionsDidChangeNotifier.connect(
    this, &MapDocument::entityDefinitionsDidChange);

  m_notifierConnection +=
    modsWillChangeNotifier.connect(this, &MapDocument::modsWillChange);
  m_notifierConnection +=
    modsDidChangeNotifier.connect(this, &MapDocument::modsDidChange);

  PreferenceManager& prefs = PreferenceManager::instance();
  m_notifierConnection +=
    prefs.preferenceDidChangeNotifier.connect(this, &MapDocument::preferenceDidChange);
  m_notifierConnection += m_editorContext->editorContextDidChangeNotifier.connect(
    editorContextDidChangeNotifier);
  m_notifierConnection += commandDoneNotifier.connect(this, &MapDocument::commandDone);
  m_notifierConnection +=
    commandUndoneNotifier.connect(this, &MapDocument::commandUndone);
  m_notifierConnection +=
    transactionDoneNotifier.connect(this, &MapDocument::transactionDone);
  m_notifierConnection +=
    transactionUndoneNotifier.connect(this, &MapDocument::transactionUndone);

  // tag management
  m_notifierConnection +=
    documentWasNewedNotifier.connect(this, &MapDocument::initializeAllNodeTags);
  m_notifierConnection +=
    documentWasLoadedNotifier.connect(this, &MapDocument::initializeAllNodeTags);
  m_notifierConnection +=
    nodesWereAddedNotifier.connect(this, &MapDocument::initializeNodeTags);
  m_notifierConnection +=
    nodesWillBeRemovedNotifier.connect(this, &MapDocument::clearNodeTags);
  m_notifierConnection +=
    nodesDidChangeNotifier.connect(this, &MapDocument::updateNodeTags);
  m_notifierConnection +=
    brushFacesDidChangeNotifier.connect(this, &MapDocument::updateFaceTags);
  m_notifierConnection +=
    modsDidChangeNotifier.connect(this, &MapDocument::updateAllFaceTags);
  m_notifierConnection += resourcesWereProcessedNotifier.connect(
    this, &MapDocument::updateFaceTagsAfterResourcesWhereProcessed);
}

void MapDocument::materialCollectionsWillChange()
{
  unsetMaterials();
}

void MapDocument::materialCollectionsDidChange()
{
  loadMaterials();
  setMaterials();
  updateAllFaceTags();
}

void MapDocument::entityDefinitionsWillChange()
{
  unloadEntityDefinitions();
  clearEntityModels();
}

void MapDocument::entityDefinitionsDidChange()
{
  loadEntityDefinitions();
  setEntityDefinitions();
  setEntityModels();
}

void MapDocument::modsWillChange()
{
  unsetEntityModels();
  unsetEntityDefinitions();
  clearEntityModels();
}

void MapDocument::modsDidChange()
{
  updateGameSearchPaths();
  setEntityDefinitions();
  setEntityModels();
}

void MapDocument::preferenceDidChange(const std::filesystem::path& path)
{
  if (isGamePathPreference(path))
  {
    const mdl::GameFactory& gameFactory = mdl::GameFactory::instance();
    const std::filesystem::path newGamePath = gameFactory.gamePath(m_game->config().name);
    m_game->setGamePath(newGamePath, logger());

    clearEntityModels();
    setEntityModels();

    reloadMaterials();
    setMaterials();
  }
}

void MapDocument::commandDone(Command& command)
{
  debug() << "Command '" << command.name() << "' executed";
}

void MapDocument::commandUndone(UndoableCommand& command)
{
  debug() << "Command '" << command.name() << "' undone";
}

void MapDocument::transactionDone(const std::string& name)
{
  debug() << "Transaction '" << name << "' executed";
}

void MapDocument::transactionUndone(const std::string& name)
{
  debug() << "Transaction '" << name << "' undone";
}

} // namespace tb::ui
