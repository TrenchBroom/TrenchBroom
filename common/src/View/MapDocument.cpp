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

#include "View/MapDocument.h"

#include "Assets/AssetUtils.h"
#include "Assets/EntityDefinition.h"
#include "Assets/EntityDefinitionFileSpec.h"
#include "Assets/EntityDefinitionGroup.h"
#include "Assets/EntityDefinitionManager.h"
#include "Assets/EntityModelManager.h"
#include "Assets/Texture.h"
#include "Assets/TextureManager.h"
#include "EL/ELExceptions.h"
#include "Exceptions.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/ExportOptions.h"
#include "IO/GameConfigParser.h"
#include "IO/IOUtils.h"
#include "IO/SimpleParserStatus.h"
#include "IO/SystemPaths.h"
#include "Model/BezierPatch.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushError.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/BrushNode.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/EditorContext.h"
#include "Model/EmptyBrushEntityIssueGenerator.h"
#include "Model/EmptyGroupIssueGenerator.h"
#include "Model/EmptyPropertyKeyIssueGenerator.h"
#include "Model/EmptyPropertyValueIssueGenerator.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/EntityProperties.h"
#include "Model/Game.h"
#include "Model/GameFactory.h"
#include "Model/GroupNode.h"
#include "Model/InvalidTextureScaleIssueGenerator.h"
#include "Model/LayerNode.h"
#include "Model/LinkSourceIssueGenerator.h"
#include "Model/LinkTargetIssueGenerator.h"
#include "Model/LockState.h"
#include "Model/LongPropertyKeyIssueGenerator.h"
#include "Model/LongPropertyValueIssueGenerator.h"
#include "Model/MissingClassnameIssueGenerator.h"
#include "Model/MissingDefinitionIssueGenerator.h"
#include "Model/MissingModIssueGenerator.h"
#include "Model/MixedBrushContentsIssueGenerator.h"
#include "Model/ModelUtils.h"
#include "Model/Node.h"
#include "Model/NodeContents.h"
#include "Model/NonIntegerVerticesIssueGenerator.h"
#include "Model/PatchNode.h"
#include "Model/PointEntityWithBrushesIssueGenerator.h"
#include "Model/Polyhedron.h"
#include "Model/Polyhedron3.h"
#include "Model/PortalFile.h"
#include "Model/PropertyKeyWithDoubleQuotationMarksIssueGenerator.h"
#include "Model/PropertyValueWithDoubleQuotationMarksIssueGenerator.h"
#include "Model/SoftMapBoundsIssueGenerator.h"
#include "Model/TagManager.h"
#include "Model/VisibilityState.h"
#include "Model/WorldBoundsIssueGenerator.h"
#include "Model/WorldNode.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Uuid.h"
#include "View/Actions.h"
#include "View/AddRemoveNodesCommand.h"
#include "View/BrushVertexCommands.h"
#include "View/CurrentGroupCommand.h"
#include "View/Grid.h"
#include "View/MapTextEncoding.h"
#include "View/PasteType.h"
#include "View/ReparentNodesCommand.h"
#include "View/RepeatStack.h"
#include "View/SelectionCommand.h"
#include "View/SetCurrentLayerCommand.h"
#include "View/SetLockStateCommand.h"
#include "View/SetVisibilityCommand.h"
#include "View/SwapNodeContentsCommand.h"
#include "View/UpdateLinkedGroupsHelper.h"
#include "View/ViewEffectsService.h"

#include <kdl/collection_utils.h>
#include <kdl/map_utils.h>
#include <kdl/memory_utils.h>
#include <kdl/overload.h>
#include <kdl/parallel.h>
#include <kdl/result.h>
#include <kdl/result_for_each.h>
#include <kdl/string_format.h>
#include <kdl/vector_set.h>
#include <kdl/vector_utils.h>

#include <vecmath/polygon.h>
#include <vecmath/util.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include <algorithm>
#include <cassert>
#include <cstdlib> // for std::abs
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace TrenchBroom {
namespace View {
template <typename T>
static auto findLinkedGroupsToUpdate(
  Model::WorldNode& worldNode, const std::vector<T*>& nodes, const bool includeGivenNodes) {
  auto result = std::vector<std::pair<const Model::GroupNode*, std::vector<Model::GroupNode*>>>{};

  const auto addGroupNode = [&](const Model::GroupNode* groupNode) {
    while (groupNode) {
      if (const auto linkedGroupId = groupNode->group().linkedGroupId()) {
        auto linkedGroups = Model::findLinkedGroups(worldNode, *linkedGroupId);
        if (linkedGroups.size() > 1u) {
          result.emplace_back(groupNode, std::move(linkedGroups));
          return;
        }
      }
      groupNode = groupNode->containingGroup();
    }
  };

  Model::Node::visitAll(
    nodes, kdl::overload(
             [](const Model::WorldNode*) {}, [](const Model::LayerNode*) {},
             [&](Model::GroupNode* groupNode) {
               if (includeGivenNodes) {
                 addGroupNode(groupNode);
               } else {
                 addGroupNode(groupNode->containingGroup());
               }
             },
             [&](Model::EntityNode* entityNode) {
               addGroupNode(entityNode->containingGroup());
             },
             [&](Model::BrushNode* brushNode) {
               addGroupNode(brushNode->containingGroup());
             },
             [&](Model::PatchNode* patchNode) {
               addGroupNode(patchNode->containingGroup());
             }));

  return kdl::vec_sort_and_remove_duplicates(std::move(result));
}

template <typename T>
static auto findContainingLinkedGroupsToUpdate(
  Model::WorldNode& worldNode, const std::vector<T*>& nodes) {
  return findLinkedGroupsToUpdate(worldNode, nodes, false);
}

template <typename T>
static auto findAllLinkedGroupsToUpdate(Model::WorldNode& worldNode, const std::vector<T*>& nodes) {
  return findLinkedGroupsToUpdate(worldNode, nodes, true);
}

/**
 * Applies the given lambda to a copy of the contents of each of the given nodes and returns a
 * vector of pairs of the original node and the modified contents.
 *
 * The lambda L needs three overloads:
 * - bool operator()(Model::Entity&);
 * - bool operator()(Model::Brush&);
 * - bool operator()(Model::BezierPatch&);
 *
 * The given node contents should be modified in place and the lambda should return true if it was
 * applied successfully and false otherwise.
 *
 * Returns a vector of pairs which map each node to its modified contents if the lambda succeeded
 * for every given node, or an empty optional otherwise.
 */
template <typename N, typename L>
static std::optional<std::vector<std::pair<Model::Node*, Model::NodeContents>>> applyToNodeContents(
  const std::vector<N*>& nodes, L lambda) {
  using NodeContentType =
    std::variant<Model::Layer, Model::Group, Model::Entity, Model::Brush, Model::BezierPatch>;

  auto newNodes = std::vector<std::pair<Model::Node*, Model::NodeContents>>{};
  newNodes.reserve(nodes.size());

  bool success = true;
  std::transform(std::begin(nodes), std::end(nodes), std::back_inserter(newNodes), [&](auto* node) {
    NodeContentType nodeContents = node->accept(kdl::overload(
      [](const Model::WorldNode* worldNode) -> NodeContentType {
        return worldNode->entity();
      },
      [](const Model::LayerNode* layerNode) -> NodeContentType {
        return layerNode->layer();
      },
      [](const Model::GroupNode* groupNode) -> NodeContentType {
        return groupNode->group();
      },
      [](const Model::EntityNode* entityNode) -> NodeContentType {
        return entityNode->entity();
      },
      [](const Model::BrushNode* brushNode) -> NodeContentType {
        return brushNode->brush();
      },
      [](const Model::PatchNode* patchNode) -> NodeContentType {
        return patchNode->patch();
      }));

    success = success && std::visit(lambda, nodeContents);
    return std::make_pair(node, Model::NodeContents(std::move(nodeContents)));
  });

  return success ? std::make_optional(newNodes) : std::nullopt;
}

/**
 * Applies the given lambda to a copy of the contents of each of the given nodes and swaps the node
 * contents if the given lambda succeeds for all node contents.
 *
 * The lambda L needs three overloads:
 * - bool operator()(Model::Entity&);
 * - bool operator()(Model::Brush&);
 * - bool operator()(Model::BezierPatch&);
 *
 * The given node contents should be modified in place and the lambda should return true if it was
 * applied successfully and false otherwise.
 *
 * For each linked group in the given list of linked groups, its changes are distributed to the
 * connected members of its link set.
 *
 * Returns true if the given lambda could be applied successfully to all node contents and false
 * otherwise. If the lambda fails, then no node contents will be swapped, and the original nodes
 * remain unmodified.
 */
template <typename N, typename L>
static bool applyAndSwap(
  MapDocument& document, const std::string& commandName, const std::vector<N*>& nodes,
  std::vector<std::pair<const Model::GroupNode*, std::vector<Model::GroupNode*>>>
    linkedGroupsToUpdate,
  L lambda) {
  if (nodes.empty()) {
    return true;
  }

  if (auto newNodes = applyToNodeContents(nodes, std::move(lambda))) {
    return document.swapNodeContents(
      commandName, std::move(*newNodes), std::move(linkedGroupsToUpdate));
  }

  return false;
}

/**
 * Applies the given lambda to a copy of each of the given faces.
 *
 * Specifically, each brush node of the given faces has its contents copied and the lambda applied
 * to the copied faces. If the lambda succeeds for each face, the node contents are subsequently
 * swapped.
 *
 * The lambda L needs to accept brush faces:
 * - bool operator()(Model::BrushFace&);
 *
 * The given node contents should be modified in place and the lambda should return true if it was
 * applied successfully and false otherwise.
 *
 * For each linked group in the given list of linked groups, its changes are distributed to the
 * connected members of its link set.
 *
 * Returns true if the given lambda could be applied successfully to each face and false otherwise.
 * If the lambda fails, then no node contents will be swapped, and the original nodes remain
 * unmodified.
 */
template <typename L>
static bool applyAndSwap(
  MapDocument& document, const std::string& commandName,
  const std::vector<Model::BrushFaceHandle>& faces, L lambda) {
  if (faces.empty()) {
    return true;
  }

  auto brushes = std::unordered_map<Model::BrushNode*, Model::Brush>{};

  bool success = true;
  std::for_each(std::begin(faces), std::end(faces), [&](const auto& faceHandle) {
    auto* brushNode = faceHandle.node();
    auto it = brushes.find(brushNode);
    if (it == std::end(brushes)) {
      it = brushes.emplace(brushNode, brushNode->brush()).first;
    }

    auto& brush = it->second;
    success = success && lambda(brush.face(faceHandle.faceIndex()));
  });

  if (success) {
    auto newNodes = std::vector<std::pair<Model::Node*, Model::NodeContents>>{};
    newNodes.reserve(brushes.size());

    for (auto& [brushNode, brush] : brushes) {
      newNodes.emplace_back(brushNode, Model::NodeContents(std::move(brush)));
    }

    auto linkedGroupsToUpdate = findContainingLinkedGroupsToUpdate(
      *document.world(), kdl::vec_transform(newNodes, [](const auto& p) {
        return p.first;
      }));
    document.swapNodeContents(commandName, std::move(newNodes), std::move(linkedGroupsToUpdate));
  }

  return success;
}

const vm::bbox3 MapDocument::DefaultWorldBounds(-32768.0, 32768.0);
const std::string MapDocument::DefaultDocumentName("unnamed.map");

MapDocument::MapDocument()
  : m_worldBounds(DefaultWorldBounds)
  , m_world(nullptr)
  , m_portalFile(nullptr)
  , m_entityDefinitionManager(std::make_unique<Assets::EntityDefinitionManager>())
  , m_entityModelManager(std::make_unique<Assets::EntityModelManager>(
      pref(Preferences::TextureMagFilter), pref(Preferences::TextureMinFilter), logger()))
  , m_textureManager(std::make_unique<Assets::TextureManager>(
      pref(Preferences::TextureMagFilter), pref(Preferences::TextureMinFilter), logger()))
  , m_tagManager(std::make_unique<Model::TagManager>())
  , m_editorContext(std::make_unique<Model::EditorContext>())
  , m_grid(std::make_unique<Grid>(4))
  , m_path(DefaultDocumentName)
  , m_lastSaveModificationCount(0)
  , m_modificationCount(0)
  , m_currentLayer(nullptr)
  , m_currentTextureName(Model::BrushFaceAttributes::NoTextureName)
  , m_lastSelectionBounds(0.0, 32.0)
  , m_selectionBoundsValid(true)
  , m_viewEffectsService(nullptr)
  , m_repeatStack(std::make_unique<RepeatStack>()) {
  connectObservers();
}

MapDocument::~MapDocument() {
  if (isPointFileLoaded()) {
    unloadPointFile();
  }
  if (isPortalFileLoaded()) {
    unloadPortalFile();
  }
  clearWorld();
}

Logger& MapDocument::logger() {
  return *this;
}

std::shared_ptr<Model::Game> MapDocument::game() const {
  return m_game;
}

const vm::bbox3& MapDocument::worldBounds() const {
  return m_worldBounds;
}

Model::WorldNode* MapDocument::world() const {
  return m_world.get();
}

bool MapDocument::isGamePathPreference(const IO::Path& path) const {
  return m_game.get() != nullptr && m_game->isGamePathPreference(path);
}

Model::LayerNode* MapDocument::currentLayer() const {
  ensure(m_currentLayer != nullptr, "currentLayer is null");
  return m_currentLayer;
}

/**
 * Sets the current layer immediately, without adding a Command to the undo stack.
 */
Model::LayerNode* MapDocument::performSetCurrentLayer(Model::LayerNode* currentLayer) {
  ensure(currentLayer != nullptr, "currentLayer is null");

  Model::LayerNode* oldCurrentLayer = m_currentLayer;
  m_currentLayer = currentLayer;
  currentLayerDidChangeNotifier(m_currentLayer);

  return oldCurrentLayer;
}

void MapDocument::setCurrentLayer(Model::LayerNode* currentLayer) {
  ensure(m_currentLayer != nullptr, "old currentLayer is null");
  ensure(currentLayer != nullptr, "new currentLayer is null");

  Transaction transaction(this, "Set Current Layer");

  while (currentGroup() != nullptr) {
    closeGroup();
  }

  const auto descendants = Model::collectDescendants({m_currentLayer});
  downgradeShownToInherit(descendants);
  downgradeUnlockedToInherit(descendants);

  executeAndStore(SetCurrentLayerCommand::set(currentLayer));
}

bool MapDocument::canSetCurrentLayer(Model::LayerNode* currentLayer) const {
  return m_currentLayer != currentLayer;
}

Model::GroupNode* MapDocument::currentGroup() const {
  return m_editorContext->currentGroup();
}

Model::Node* MapDocument::currentGroupOrWorld() const {
  Model::Node* result = currentGroup();
  if (result == nullptr) {
    result = m_world.get();
  }
  return result;
}

Model::Node* MapDocument::parentForNodes(const std::vector<Model::Node*>& nodes) const {
  if (nodes.empty()) {
    // No reference nodes, so return either the current group (if open) or current layer
    Model::Node* result = currentGroup();
    if (result == nullptr) {
      result = currentLayer();
    }
    return result;
  }

  Model::GroupNode* parentGroup = Model::findContainingGroup(nodes.at(0));
  if (parentGroup != nullptr) {
    return parentGroup;
  }

  Model::LayerNode* parentLayer = Model::findContainingLayer(nodes.at(0));
  ensure(parentLayer != nullptr, "no parent layer");
  return parentLayer;
}

Model::EditorContext& MapDocument::editorContext() const {
  return *m_editorContext;
}

Assets::EntityDefinitionManager& MapDocument::entityDefinitionManager() {
  return *m_entityDefinitionManager;
}

Assets::EntityModelManager& MapDocument::entityModelManager() {
  return *m_entityModelManager;
}

Assets::TextureManager& MapDocument::textureManager() {
  return *m_textureManager;
}

Grid& MapDocument::grid() const {
  return *m_grid;
}

std::optional<PointFile>& MapDocument::pointFile() {
  return m_pointFile;
}

Model::PortalFile* MapDocument::portalFile() const {
  return m_portalFile.get();
}

void MapDocument::setViewEffectsService(ViewEffectsService* viewEffectsService) {
  m_viewEffectsService = viewEffectsService;
}

void MapDocument::createTagActions() {
  const auto& actionManager = ActionManager::instance();
  m_tagActions = actionManager.createTagActions(m_tagManager->smartTags());
}

void MapDocument::clearTagActions() {
  m_tagActions.clear();
}

void MapDocument::createEntityDefinitionActions() {
  const auto& actionManager = ActionManager::instance();
  m_entityDefinitionActions =
    actionManager.createEntityDefinitionActions(m_entityDefinitionManager->definitions());
}

void MapDocument::newDocument(
  const Model::MapFormat mapFormat, const vm::bbox3& worldBounds,
  std::shared_ptr<Model::Game> game) {
  info("Creating new document");

  clearRepeatableCommands();
  doClearCommandProcessor();
  clearDocument();
  createWorld(mapFormat, worldBounds, game);

  loadAssets();
  registerIssueGenerators();
  registerSmartTags();
  createTagActions();

  clearModificationCount();

  documentWasNewedNotifier(this);
}

void MapDocument::loadDocument(
  const Model::MapFormat mapFormat, const vm::bbox3& worldBounds, std::shared_ptr<Model::Game> game,
  const IO::Path& path) {
  info("Loading document from " + path.asString());

  clearRepeatableCommands();
  doClearCommandProcessor();
  clearDocument();
  loadWorld(mapFormat, worldBounds, game, path);

  loadAssets();
  registerIssueGenerators();
  registerSmartTags();
  createTagActions();

  documentWasLoadedNotifier(this);
}

void MapDocument::saveDocument() {
  doSaveDocument(m_path);
}

void MapDocument::saveDocumentAs(const IO::Path& path) {
  doSaveDocument(path);
}

void MapDocument::saveDocumentTo(const IO::Path& path) {
  ensure(m_game.get() != nullptr, "game is null");
  ensure(m_world != nullptr, "world is null");
  m_game->writeMap(*m_world, path);
}

void MapDocument::exportDocumentAs(const IO::ExportOptions& options) {
  m_game->exportMap(*m_world, options);
}

void MapDocument::doSaveDocument(const IO::Path& path) {
  saveDocumentTo(path);
  setLastSaveModificationCount();
  setPath(path);
  documentWasSavedNotifier(this);
}

void MapDocument::clearDocument() {
  if (m_world != nullptr) {
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

MapTextEncoding MapDocument::encoding() const {
  return MapTextEncoding::Quake;
}

std::string MapDocument::serializeSelectedNodes() {
  std::stringstream stream;
  m_game->writeNodesToStream(*m_world, m_selectedNodes.nodes(), stream);
  return stream.str();
}

std::string MapDocument::serializeSelectedBrushFaces() {
  std::stringstream stream;
  const auto faces = kdl::vec_transform(m_selectedBrushFaces, [](const auto& h) {
    return h.face();
  });
  m_game->writeBrushFacesToStream(*m_world, faces, stream);
  return stream.str();
}

PasteType MapDocument::paste(const std::string& str) {
  // Try parsing as entities, then as brushes, in all compatible formats
  const std::vector<Model::Node*> nodes =
    m_game->parseNodes(str, m_world->mapFormat(), m_worldBounds, logger());
  if (!nodes.empty()) {
    if (pasteNodes(nodes)) {
      return PasteType::Node;
    }
    return PasteType::Failed;
  }

  // Try parsing as brush faces
  try {
    const std::vector<Model::BrushFace> faces =
      m_game->parseBrushFaces(str, m_world->mapFormat(), m_worldBounds, logger());
    if (!faces.empty() && pasteBrushFaces(faces)) {
      return PasteType::BrushFace;
    }
  } catch (const ParserException& e) {
    error() << "Could not parse clipboard contents: " << e.what();
  }
  return PasteType::Failed;
}

static std::vector<Model::IdType> allPersistentGroupIds(const Model::Node& root) {
  auto result = std::vector<Model::IdType>{};
  root.accept(kdl::overload(
    [](auto&& thisLambda, const Model::WorldNode* worldNode) {
      worldNode->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, const Model::LayerNode* layerNode) {
      layerNode->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, const Model::GroupNode* groupNode) {
      if (const auto persistentId = groupNode->persistentId()) {
        result.push_back(*persistentId);
      }
      groupNode->visitChildren(thisLambda);
    },
    [](const Model::EntityNode*) {}, [](const Model::BrushNode*) {},
    [](const Model::PatchNode*) {}));
  return result;
}

bool MapDocument::pasteNodes(const std::vector<Model::Node*>& nodes) {
  auto nodesToDetach = std::vector<Model::Node*>{};
  auto nodesToDelete = std::vector<Model::Node*>{};
  auto nodesToAdd = std::map<Model::Node*, std::vector<Model::Node*>>{};

  auto* parent = parentForNodes();
  for (auto* node : nodes) {
    node->accept(kdl::overload(
      [&](auto&& thisLambda, Model::WorldNode* world) {
        world->visitChildren(thisLambda);
        nodesToDelete.push_back(world);
      },
      [&](auto&& thisLambda, Model::LayerNode* layer) {
        layer->visitChildren(thisLambda);
        nodesToDetach.push_back(layer);
        nodesToDelete.push_back(layer);
      },
      [&](Model::GroupNode* group) {
        nodesToDetach.push_back(group);
        nodesToAdd[parent].push_back(group);
      },
      [&](auto&& thisLambda, Model::EntityNode* entityNode) {
        if (Model::isWorldspawn(
              entityNode->entity().classname(), entityNode->entity().properties())) {
          entityNode->visitChildren(thisLambda);
          nodesToDetach.push_back(entityNode);
          nodesToDelete.push_back(entityNode);
        } else {
          nodesToDetach.push_back(entityNode);
          nodesToAdd[parent].push_back(entityNode);
        }
      },
      [&](Model::BrushNode* brush) {
        nodesToDetach.push_back(brush);
        nodesToAdd[parent].push_back(brush);
      },
      [&](Model::PatchNode* patch) {
        nodesToDetach.push_back(patch);
        nodesToAdd[parent].push_back(patch);
      }));
  }

  for (auto* node : nodesToDetach) {
    auto* nodeParent = node->parent();
    if (nodeParent != nullptr) {
      nodeParent->removeChild(node);
    }
  }
  kdl::vec_clear_and_delete(nodesToDelete);

  // Clean up persistent IDs of any groups being pasted
  auto persistentGroupIds = kdl::vector_set(allPersistentGroupIds(*m_world.get()));
  for (auto& [newParent, nodesToAddToParent] : nodesToAdd) {
    for (auto* node : nodesToAddToParent) {
      node->accept(kdl::overload(
        [&](auto&& thisLambda, Model::WorldNode* worldNode) {
          worldNode->visitChildren(thisLambda);
        },
        [&](auto&& thisLambda, Model::LayerNode* layerNode) {
          layerNode->visitChildren(thisLambda);
        },
        [&](auto&& thisLambda, Model::GroupNode* groupNode) {
          if (const auto persistentGroupId = groupNode->persistentId()) {
            if (!persistentGroupIds.insert(*persistentGroupId).second) {
              // a group with this ID is already in the map or being pasted, so reset the ID
              groupNode->resetPersistentId();
            }
          }
          groupNode->visitChildren(thisLambda);
        },
        [](Model::EntityNode*) {}, [](Model::BrushNode*) {}, [](Model::PatchNode*) {}));
    }
  }

  const std::vector<Model::Node*> addedNodes = addNodes(nodesToAdd);
  if (addedNodes.empty())
    return false;

  deselectAll();

  const auto nodesToSelect = Model::collectSelectableNodes(addedNodes, editorContext());
  selectNodes(nodesToSelect);

  return true;
}

bool MapDocument::pasteBrushFaces(const std::vector<Model::BrushFace>& faces) {
  assert(!faces.empty());
  return setFaceAttributesExceptContentFlags(faces.back().attributes());
}

void MapDocument::loadPointFile(const IO::Path path) {
  static_assert(
    !std::is_reference<decltype(path)>::value,
    "path must be passed by value because reloadPointFile() passes m_pointFilePath");

  if (isPointFileLoaded()) {
    unloadPointFile();
  }

  auto file = IO::openPathAsInputStream(path);
  if (auto trace = Model::loadPointFile(file)) {
    m_pointFile = PointFile{*trace, path};
    info() << "Loaded point file " << path;
    pointFileWasLoadedNotifier();
  } else {
    warn() << "Failed to load point file " << path;
  }
}

bool MapDocument::isPointFileLoaded() const {
  return m_pointFile != std::nullopt;
}

bool MapDocument::canReloadPointFile() const {
  return isPointFileLoaded();
}

void MapDocument::reloadPointFile() {
  assert(isPointFileLoaded());
  loadPointFile(m_pointFile->path);
}

void MapDocument::unloadPointFile() {
  assert(isPointFileLoaded());
  m_pointFile = std::nullopt;

  info() << "Unloaded point file";
  pointFileWasUnloadedNotifier();
}

void MapDocument::loadPortalFile(const IO::Path path) {
  static_assert(
    !std::is_reference<decltype(path)>::value,
    "path must be passed by value because reloadPortalFile() passes m_portalFilePath");

  if (!Model::PortalFile::canLoad(path)) {
    return;
  }

  if (isPortalFileLoaded()) {
    unloadPortalFile();
  }

  try {
    m_portalFilePath = path;
    m_portalFile = std::make_unique<Model::PortalFile>(path);
  } catch (const std::exception& exception) {
    info("Couldn't load portal file " + m_portalFilePath.asString() + ": " + exception.what());
  }

  if (isPortalFileLoaded()) {
    info("Loaded portal file " + m_portalFilePath.asString());
    portalFileWasLoadedNotifier();
  }
}

bool MapDocument::isPortalFileLoaded() const {
  return m_portalFile != nullptr;
}

bool MapDocument::canReloadPortalFile() const {
  return m_portalFile != nullptr && Model::PortalFile::canLoad(m_portalFilePath);
}

void MapDocument::reloadPortalFile() {
  assert(isPortalFileLoaded());
  loadPortalFile(m_portalFilePath);
}

void MapDocument::unloadPortalFile() {
  assert(isPortalFileLoaded());
  m_portalFile = nullptr;
  m_portalFilePath = IO::Path();

  info("Unloaded portal file");
  portalFileWasUnloadedNotifier();
}

bool MapDocument::hasSelection() const {
  return hasSelectedNodes() || hasSelectedBrushFaces();
}

bool MapDocument::hasSelectedNodes() const {
  return !m_selectedNodes.empty();
}

bool MapDocument::hasSelectedBrushFaces() const {
  return !m_selectedBrushFaces.empty();
}

bool MapDocument::hasAnySelectedBrushFaces() const {
  return hasSelectedBrushFaces() || selectedNodes().hasBrushes();
}

std::vector<Model::EntityNodeBase*> MapDocument::allSelectedEntityNodes() const {
  if (!hasSelection()) {
    return m_world ? std::vector<Model::EntityNodeBase*>({m_world.get()})
                   : std::vector<Model::EntityNodeBase*>{};
  }

  std::vector<Model::EntityNodeBase*> nodes;
  const auto addEntity = [&](auto* entity) {
    ensure(entity != nullptr, "entity is null");
    nodes.push_back(entity);
  };

  for (auto* node : m_selectedNodes) {
    node->accept(kdl::overload(
      [&](auto&& thisLambda, Model::WorldNode* world) {
        nodes.push_back(world);
        world->visitChildren(thisLambda);
      },
      [&](auto&& thisLambda, Model::LayerNode* layer) {
        layer->visitChildren(thisLambda);
      },
      [&](auto&& thisLambda, Model::GroupNode* group) {
        group->visitChildren(thisLambda);
      },
      [&](Model::EntityNode* entity) {
        addEntity(entity);
      },
      [&](Model::BrushNode* brush) {
        addEntity(brush->entity());
      },
      [&](Model::PatchNode* patch) {
        addEntity(patch->entity());
      }));
  }

  return kdl::vec_sort_and_remove_duplicates(std::move(nodes));
}

std::vector<Model::BrushNode*> MapDocument::allSelectedBrushNodes() const {
  auto brushes = std::vector<Model::BrushNode*>{};
  for (auto* node : m_selectedNodes.nodes()) {
    node->accept(kdl::overload(
      [](auto&& thisLambda, Model::WorldNode* world) {
        world->visitChildren(thisLambda);
      },
      [](auto&& thisLambda, Model::LayerNode* layer) {
        layer->visitChildren(thisLambda);
      },
      [](auto&& thisLambda, Model::GroupNode* group) {
        group->visitChildren(thisLambda);
      },
      [](auto&& thisLambda, Model::EntityNode* entity) {
        entity->visitChildren(thisLambda);
      },
      [&](Model::BrushNode* brush) {
        brushes.push_back(brush);
      },
      [&](Model::PatchNode*) {}));
  }
  return brushes;
}

bool MapDocument::hasAnySelectedBrushNodes() const {
  // This is just an optimization of `!allSelectedBrushNodes().empty()`
  // that stops after finding the first brush
  const auto visitChildrenAndExitEarly = [](auto&& thisLambda, const auto* node) {
    for (const auto* child : node->children()) {
      if (child->accept(thisLambda)) {
        return true;
      }
    }
    return false;
  };

  for (const auto* node : m_selectedNodes.nodes()) {
    const auto hasBrush = node->accept(kdl::overload(
      [&](auto&& thisLambda, const Model::WorldNode* world) -> bool {
        return visitChildrenAndExitEarly(thisLambda, world);
      },
      [&](auto&& thisLambda, const Model::LayerNode* layer) -> bool {
        return visitChildrenAndExitEarly(thisLambda, layer);
      },
      [&](auto&& thisLambda, const Model::GroupNode* group) -> bool {
        return visitChildrenAndExitEarly(thisLambda, group);
      },
      [&](auto&& thisLambda, const Model::EntityNode* entity) -> bool {
        return visitChildrenAndExitEarly(thisLambda, entity);
      },
      [](const Model::BrushNode*) -> bool {
        return true;
      },
      [](const Model::PatchNode*) -> bool {
        return false;
      }));
    if (hasBrush) {
      return true;
    }
  }

  return false;
}

const Model::NodeCollection& MapDocument::selectedNodes() const {
  return m_selectedNodes;
}

std::vector<Model::BrushFaceHandle> MapDocument::allSelectedBrushFaces() const {
  if (hasSelectedBrushFaces())
    return selectedBrushFaces();

  const auto faces = Model::collectBrushFaces(m_selectedNodes.nodes());
  return Model::faceSelectionWithLinkedGroupConstraints(*m_world.get(), faces).facesToSelect;
}

std::vector<Model::BrushFaceHandle> MapDocument::selectedBrushFaces() const {
  return m_selectedBrushFaces;
}

const vm::bbox3& MapDocument::referenceBounds() const {
  if (hasSelectedNodes())
    return selectionBounds();
  return lastSelectionBounds();
}

const vm::bbox3& MapDocument::lastSelectionBounds() const {
  return m_lastSelectionBounds;
}

const vm::bbox3& MapDocument::selectionBounds() const {
  if (!m_selectionBoundsValid)
    validateSelectionBounds();
  return m_selectionBounds;
}

const std::string& MapDocument::currentTextureName() const {
  return m_currentTextureName;
}

void MapDocument::setCurrentTextureName(const std::string& currentTextureName) {
  if (m_currentTextureName == currentTextureName)
    return;
  m_currentTextureName = currentTextureName;
  currentTextureNameDidChangeNotifier(m_currentTextureName);
}

void MapDocument::selectAllNodes() {
  m_repeatStack->clearOnNextPush();
  executeAndStore(SelectionCommand::selectAllNodes());
}

void MapDocument::selectSiblings() {
  const std::vector<Model::Node*>& nodes = selectedNodes().nodes();
  if (nodes.empty()) {
    return;
  }

  auto visited = std::unordered_set<Model::Node*>{};
  auto nodesToSelect = std::vector<Model::Node*>{};

  for (auto* node : nodes) {
    auto* parent = node->parent();
    if (visited.insert(parent).second) {
      nodesToSelect = kdl::vec_concat(
        std::move(nodesToSelect),
        Model::collectSelectableNodes(parent->children(), editorContext()));
    }
  }

  Transaction transaction(this, "Select Siblings");
  deselectAll();
  selectNodes(nodesToSelect);
}

void MapDocument::selectTouching(const bool del) {
  const auto nodes = kdl::vec_filter(
    Model::collectTouchingNodes(
      std::vector<Model::Node*>{m_world.get()}, m_selectedNodes.brushes()),
    [&](Model::Node* node) {
      return m_editorContext->selectable(node);
    });

  Transaction transaction(this, "Select Touching");
  if (del)
    deleteObjects();
  else
    deselectAll();
  selectNodes(nodes);
}

void MapDocument::selectInside(const bool del) {
  const auto nodes = kdl::vec_filter(
    Model::collectContainedNodes(
      std::vector<Model::Node*>{m_world.get()}, m_selectedNodes.brushes()),
    [&](Model::Node* node) {
      return m_editorContext->selectable(node);
    });

  Transaction transaction(this, "Select Inside");
  if (del)
    deleteObjects();
  else
    deselectAll();
  selectNodes(nodes);
}

void MapDocument::selectInverse() {
  // This only selects nodes that have no selected children (or parents).
  // This is because if a brush entity only 1 selected child and 1 unselected,
  // we treat it as partially selected and don't want to try to select the entity if the
  // selection is inverted, which would reselect both children.

  auto nodesToSelect = std::vector<Model::Node*>{};
  const auto collectNode = [&](auto* node) {
    if (
      !node->transitivelySelected() && !node->descendantSelected() &&
      m_editorContext->selectable(node)) {
      nodesToSelect.push_back(node);
    }
  };

  currentGroupOrWorld()->accept(kdl::overload(
    [](auto&& thisLambda, Model::WorldNode* world) {
      world->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, Model::LayerNode* layer) {
      layer->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, Model::GroupNode* group) {
      collectNode(group);
      group->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, Model::EntityNode* entity) {
      collectNode(entity);
      entity->visitChildren(thisLambda);
    },
    [&](Model::BrushNode* brush) {
      collectNode(brush);
    },
    [&](Model::PatchNode* patch) {
      collectNode(patch);
    }));

  Transaction transaction(this, "Select Inverse");
  deselectAll();
  selectNodes(nodesToSelect);
}

void MapDocument::selectNodesWithFilePosition(const std::vector<size_t>& positions) {
  const auto nodes = kdl::vec_filter(
    Model::collectSelectableNodes(std::vector<Model::Node*>{m_world.get()}, *m_editorContext),
    [&](const auto* node) {
      for (const size_t position : positions) {
        if (node->containsLine(position)) {
          return true;
        }
      }
      return false;
    });

  Transaction transaction(this, "Select by Line Number");
  deselectAll();
  selectNodes(nodes);
}

void MapDocument::selectNodes(const std::vector<Model::Node*>& nodes) {
  m_repeatStack->clearOnNextPush();
  executeAndStore(SelectionCommand::select(nodes));
}

void MapDocument::selectBrushFaces(const std::vector<Model::BrushFaceHandle>& handles) {
  m_repeatStack->clearOnNextPush();
  executeAndStore(SelectionCommand::select(handles));
  if (!handles.empty()) {
    setCurrentTextureName(handles.back().face().attributes().textureName());
  }
}

void MapDocument::convertToFaceSelection() {
  m_repeatStack->clearOnNextPush();
  executeAndStore(SelectionCommand::convertToFaces());
}

void MapDocument::selectFacesWithTexture(const Assets::Texture* texture) {
  const auto faces = kdl::vec_filter(
    Model::collectSelectableBrushFaces(std::vector<Model::Node*>{m_world.get()}, *m_editorContext),
    [&](const auto& faceHandle) {
      return faceHandle.face().texture() == texture;
    });

  Transaction transaction(this, "Select Faces with Texture");
  deselectAll();
  selectBrushFaces(faces);
}

void MapDocument::selectTall(const vm::axis::type cameraAxis) {
  const vm::vec3 cameraAbsDirection = vm::vec3::axis(cameraAxis);
  const vm::bbox3 tallBounds =
    worldBounds().expand(-1.0); // we can't make a brush that is exactly as large as worldBounds

  const FloatType min = vm::dot(tallBounds.min, cameraAbsDirection);
  const FloatType max = vm::dot(tallBounds.max, cameraAbsDirection);

  const vm::plane3 minPlane(min, cameraAbsDirection);
  const vm::plane3 maxPlane(max, cameraAbsDirection);

  const std::vector<Model::BrushNode*>& selectionBrushNodes = selectedNodes().brushes();
  assert(!selectionBrushNodes.empty());

  const Model::BrushBuilder brushBuilder(world()->mapFormat(), worldBounds());

  kdl::for_each_result(
    selectionBrushNodes,
    [&](const Model::BrushNode* selectionBrushNode) {
      const Model::Brush& selectionBrush = selectionBrushNode->brush();

      std::vector<vm::vec3> tallVertices;
      tallVertices.reserve(2 * selectionBrush.vertexCount());

      for (const Model::BrushVertex* vertex : selectionBrush.vertices()) {
        tallVertices.push_back(minPlane.project_point(vertex->position()));
        tallVertices.push_back(maxPlane.project_point(vertex->position()));
      }

      return brushBuilder.createBrush(tallVertices, Model::BrushFaceAttributes::NoTextureName)
        .and_then([](Model::Brush&& brush) {
          return std::make_unique<Model::BrushNode>(std::move(brush));
        });
    })
    .and_then([&](const std::vector<std::unique_ptr<Model::BrushNode>>& tallBrushes) {
      // delete the original selection brushes before searching for the objects to select
      Transaction transaction(this, "Select Tall");
      deleteObjects();

      const auto nodesToSelect = kdl::vec_filter(
        Model::collectContainedNodes(
          std::vector<Model::Node*>{world()}, kdl::vec_transform(
                                                tallBrushes,
                                                [](const auto& b) {
                                                  return b.get();
                                                })),
        [&](const auto* node) {
          return editorContext().selectable(node);
        });
      selectNodes(nodesToSelect);
    })
    .handle_errors([&](const Model::BrushError& e) {
      logger().error() << "Could not create selection brush: " << e;
    });
}

void MapDocument::deselectAll() {
  if (hasSelection()) {
    m_repeatStack->clearOnNextPush();
    executeAndStore(SelectionCommand::deselectAll());
  }
}

void MapDocument::deselectNodes(const std::vector<Model::Node*>& nodes) {
  m_repeatStack->clearOnNextPush();
  executeAndStore(SelectionCommand::deselect(nodes));
}

void MapDocument::deselectBrushFaces(const std::vector<Model::BrushFaceHandle>& handles) {
  m_repeatStack->clearOnNextPush();
  executeAndStore(SelectionCommand::deselect(handles));
}

void MapDocument::updateLastSelectionBounds() {
  const auto currentSelectionBounds = selectionBounds();
  if (currentSelectionBounds.is_valid() && !currentSelectionBounds.is_empty()) {
    m_lastSelectionBounds = selectionBounds();
  }
}

void MapDocument::invalidateSelectionBounds() {
  m_selectionBoundsValid = false;
}

void MapDocument::validateSelectionBounds() const {
  m_selectionBounds = computeLogicalBounds(m_selectedNodes.nodes());
  m_selectionBoundsValid = true;
}

void MapDocument::clearSelection() {
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
std::vector<Model::Node*> MapDocument::addNodes(
  const std::map<Model::Node*, std::vector<Model::Node*>>& nodes) {
  for (const auto& [parent, children] : nodes) {
    assert(parent == m_world.get() || parent->isDescendantOf(m_world.get()));
    unused(parent);
  }

  Transaction transaction(this, "Add Objects");
  const auto result = executeAndStore(
    AddRemoveNodesCommand::add(nodes, findAllLinkedGroupsToUpdate(*m_world, kdl::map_keys(nodes))));
  if (!result->success()) {
    return {};
  }

  const std::vector<Model::Node*> addedNodes = collectChildren(nodes);
  ensureVisible(addedNodes);
  ensureUnlocked(addedNodes);
  return addedNodes;
}

static std::vector<std::string> getLinkedGroupIdsRecursively(
  const std::map<Model::Node*, std::vector<Model::Node*>>& parentChildrenMap) {
  std::vector<std::string> linkedGroupIds;
  for (const auto& [parent, children] : parentChildrenMap) {
    Model::Node::visitAll(
      children, kdl::overload(
                  [](auto&& thisLambda, const Model::WorldNode* worldNode) {
                    worldNode->visitChildren(thisLambda);
                  },
                  [](auto&& thisLambda, const Model::LayerNode* layerNode) {
                    layerNode->visitChildren(thisLambda);
                  },
                  [&](auto&& thisLambda, const Model::GroupNode* groupNode) {
                    if (const auto& linkedGroupId = groupNode->group().linkedGroupId()) {
                      linkedGroupIds.push_back(*linkedGroupId);
                    }
                    groupNode->visitChildren(thisLambda);
                  },
                  [](const Model::EntityNode*) {}, [](const Model::BrushNode*) {},
                  [](const Model::PatchNode*) {}));
  }

  return kdl::vec_sort_and_remove_duplicates(std::move(linkedGroupIds));
}

/**
 * Removes the given nodes. If this causes any groups/entities to become empty, removes them as
 * well.
 *
 * Ownership of the removed nodes is transferred to the undo system.
 */
void MapDocument::removeNodes(const std::vector<Model::Node*>& nodes) {
  std::map<Model::Node*, std::vector<Model::Node*>> removableNodes =
    parentChildrenMap(removeImplicitelyRemovedNodes(nodes));
  auto linkedGroupIdsOfRemovedGroups = std::vector<std::string>{};

  Transaction transaction(this);
  while (!removableNodes.empty()) {
    linkedGroupIdsOfRemovedGroups = kdl::vec_concat(
      std::move(linkedGroupIdsOfRemovedGroups), getLinkedGroupIdsRecursively(removableNodes));

    closeRemovedGroups(removableNodes);
    executeAndStore(AddRemoveNodesCommand::remove(
      removableNodes, findAllLinkedGroupsToUpdate(*m_world, kdl::map_keys(removableNodes))));

    removableNodes = collectRemovableParents(removableNodes);
  }

  auto singletonLinkSetsAfterRemoval = std::vector<Model::GroupNode*>{};
  for (const auto& linkedGroupId : linkedGroupIdsOfRemovedGroups) {
    const auto linkedGroups = Model::findLinkedGroups(*m_world, linkedGroupId);
    if (linkedGroups.size() == 1u) {
      singletonLinkSetsAfterRemoval.push_back(linkedGroups.front());
    }
  }

  applyAndSwap(
    *this, "Reset Linked Group ID", singletonLinkSetsAfterRemoval,
    findContainingLinkedGroupsToUpdate(*m_world, singletonLinkSetsAfterRemoval),
    kdl::overload(
      [](Model::Layer&) {
        return true;
      },
      [&](Model::Group& group) {
        group.resetLinkedGroupId();
        return true;
      },
      [](Model::Entity&) {
        return true;
      },
      [](Model::Brush&) {
        return true;
      },
      [](Model::BezierPatch&) {
        return true;
      }));
}

std::map<Model::Node*, std::vector<Model::Node*>> MapDocument::collectRemovableParents(
  const std::map<Model::Node*, std::vector<Model::Node*>>& nodes) const {
  std::map<Model::Node*, std::vector<Model::Node*>> result;
  for (const auto& [node, children] : nodes) {
    if (node->removeIfEmpty() && !node->hasChildren()) {
      Model::Node* parent = node->parent();
      ensure(parent != nullptr, "parent is null");
      result[parent].push_back(node);
    }
  }
  return result;
}

struct MapDocument::CompareByAncestry {
  bool operator()(const Model::Node* lhs, const Model::Node* rhs) const {
    return lhs->isAncestorOf(rhs);
  }
};

std::vector<Model::Node*> MapDocument::removeImplicitelyRemovedNodes(
  std::vector<Model::Node*> nodes) const {
  if (nodes.empty())
    return nodes;

  nodes = kdl::vec_sort(std::move(nodes), CompareByAncestry());

  std::vector<Model::Node*> result;
  result.reserve(nodes.size());
  result.push_back(nodes.front());

  for (size_t i = 1; i < nodes.size(); ++i) {
    Model::Node* node = nodes[i];
    if (!node->isDescendantOf(result))
      result.push_back(node);
  }

  return result;
}

void MapDocument::closeRemovedGroups(
  const std::map<Model::Node*, std::vector<Model::Node*>>& toRemove) {
  for (const auto& [parent, nodes] : toRemove) {
    for (const Model::Node* node : nodes) {
      if (node == currentGroup()) {
        closeGroup();
        closeRemovedGroups(toRemove);
        return;
      }
    }
  }
}

bool MapDocument::reparentNodes(
  const std::map<Model::Node*, std::vector<Model::Node*>>& nodesToAdd) {
  if (!checkReparenting(nodesToAdd))
    return false;

  std::map<Model::Node*, std::vector<Model::Node*>> nodesToRemove;
  for (const auto& [newParent, children] : nodesToAdd) {
    nodesToRemove = kdl::map_merge(nodesToRemove, Model::parentChildrenMap(children));
  }

  Transaction transaction(this, "Reparent Objects");

  // This handles two main cases:
  // - creating brushes in a hidden layer, and then grouping / ungrouping them keeps them visible
  // - creating brushes in a hidden layer, then moving them to a hidden layer, should downgrade them
  //   to inherited and hide them
  for (auto& [newParent, nodes] : nodesToAdd) {
    Model::LayerNode* newParentLayer = Model::findContainingLayer(newParent);

    const auto nodesToDowngrade = kdl::vec_filter(Model::collectNodes(nodes), [&](auto* node) {
      return Model::findContainingLayer(node) != newParentLayer;
    });

    downgradeUnlockedToInherit(nodesToDowngrade);
    downgradeShownToInherit(nodesToDowngrade);
  }

  auto linkedGroupsToUpdate = findAllLinkedGroupsToUpdate(
    *m_world, kdl::vec_concat(kdl::map_keys(nodesToAdd), kdl::map_keys(nodesToRemove)));
  const auto success =
    executeAndStore(ReparentNodesCommand::reparent(
                      std::move(nodesToAdd), nodesToRemove, std::move(linkedGroupsToUpdate)))
      ->success();
  if (success) {
    std::map<Model::Node*, std::vector<Model::Node*>> removableNodes =
      collectRemovableParents(nodesToRemove);
    while (!removableNodes.empty()) {
      closeRemovedGroups(removableNodes);
      executeAndStore(AddRemoveNodesCommand::remove(
        removableNodes, findAllLinkedGroupsToUpdate(*m_world, kdl::map_keys(removableNodes))));

      removableNodes = collectRemovableParents(removableNodes);
    }
  }

  return success;
}

bool MapDocument::checkReparenting(
  const std::map<Model::Node*, std::vector<Model::Node*>>& nodesToAdd) const {
  for (const auto& [newParent, children] : nodesToAdd) {
    if (!newParent->canAddChildren(std::begin(children), std::end(children)))
      return false;
  }
  return true;
}

bool MapDocument::deleteObjects() {
  Transaction transaction(this, "Delete Objects");
  const std::vector<Model::Node*> nodes = m_selectedNodes.nodes();
  deselectAll();
  removeNodes(nodes);
  return true;
}

/**
 * Returns whether, for UI reasons, duplicating the given node should also cause its parent to be
 * duplicated.
 *
 * Applies when duplicating a brush inside a brush entity.
 */
static bool shouldCloneParentWhenCloningNode(const Model::Node* node) {
  return node->parent()->accept(kdl::overload(
    [](const Model::WorldNode*) {
      return false;
    },
    [](const Model::LayerNode*) {
      return false;
    },
    [](const Model::GroupNode*) {
      return false;
    },
    [&](const Model::EntityNode*) {
      return true;
    },
    [](const Model::BrushNode*) {
      return false;
    },
    [](const Model::PatchNode*) {
      return false;
    }));
}

void MapDocument::duplicateObjects() {
  auto nodesToAdd = std::map<Model::Node*, std::vector<Model::Node*>>{};
  auto nodesToSelect = std::vector<Model::Node*>{};
  auto newParentMap = std::map<Model::Node*, Model::Node*>{};

  for (Model::Node* original : selectedNodes().nodes()) {
    Model::Node* suggestedParent = parentForNodes(std::vector<Model::Node*>{original});
    Model::Node* clone = original->cloneRecursively(m_worldBounds);

    if (shouldCloneParentWhenCloningNode(original)) {
      // e.g. original is a brush in a brush entity, so we need to clone the entity (parent)
      // see if the parent was already cloned and if not, clone it and store it
      Model::Node* parent = original->parent();
      Model::Node* newParent = nullptr;
      const auto it = newParentMap.find(parent);
      if (it != std::end(newParentMap)) {
        // parent was already cloned
        newParent = it->second;
      } else {
        // parent was not cloned yet
        newParent = parent->clone(m_worldBounds);
        newParentMap.insert({parent, newParent});
        nodesToAdd[suggestedParent].push_back(newParent);
      }

      // the hierarchy will look like (parent -> child): suggestedParent -> newParent -> clone
      newParent->addChild(clone);
    } else {
      nodesToAdd[suggestedParent].push_back(clone);
    }

    nodesToSelect.push_back(clone);
  }

  {
    Transaction transaction(this, "Duplicate Objects");
    deselectAll();
    addNodes(nodesToAdd);
    selectNodes(nodesToSelect);
  }

  if (m_viewEffectsService) {
    m_viewEffectsService->flashSelection();
  }
  m_repeatStack->push([=]() {
    this->duplicateObjects();
  });
}

Model::EntityNode* MapDocument::createPointEntity(
  const Assets::PointEntityDefinition* definition, const vm::vec3& delta) {
  ensure(definition != nullptr, "definition is null");

  auto* entityNode = new Model::EntityNode{Model::Entity{
    m_world->entityPropertyConfig(), {{Model::EntityPropertyKeys::Classname, definition->name()}}}};

  std::stringstream name;
  name << "Create " << definition->name();

  const Transaction transaction(this, name.str());
  deselectAll();
  addNodes({{parentForNodes(), {entityNode}}});
  selectNodes({entityNode});
  translateObjects(delta);

  return entityNode;
}

Model::EntityNode* MapDocument::createBrushEntity(const Assets::BrushEntityDefinition* definition) {
  ensure(definition != nullptr, "definition is null");

  const auto brushes = selectedNodes().brushes();
  assert(!brushes.empty());

  auto entity = Model::Entity{};

  // if all brushes belong to the same entity, and that entity is not worldspawn, copy its
  // properties
  auto* entityTemplate = brushes.front()->entity();
  if (entityTemplate != m_world.get()) {
    for (auto* brush : brushes) {
      if (brush->entity() != entityTemplate) {
        entityTemplate = nullptr;
        break;
      }
    }

    if (entityTemplate != nullptr) {
      entity = entityTemplate->entity();
    }
  }

  entity.addOrUpdateProperty(
    m_world->entityPropertyConfig(), Model::EntityPropertyKeys::Classname, definition->name());
  auto* entityNode = new Model::EntityNode(std::move(entity));

  std::stringstream name;
  name << "Create " << definition->name();

  const std::vector<Model::Node*> nodes(std::begin(brushes), std::end(brushes));

  const Transaction transaction(this, name.str());
  deselectAll();
  addNodes({{parentForNodes(), {entityNode}}});
  reparentNodes({{entityNode, nodes}});
  selectNodes(nodes);

  return entityNode;
}

static std::vector<Model::Node*> collectGroupableNodes(
  const std::vector<Model::Node*>& selectedNodes, const Model::EntityNodeBase* world) {
  std::vector<Model::Node*> result;
  const auto addNode = [&](auto&& thisLambda, auto* node) {
    if (node->entity() == world) {
      result.push_back(node);
    } else {
      node->visitParent(thisLambda);
    }
  };

  Model::Node::visitAll(
    selectedNodes, kdl::overload(
                     [](Model::WorldNode*) {}, [](Model::LayerNode*) {},
                     [&](Model::GroupNode* group) {
                       result.push_back(group);
                     },
                     [&](Model::EntityNode* entity) {
                       result.push_back(entity);
                     },
                     [&](auto&& thisLambda, Model::BrushNode* brush) {
                       addNode(thisLambda, brush);
                     },
                     [&](auto&& thisLambda, Model::PatchNode* patch) {
                       addNode(thisLambda, patch);
                     }));
  return kdl::vec_sort_and_remove_duplicates(std::move(result));
}

Model::GroupNode* MapDocument::groupSelection(const std::string& name) {
  if (!hasSelectedNodes())
    return nullptr;

  const std::vector<Model::Node*> nodes = collectGroupableNodes(selectedNodes().nodes(), world());
  if (nodes.empty())
    return nullptr;

  Model::GroupNode* group = new Model::GroupNode(Model::Group(name));

  const Transaction transaction(this, "Group Selected Objects");
  deselectAll();
  addNodes({{parentForNodes(nodes), {group}}});
  reparentNodes({{group, nodes}});
  selectNodes({group});

  return group;
}

void MapDocument::mergeSelectedGroupsWithGroup(Model::GroupNode* group) {
  if (!hasSelectedNodes() || !m_selectedNodes.hasOnlyGroups())
    return;

  const Transaction transaction(this, "Merge Groups");
  const std::vector<Model::GroupNode*> groupsToMerge = m_selectedNodes.groups();

  deselectAll();
  for (auto groupToMerge : groupsToMerge) {
    if (groupToMerge == group)
      continue;

    const std::vector<Model::Node*> children = groupToMerge->children();
    reparentNodes({{group, children}});
  }
  selectNodes({group});
}

void MapDocument::ungroupSelection() {
  if (!hasSelectedNodes())
    return;

  const Transaction transaction(this, "Ungroup");
  separateSelectedLinkedGroups(false);

  const std::vector<Model::Node*> selectedNodes = m_selectedNodes.nodes();
  std::vector<Model::Node*> nodesToReselect;

  deselectAll();

  Model::Node::visitAll(
    selectedNodes, kdl::overload(
                     [](Model::WorldNode*) {}, [](Model::LayerNode*) {},
                     [&](Model::GroupNode* group) {
                       Model::Node* parent = group->parent();
                       const std::vector<Model::Node*> children = group->children();
                       reparentNodes({{parent, children}});
                       nodesToReselect = kdl::vec_concat(std::move(nodesToReselect), children);
                     },
                     [&](Model::EntityNode* entity) {
                       nodesToReselect.push_back(entity);
                     },
                     [&](Model::BrushNode* brush) {
                       nodesToReselect.push_back(brush);
                     },
                     [&](Model::PatchNode* patch) {
                       nodesToReselect.push_back(patch);
                     }));

  selectNodes(nodesToReselect);
}

void MapDocument::renameGroups(const std::string& name) {
  if (!hasSelectedNodes() || !m_selectedNodes.hasOnlyGroups())
    return;

  const auto commandName =
    kdl::str_plural("Rename ", m_selectedNodes.groupCount(), "Group", "Groups");
  applyAndSwap(
    *this, commandName, m_selectedNodes.groups(), {},
    kdl::overload(
      [](Model::Layer&) {
        return true;
      },
      [&](Model::Group& group) {
        group.setName(name);
        return true;
      },
      [](Model::Entity&) {
        return true;
      },
      [](Model::Brush&) {
        return true;
      },
      [](Model::BezierPatch&) {
        return true;
      }));
}

void MapDocument::openGroup(Model::GroupNode* group) {
  const Transaction transaction(this, "Open Group");

  deselectAll();
  Model::GroupNode* previousGroup = m_editorContext->currentGroup();
  if (previousGroup == nullptr)
    lock(std::vector<Model::Node*>{m_world.get()});
  else
    resetLock(std::vector<Model::Node*>{previousGroup});
  unlock(std::vector<Model::Node*>{group});
  executeAndStore(CurrentGroupCommand::push(group));
}

void MapDocument::closeGroup() {
  const Transaction transaction(this, "Close Group");

  deselectAll();
  Model::GroupNode* previousGroup = m_editorContext->currentGroup();
  resetLock(std::vector<Model::Node*>{previousGroup});
  executeAndStore(CurrentGroupCommand::pop());

  Model::GroupNode* currentGroup = m_editorContext->currentGroup();
  if (currentGroup != nullptr) {
    unlock(std::vector<Model::Node*>{currentGroup});
  } else {
    unlock(std::vector<Model::Node*>{m_world.get()});
  }
}

Model::GroupNode* MapDocument::createLinkedDuplicate() {
  if (!canCreateLinkedDuplicate()) {
    return nullptr;
  }

  Transaction transaction(this, "Create Linked Duplicate");

  auto* groupNode = m_selectedNodes.groups().front();
  if (!groupNode->group().linkedGroupId()) {
    applyAndSwap(
      *this, "Set Linked Group ID", m_selectedNodes.groups(),
      findContainingLinkedGroupsToUpdate(*m_world, m_selectedNodes.groups()),
      kdl::overload(
        [](Model::Layer&) {
          return true;
        },
        [&](Model::Group& group) {
          group.setLinkedGroupId(generateUuid());
          return true;
        },
        [](Model::Entity&) {
          return true;
        },
        [](Model::Brush&) {
          return true;
        },
        [](Model::BezierPatch&) {
          return true;
        }));
  }

  auto* groupNodeClone = static_cast<Model::GroupNode*>(groupNode->cloneRecursively(m_worldBounds));
  auto* suggestedParent = parentForNodes(std::vector<Model::Node*>{groupNode});
  addNodes({{suggestedParent, {groupNodeClone}}});

  return groupNodeClone;
}

bool MapDocument::canCreateLinkedDuplicate() const {
  return m_selectedNodes.hasOnlyGroups() && m_selectedNodes.groupCount() == 1u;
}

void MapDocument::selectLinkedGroups() {
  if (!canSelectLinkedGroups()) {
    return;
  }

  const auto linkedGroupIdsToSelect = kdl::vec_sort_and_remove_duplicates(kdl::vec_transform(
    kdl::vec_filter(
      m_selectedNodes.groups(),
      [](const auto* g) {
        return g->group().linkedGroupId().has_value();
      }),
    [](const auto* g) {
      return *g->group().linkedGroupId();
    }));

  auto groupNodesToSelect = std::vector<Model::Node*>{};
  for (const auto& linkedGroupId : linkedGroupIdsToSelect) {
    groupNodesToSelect = kdl::vec_concat(
      std::move(groupNodesToSelect), Model::findLinkedGroups(*m_world.get(), linkedGroupId));
  }

  groupNodesToSelect = kdl::vec_sort_and_remove_duplicates(std::move(groupNodesToSelect));

  Transaction transaction{this, "Select Linked Groups"};
  deselectAll();
  selectNodes(groupNodesToSelect);
}

bool MapDocument::canSelectLinkedGroups() const {
  if (!m_selectedNodes.hasOnlyGroups()) {
    return false;
  }

  for (const auto* groupNode : m_selectedNodes.groups()) {
    if (!groupNode->group().linkedGroupId()) {
      return false;
    }
  }

  return true;
}

void MapDocument::linkGroups(const std::vector<Model::GroupNode*>& groupNodes) {
  const auto newLinkedGroupId = generateUuid();
  applyAndSwap(
    *this, "Set Linked Group ID", groupNodes,
    findContainingLinkedGroupsToUpdate(*m_world, groupNodes),
    kdl::overload(
      [](Model::Layer&) {
        return true;
      },
      [&](Model::Group& group) {
        group.setLinkedGroupId(newLinkedGroupId);
        return true;
      },
      [](Model::Entity&) {
        return true;
      },
      [](Model::Brush&) {
        return true;
      },
      [](Model::BezierPatch&) {
        return true;
      }));
}

void MapDocument::unlinkGroups(const std::vector<Model::GroupNode*>& groupNodes) {
  applyAndSwap(
    *this, "Reset Linked Group ID", groupNodes,
    findContainingLinkedGroupsToUpdate(*m_world, groupNodes),
    kdl::overload(
      [](Model::Layer&) {
        return true;
      },
      [&](Model::Group& group) {
        group.resetLinkedGroupId();
        return true;
      },
      [](Model::Entity&) {
        return true;
      },
      [](Model::Brush&) {
        return true;
      },
      [](Model::BezierPatch&) {
        return true;
      }));
}

void MapDocument::separateLinkedGroups() {
  Transaction transaction(this, "Separate Linked Groups");
  separateSelectedLinkedGroups(true);
}

bool MapDocument::canSeparateLinkedGroups() const {
  const auto& selectedGroupNodes = m_selectedNodes.groups();
  return std::any_of(
    std::begin(selectedGroupNodes), std::end(selectedGroupNodes), [&](const auto* groupNode) {
      if (const auto linkedGroupId = groupNode->group().linkedGroupId()) {
        const auto linkedGroups = Model::findLinkedGroups(*m_world, *linkedGroupId);
        return linkedGroups.size() > 1u &&
               std::any_of(
                 std::begin(linkedGroups), std::end(linkedGroups), [](const auto* linkedGroupNode) {
                   return !linkedGroupNode->selected();
                 });
      }
      return false;
    });
}

void MapDocument::separateSelectedLinkedGroups(const bool relinkGroups) {
  const auto selectedGroupsWithLinkGroupIds =
    kdl::vec_filter(m_selectedNodes.groups(), [](const auto& g) {
      return g->group().linkedGroupId() != std::nullopt;
    });
  auto selectedLinkedGroupIds =
    kdl::vec_transform(selectedGroupsWithLinkGroupIds, [](const auto& g) {
      return *g->group().linkedGroupId();
    });
  selectedLinkedGroupIds = kdl::vec_sort_and_remove_duplicates(std::move(selectedLinkedGroupIds));

  auto groupsToUnlink = std::vector<Model::GroupNode*>{};
  auto groupsToRelink = std::vector<std::vector<Model::GroupNode*>>{};

  for (const auto& linkedGroupId : selectedLinkedGroupIds) {
    auto linkedGroups = Model::findLinkedGroups(*m_world, linkedGroupId);

    // partition the linked groups into selected and unselected ones
    const auto it = std::partition(
      std::begin(linkedGroups), std::end(linkedGroups), [](const auto* linkedGroupNode) {
        return linkedGroupNode->selected();
      });

    auto selectedLinkedGroups = std::vector<Model::GroupNode*>(std::begin(linkedGroups), it);
    auto unselectedLinkedGroups = std::vector<Model::GroupNode*>(it, std::end(linkedGroups));

    assert(!selectedLinkedGroups.empty());
    if (!unselectedLinkedGroups.empty()) {
      if (selectedLinkedGroups.size() == 1u) {
        // unset the linked group ID later
        groupsToUnlink.push_back(selectedLinkedGroups.front());
      } else if (relinkGroups) {
        groupsToRelink.push_back(std::move(selectedLinkedGroups));
      } else {
        groupsToUnlink =
          kdl::vec_concat(std::move(groupsToUnlink), std::move(selectedLinkedGroups));
      }

      if (unselectedLinkedGroups.size() == 1u) {
        // unset the linked group ID later
        groupsToUnlink.push_back(unselectedLinkedGroups.front());
      }
    } else if (!relinkGroups) {
      // all members of a link group are being separated, and we don't want to relink them, so we
      // need to reset their linked group IDs
      groupsToUnlink = kdl::vec_concat(std::move(groupsToUnlink), std::move(selectedLinkedGroups));
    }
  }

  unlinkGroups(groupsToUnlink);
  for (const auto& groupNodes : groupsToRelink) {
    linkGroups(groupNodes);
  }
}

void MapDocument::renameLayer(Model::LayerNode* layerNode, const std::string& name) {
  applyAndSwap(
    *this, "Rename Layer", std::vector<Model::Node*>{layerNode}, {},
    kdl::overload(
      [&](Model::Layer& layer) {
        layer.setName(name);
        return true;
      },
      [](Model::Group&) {
        return true;
      },
      [](Model::Entity&) {
        return true;
      },
      [](Model::Brush&) {
        return true;
      },
      [](Model::BezierPatch&) {
        return true;
      }));
}

bool MapDocument::moveLayerByOne(Model::LayerNode* layerNode, MoveDirection direction) {
  const std::vector<Model::LayerNode*> sorted = m_world->customLayersUserSorted();

  const auto maybeIndex = kdl::vec_index_of(sorted, layerNode);
  if (!maybeIndex.has_value()) {
    return false;
  }

  const int newIndex = static_cast<int>(*maybeIndex) + (direction == MoveDirection::Down ? 1 : -1);
  if (newIndex < 0 || newIndex >= static_cast<int>(sorted.size())) {
    return false;
  }

  Model::LayerNode* neighbourNode = sorted.at(static_cast<size_t>(newIndex));
  auto layer = layerNode->layer();
  auto neighbourLayer = neighbourNode->layer();

  const int layerSortIndex = layer.sortIndex();
  const int neighbourSortIndex = neighbourLayer.sortIndex();

  // Swap the sort indices of `layer` and `neighbour`
  layer.setSortIndex(neighbourSortIndex);
  neighbourLayer.setSortIndex(layerSortIndex);

  swapNodeContents(
    "Swap Layer Positions",
    {{layerNode, Model::NodeContents(std::move(layer))},
     {neighbourNode, Model::NodeContents(std::move(neighbourLayer))}},
    {});

  return true;
}

void MapDocument::moveLayer(Model::LayerNode* layer, const int offset) {
  ensure(layer != m_world->defaultLayer(), "attempted to move default layer");

  const Transaction transaction(this, "Move Layer");

  const MoveDirection direction = (offset > 0) ? MoveDirection::Down : MoveDirection::Up;
  for (int i = 0; i < std::abs(offset); ++i) {
    if (!moveLayerByOne(layer, direction)) {
      break;
    }
  }
}

bool MapDocument::canMoveLayer(Model::LayerNode* layer, const int offset) const {
  ensure(layer != nullptr, "null layer");

  Model::WorldNode* world = this->world();
  if (layer == world->defaultLayer()) {
    return false;
  }

  const std::vector<Model::LayerNode*> sorted = world->customLayersUserSorted();
  const auto maybeIndex = kdl::vec_index_of(sorted, layer);
  if (!maybeIndex.has_value()) {
    return false;
  }

  const int newIndex = static_cast<int>(*maybeIndex) + offset;
  return (newIndex >= 0 && newIndex < static_cast<int>(sorted.size()));
}

void MapDocument::moveSelectionToLayer(Model::LayerNode* layer) {
  Transaction transaction(this, "Move Nodes to " + layer->name());

  const auto& selectedNodes = this->selectedNodes().nodes();

  auto nodesToMove = std::vector<Model::Node*>{};
  auto nodesToSelect = std::vector<Model::Node*>{};

  const auto addBrushOrPatchNode = [&](auto* node) {
    assert(node->selected());

    if (!node->containedInGroup()) {
      auto* entity = node->entity();
      if (entity == m_world.get()) {
        nodesToMove.push_back(node);
        nodesToSelect.push_back(node);
      } else {
        if (!kdl::vec_contains(nodesToMove, entity)) {
          nodesToMove.push_back(entity);
          nodesToSelect = kdl::vec_concat(std::move(nodesToSelect), entity->children());
        }
      }
    }
  };

  for (auto* node : selectedNodes) {
    node->accept(kdl::overload(
      [](Model::WorldNode*) {}, [](Model::LayerNode*) {},
      [&](Model::GroupNode* group) {
        assert(group->selected());

        if (!group->containedInGroup()) {
          nodesToMove.push_back(group);
          nodesToSelect.push_back(group);
        }
      },
      [&](Model::EntityNode* entity) {
        assert(entity->selected());

        if (!entity->containedInGroup()) {
          nodesToMove.push_back(entity);
          nodesToSelect.push_back(entity);
        }
      },
      [&](Model::BrushNode* brush) {
        addBrushOrPatchNode(brush);
      },
      [&](Model::PatchNode* patch) {
        addBrushOrPatchNode(patch);
      }));
  }

  if (!nodesToMove.empty()) {
    deselectAll();
    reparentNodes({{layer, nodesToMove}});
    if (!layer->hidden() && !layer->locked()) {
      selectNodes(nodesToSelect);
    }
  }
}

bool MapDocument::canMoveSelectionToLayer(Model::LayerNode* layer) const {
  ensure(layer != nullptr, "null layer");
  const auto& nodes = selectedNodes().nodes();

  const bool isAnyNodeInGroup = std::any_of(std::begin(nodes), std::end(nodes), [&](auto* node) {
    return Model::findContainingGroup(node) != nullptr;
  });
  const bool isAnyNodeInOtherLayer =
    std::any_of(std::begin(nodes), std::end(nodes), [&](auto* node) {
      return Model::findContainingLayer(node) != layer;
    });

  return !nodes.empty() && !isAnyNodeInGroup && isAnyNodeInOtherLayer;
}

void MapDocument::hideLayers(const std::vector<Model::LayerNode*>& layers) {
  Transaction transaction(this, "Hide Layers");
  hide(std::vector<Model::Node*>(std::begin(layers), std::end(layers)));
}

bool MapDocument::canHideLayers(const std::vector<Model::LayerNode*>& layers) const {
  for (auto* layer : layers) {
    if (layer->visible()) {
      return true;
    }
  }
  return false;
}

void MapDocument::isolateLayers(const std::vector<Model::LayerNode*>& layers) {
  const auto allLayers = world()->allLayers();

  Transaction transaction(this, "Isolate Layers");
  hide(std::vector<Model::Node*>(std::begin(allLayers), std::end(allLayers)));
  show(std::vector<Model::Node*>(std::begin(layers), std::end(layers)));
}

bool MapDocument::canIsolateLayers(const std::vector<Model::LayerNode*>& layers) const {
  if (layers.empty()) {
    return false;
  }
  for (auto* layer : m_world->allLayers()) {
    const bool shouldShowLayer = kdl::vec_contains(layers, layer);

    if (shouldShowLayer != layer->visible()) {
      return true;
    }
  }
  // The layers are already isolated
  return false;
}

void MapDocument::isolate() {
  auto selectedNodes = std::vector<Model::Node*>{};
  auto unselectedNodes = std::vector<Model::Node*>{};

  const auto collectNode = [&](auto* node) {
    if (node->transitivelySelected() || node->descendantSelected()) {
      selectedNodes.push_back(node);
    } else {
      unselectedNodes.push_back(node);
    }
  };

  m_world->accept(kdl::overload(
    [](auto&& thisLambda, Model::WorldNode* world) {
      world->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, Model::LayerNode* layer) {
      layer->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, Model::GroupNode* group) {
      collectNode(group);
      group->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, Model::EntityNode* entity) {
      collectNode(entity);
      entity->visitChildren(thisLambda);
    },
    [&](Model::BrushNode* brush) {
      collectNode(brush);
    },
    [&](Model::PatchNode* patch) {
      collectNode(patch);
    }));

  Transaction transaction(this, "Isolate Objects");
  executeAndStore(SetVisibilityCommand::hide(unselectedNodes));
  executeAndStore(SetVisibilityCommand::show(selectedNodes));
}

void MapDocument::setOmitLayerFromExport(Model::LayerNode* layerNode, const bool omitFromExport) {
  const auto commandName = omitFromExport ? "Omit Layer from Export" : "Include Layer in Export";

  auto layer = layerNode->layer();
  layer.setOmitFromExport(omitFromExport);
  swapNodeContents(commandName, {{layerNode, Model::NodeContents(std::move(layer))}}, {});
}

void MapDocument::selectAllInLayers(const std::vector<Model::LayerNode*>& layers) {
  const auto nodes =
    Model::collectSelectableNodes(kdl::vec_element_cast<Model::Node*>(layers), editorContext());

  deselectAll();
  selectNodes(nodes);
}

bool MapDocument::canSelectAllInLayers(const std::vector<Model::LayerNode*>& /* layers */) const {
  return editorContext().canChangeSelection();
}

void MapDocument::hide(const std::vector<Model::Node*> nodes) {
  const Transaction transaction(this, "Hide Objects");

  // Deselect any selected nodes inside `nodes`
  deselectNodes(Model::collectSelectedNodes(nodes));

  // Reset visibility of any forced shown children of `nodes`
  downgradeShownToInherit(Model::collectDescendants(nodes));

  executeAndStore(SetVisibilityCommand::hide(nodes));
}

void MapDocument::hideSelection() {
  hide(m_selectedNodes.nodes());
}

void MapDocument::show(const std::vector<Model::Node*>& nodes) {
  executeAndStore(SetVisibilityCommand::show(nodes));
}

void MapDocument::showAll() {
  resetVisibility(
    Model::collectDescendants(kdl::vec_element_cast<Model::Node*>(m_world->allLayers())));
}

void MapDocument::ensureVisible(const std::vector<Model::Node*>& nodes) {
  executeAndStore(SetVisibilityCommand::ensureVisible(nodes));
}

void MapDocument::resetVisibility(const std::vector<Model::Node*>& nodes) {
  executeAndStore(SetVisibilityCommand::reset(nodes));
}

void MapDocument::lock(const std::vector<Model::Node*>& nodes) {
  const Transaction transaction(this, "Lock Objects");

  // Deselect any selected nodes or faces inside `nodes`
  deselectNodes(Model::collectSelectedNodes(nodes));
  deselectBrushFaces(Model::collectSelectedBrushFaces(nodes));

  // Reset lock state of any forced unlocked children of `nodes`
  downgradeUnlockedToInherit(Model::collectDescendants(nodes));

  executeAndStore(SetLockStateCommand::lock(nodes));
}

void MapDocument::unlock(const std::vector<Model::Node*>& nodes) {
  executeAndStore(SetLockStateCommand::unlock(nodes));
}

/**
 * Unlocks only those nodes from the given list whose lock state resolves to "locked"
 */
void MapDocument::ensureUnlocked(const std::vector<Model::Node*>& nodes) {
  std::vector<Model::Node*> nodesToUnlock;
  for (auto* node : nodes) {
    if (node->locked()) {
      nodesToUnlock.push_back(node);
    }
  }
  unlock(nodesToUnlock);
}

void MapDocument::resetLock(const std::vector<Model::Node*>& nodes) {
  executeAndStore(SetLockStateCommand::reset(nodes));
}

/**
 * This is called to clear the forced Visibility_Shown that was set on newly created nodes
 * so they could be visible if created in a hidden layer
 */
void MapDocument::downgradeShownToInherit(const std::vector<Model::Node*>& nodes) {
  std::vector<Model::Node*> nodesToReset;
  for (auto* node : nodes) {
    if (node->visibilityState() == Model::VisibilityState::Shown) {
      nodesToReset.push_back(node);
    }
  }
  resetVisibility(nodesToReset);
}

/**
 * See downgradeShownToInherit
 */
void MapDocument::downgradeUnlockedToInherit(const std::vector<Model::Node*>& nodes) {
  std::vector<Model::Node*> nodesToReset;
  for (auto* node : nodes) {
    if (node->lockState() == Model::LockState::Unlocked) {
      nodesToReset.push_back(node);
    }
  }
  resetLock(nodesToReset);
}

bool MapDocument::swapNodeContents(
  const std::string& commandName,
  std::vector<std::pair<Model::Node*, Model::NodeContents>> nodesToSwap,
  std::vector<std::pair<const Model::GroupNode*, std::vector<Model::GroupNode*>>>
    linkedGroupsToUpdate) {
  return executeAndStore(std::make_unique<SwapNodeContentsCommand>(
                           commandName, std::move(nodesToSwap), std::move(linkedGroupsToUpdate)))
    ->success();
}

bool MapDocument::swapNodeContents(
  const std::string& commandName,
  std::vector<std::pair<Model::Node*, Model::NodeContents>> nodesToSwap) {
  auto linkedGroupsToUpdate =
    findContainingLinkedGroupsToUpdate(*m_world, kdl::vec_transform(nodesToSwap, [](const auto& p) {
      return p.first;
    }));

  return swapNodeContents(commandName, std::move(nodesToSwap), std::move(linkedGroupsToUpdate));
}

bool MapDocument::transformObjects(
  const std::string& commandName, const vm::mat4x4& transformation) {
  auto nodesToTransform = std::vector<Model::Node*>{};

  const auto addEntity = [&](auto* node) {
    if (auto* entity = node->entity()) {
      if (entity->childSelectionCount() == entity->childCount()) {
        nodesToTransform.push_back(entity);
      }
    }
  };

  for (auto* node : m_selectedNodes) {
    node->accept(kdl::overload(
      [&](auto&& thisLambda, Model::WorldNode* world) {
        world->visitChildren(thisLambda);
      },
      [&](auto&& thisLambda, Model::LayerNode* layer) {
        layer->visitChildren(thisLambda);
      },
      [&](auto&& thisLambda, Model::GroupNode* group) {
        nodesToTransform.push_back(group);
        group->visitChildren(thisLambda);
      },
      [&](auto&& thisLambda, Model::EntityNode* entity) {
        if (!entity->hasChildren()) {
          nodesToTransform.push_back(entity);
        } else {
          entity->visitChildren(thisLambda);
        }
      },
      [&](Model::BrushNode* brush) {
        nodesToTransform.push_back(brush);
        addEntity(brush);
      },
      [&](Model::PatchNode* patch) {
        nodesToTransform.push_back(patch);
        addEntity(patch);
      }));
  }

  // brush entites can be added many times
  nodesToTransform = kdl::vec_sort_and_remove_duplicates(std::move(nodesToTransform));

  using TransformResult =
    kdl::result<std::pair<Model::Node*, Model::NodeContents>, Model::BrushError>;

  const bool lockTexturesPref = pref(Preferences::TextureLock);
  auto transformResults =
    kdl::vec_parallel_transform(nodesToTransform, [&](Model::Node* node) -> TransformResult {
      return node->accept(kdl::overload(
        [&](Model::WorldNode*) -> TransformResult {
          ensure(false, "Unexpected world node");
        },
        [&](Model::LayerNode*) -> TransformResult {
          ensure(false, "Unexpected layer node");
        },
        [&](Model::GroupNode* groupNode) -> TransformResult {
          auto group = groupNode->group();
          group.transform(transformation);
          return std::make_pair(groupNode, Model::NodeContents{std::move(group)});
        },
        [&](Model::EntityNode* entityNode) -> TransformResult {
          auto entity = entityNode->entity();
          entity.transform(m_world->entityPropertyConfig(), transformation);
          return std::make_pair(entityNode, Model::NodeContents{std::move(entity)});
        },
        [&](Model::BrushNode* brushNode) -> TransformResult {
          const bool lockTextures =
            lockTexturesPref || (Model::findContainingLinkedGroup(*brushNode) != nullptr);

          auto brush = brushNode->brush();
          return brush.transform(m_worldBounds, transformation, lockTextures)
            .and_then([&]() -> TransformResult {
              return std::make_pair(brushNode, Model::NodeContents{std::move(brush)});
            });
        },
        [&](Model::PatchNode* patchNode) -> TransformResult {
          auto patch = patchNode->patch();
          patch.transform(transformation);
          return std::make_pair(patchNode, Model::NodeContents{std::move(patch)});
        }));
    });

  bool transformFailed = false;
  auto nodesToUpdate =
    kdl::collect_values(std::move(transformResults), kdl::overload([&](const Model::BrushError& e) {
                          error() << "Could not transform brush: " << e;
                          transformFailed = true;
                        }));

  if (transformFailed) {
    return false;
  }

  const auto success = swapNodeContents(
    commandName, std::move(nodesToUpdate),
    findContainingLinkedGroupsToUpdate(*m_world, m_selectedNodes.nodes()));

  if (success) {
    m_repeatStack->push([=]() {
      this->transformObjects(commandName, transformation);
    });
    return true;
  }
  return false;
}

bool MapDocument::translateObjects(const vm::vec3& delta) {
  return transformObjects("Translate Objects", vm::translation_matrix(delta));
}

bool MapDocument::rotateObjects(
  const vm::vec3& center, const vm::vec3& axis, const FloatType angle) {
  const auto transformation = vm::translation_matrix(center) * vm::rotation_matrix(axis, angle) *
                              vm::translation_matrix(-center);
  return transformObjects("Rotate Objects", transformation);
}

bool MapDocument::scaleObjects(const vm::bbox3& oldBBox, const vm::bbox3& newBBox) {
  const auto transformation = vm::scale_bbox_matrix(oldBBox, newBBox);
  return transformObjects("Scale Objects", transformation);
}

bool MapDocument::scaleObjects(const vm::vec3& center, const vm::vec3& scaleFactors) {
  const auto transformation = vm::translation_matrix(center) * vm::scaling_matrix(scaleFactors) *
                              vm::translation_matrix(-center);
  return transformObjects("Scale Objects", transformation);
}

bool MapDocument::shearObjects(
  const vm::bbox3& box, const vm::vec3& sideToShear, const vm::vec3& delta) {
  const auto transformation = vm::shear_bbox_matrix(box, sideToShear, delta);
  return transformObjects("Scale Objects", transformation);
}

bool MapDocument::flipObjects(const vm::vec3& center, const vm::axis::type axis) {
  const auto transformation = vm::translation_matrix(center) * vm::mirror_matrix<FloatType>(axis) *
                              vm::translation_matrix(-center);
  return transformObjects("Flip Objects", transformation);
}

bool MapDocument::createBrush(const std::vector<vm::vec3>& points) {
  Model::BrushBuilder builder(m_world->mapFormat(), m_worldBounds, m_game->defaultFaceAttribs());
  return builder.createBrush(points, currentTextureName())
    .and_then([&](Model::Brush&& b) {
      Model::BrushNode* brushNode = new Model::BrushNode(std::move(b));

      Transaction transaction(this, "Create Brush");
      deselectAll();
      addNodes({{parentForNodes(), {brushNode}}});
      selectNodes({brushNode});
    })
    .handle_errors([&](const Model::BrushError e) {
      error() << "Could not create brush: " << e;
    });
}

bool MapDocument::csgConvexMerge() {
  if (!hasSelectedBrushFaces() && !selectedNodes().hasOnlyBrushes()) {
    return false;
  }

  std::vector<vm::vec3> points;

  if (hasSelectedBrushFaces()) {
    for (const auto& handle : selectedBrushFaces()) {
      for (const Model::BrushVertex* vertex : handle.face().vertices()) {
        points.push_back(vertex->position());
      }
    }
  } else if (selectedNodes().hasOnlyBrushes()) {
    for (const Model::BrushNode* brushNode : selectedNodes().brushes()) {
      const Model::Brush& brush = brushNode->brush();
      for (const Model::BrushVertex* vertex : brush.vertices()) {
        points.push_back(vertex->position());
      }
    }
  }

  Model::Polyhedron3 polyhedron(std::move(points));
  if (!polyhedron.polyhedron() || !polyhedron.closed()) {
    return false;
  }

  const Model::BrushBuilder builder(
    m_world->mapFormat(), m_worldBounds, m_game->defaultFaceAttribs());
  return builder.createBrush(polyhedron, currentTextureName())
    .and_then([&](Model::Brush&& b) {
      b.cloneFaceAttributesFrom(
        kdl::vec_transform(selectedNodes().brushes(), [](const auto* brushNode) {
          return &brushNode->brush();
        }));

      // The nodelist is either empty or contains only brushes.
      const auto toRemove = selectedNodes().nodes();

      // We could be merging brushes that have different parents; use the parent of the first brush.
      Model::Node* parentNode = nullptr;
      if (!selectedNodes().brushes().empty()) {
        parentNode = selectedNodes().brushes().front()->parent();
      } else if (!selectedBrushFaces().empty()) {
        parentNode = selectedBrushFaces().front().node()->parent();
      } else {
        parentNode = parentForNodes();
      }

      Model::BrushNode* brushNode = new Model::BrushNode(std::move(b));

      const Transaction transaction(this, "CSG Convex Merge");
      deselectAll();
      addNodes({{parentNode, {brushNode}}});
      removeNodes(toRemove);
      selectNodes({brushNode});
    })
    .handle_errors([&](const Model::BrushError e) {
      error() << "Could not create brush: " << e;
    });
}

bool MapDocument::csgSubtract() {
  const auto subtrahendNodes = std::vector<Model::BrushNode*>{selectedNodes().brushes()};
  if (subtrahendNodes.empty()) {
    return false;
  }

  Transaction transaction(this, "CSG Subtract");
  // Select touching, but don't delete the subtrahends yet
  selectTouching(false);

  const auto minuendNodes = std::vector<Model::BrushNode*>{selectedNodes().brushes()};
  const auto subtrahends = kdl::vec_transform(subtrahendNodes, [](const auto* subtrahendNode) {
    return &subtrahendNode->brush();
  });

  auto toAdd = std::map<Model::Node*, std::vector<Model::Node*>>{};
  auto toRemove = std::vector<Model::Node*>{std::begin(subtrahendNodes), std::end(subtrahendNodes)};

  for (auto* minuendNode : minuendNodes) {
    const Model::Brush& minuend = minuendNode->brush();
    auto currentSubtractionResults =
      minuend.subtract(m_world->mapFormat(), m_worldBounds, currentTextureName(), subtrahends);
    auto currentBrushes =
      kdl::collect_values(std::move(currentSubtractionResults), [&](const Model::BrushError& e) {
        error() << "Could not create brush: " << e;
      });

    if (!currentBrushes.empty()) {
      auto resultNodes = kdl::vec_transform(std::move(currentBrushes), [&](auto b) {
        return new Model::BrushNode(std::move(b));
      });
      auto& toAddForParent = toAdd[minuendNode->parent()];
      toAddForParent = kdl::vec_concat(std::move(toAddForParent), std::move(resultNodes));
    }

    toRemove.push_back(minuendNode);
  }

  deselectAll();
  const auto added = addNodes(toAdd);
  removeNodes(toRemove);
  selectNodes(added);
  return true;
}

bool MapDocument::csgIntersect() {
  const std::vector<Model::BrushNode*> brushes = selectedNodes().brushes();
  if (brushes.size() < 2u) {
    return false;
  }

  Model::Brush intersection = brushes.front()->brush();

  bool valid = true;
  for (auto it = std::next(std::begin(brushes)), end = std::end(brushes); it != end && valid;
       ++it) {
    Model::BrushNode* brushNode = *it;
    const Model::Brush& brush = brushNode->brush();
    valid =
      intersection.intersect(m_worldBounds, brush).handle_errors([&](const Model::BrushError e) {
        error() << "Could not intersect brushes: " << e;
      });
  }

  const std::vector<Model::Node*> toRemove(std::begin(brushes), std::end(brushes));

  Transaction transaction(this, "CSG Intersect");
  deselectNodes(toRemove);

  if (valid) {
    Model::BrushNode* intersectionNode = new Model::BrushNode(std::move(intersection));
    addNodes({{parentForNodes(toRemove), {intersectionNode}}});
    removeNodes(toRemove);
    selectNodes({intersectionNode});
  } else {
    removeNodes(toRemove);
  }

  return true;
}

bool MapDocument::csgHollow() {
  const std::vector<Model::BrushNode*> brushNodes = selectedNodes().brushes();
  if (brushNodes.empty()) {
    return false;
  }

  bool didHollowAnything = false;
  std::vector<std::pair<Model::BrushNode*, std::vector<Model::Brush>>> fragmentsAndSourceNodes =
    kdl::vec_transform(brushNodes, [&](Model::BrushNode* brushNode) {
      const auto& originalBrush = brushNode->brush();

      auto shrunkenBrush = originalBrush;
      std::vector<Model::Brush> fragments;
      shrunkenBrush.expand(m_worldBounds, -1.0 * static_cast<FloatType>(m_grid->actualSize()), true)
        .and_then([&]() {
          didHollowAnything = true;

          auto subtractionResults = originalBrush.subtract(
            m_world->mapFormat(), m_worldBounds, currentTextureName(), shrunkenBrush);
          fragments =
            kdl::collect_values(std::move(subtractionResults), [&](const Model::BrushError& e) {
              error() << "Could not create brush: " << e;
            });
        })
        .handle_errors([&](const Model::BrushError& e) {
          error() << "Could not hollow brush: " << e;
          fragments = {originalBrush};
        });

      return std::make_pair(brushNode, std::move(fragments));
    });

  if (!didHollowAnything) {
    return false;
  }

  auto toAdd = std::map<Model::Node*, std::vector<Model::Node*>>{};
  auto toRemove = std::vector<Model::Node*>{};

  for (auto& [sourceNode, fragments] : fragmentsAndSourceNodes) {
    auto fragmentNodes = kdl::vec_transform(std::move(fragments), [](auto&& b) {
      return new Model::BrushNode(std::move(b));
    });

    auto& toAddForParent = toAdd[sourceNode->parent()];
    toAddForParent = kdl::vec_concat(std::move(toAddForParent), fragmentNodes);
    toRemove.push_back(sourceNode);
  }

  Transaction transaction(this, "CSG Hollow");
  deselectAll();
  const auto added = addNodes(toAdd);
  removeNodes(toRemove);
  selectNodes(added);

  return true;
}

bool MapDocument::clipBrushes(const vm::vec3& p1, const vm::vec3& p2, const vm::vec3& p3) {
  return kdl::for_each_result(
           m_selectedNodes.brushes(),
           [&](const Model::BrushNode* originalBrush) {
             auto clippedBrush = originalBrush->brush();
             return Model::BrushFace::create(
                      p1, p2, p3, Model::BrushFaceAttributes(currentTextureName()),
                      m_world->mapFormat())
               .and_then([&](Model::BrushFace&& clipFace) {
                 return clippedBrush.clip(m_worldBounds, std::move(clipFace));
               })
               .and_then([&]() -> kdl::result<std::pair<Model::Node*, Model::Brush>> {
                 return std::make_pair(originalBrush->parent(), std::move(clippedBrush));
               });
           })
    .and_then([&](auto&& clippedBrushAndParents) {
      auto toAdd = std::map<Model::Node*, std::vector<Model::Node*>>{};
      const auto toRemove = kdl::vec_element_cast<Model::Node*>(m_selectedNodes.brushes());

      for (auto& [parentNode, clippedBrush] : clippedBrushAndParents) {
        toAdd[parentNode].push_back(new Model::BrushNode(std::move(clippedBrush)));
      }

      Transaction transaction(this, "Clip Brushes");
      deselectAll();
      removeNodes(toRemove);

      const auto addedNodes = addNodes(toAdd);
      selectNodes(addedNodes);
    })
    .handle_errors([&](const Model::BrushError e) {
      error() << "Could not clip brushes: " << e;
    });
}

bool MapDocument::setProperty(
  const std::string& key, const std::string& value, const bool defaultToProtected) {
  const auto entityNodes = allSelectedEntityNodes();
  return applyAndSwap(
    *this, "Set Property", entityNodes, findContainingLinkedGroupsToUpdate(*m_world, entityNodes),
    kdl::overload(
      [](Model::Layer&) {
        return true;
      },
      [](Model::Group&) {
        return true;
      },
      [&](Model::Entity& entity) {
        entity.addOrUpdateProperty(m_world->entityPropertyConfig(), key, value, defaultToProtected);
        return true;
      },
      [](Model::Brush&) {
        return true;
      },
      [](Model::BezierPatch&) {
        return true;
      }));
}

bool MapDocument::renameProperty(const std::string& oldKey, const std::string& newKey) {
  const auto entityNodes = allSelectedEntityNodes();
  return applyAndSwap(
    *this, "Rename Property", entityNodes,
    findContainingLinkedGroupsToUpdate(*m_world, entityNodes),
    kdl::overload(
      [](Model::Layer&) {
        return true;
      },
      [](Model::Group&) {
        return true;
      },
      [&](Model::Entity& entity) {
        entity.renameProperty(m_world->entityPropertyConfig(), oldKey, newKey);
        return true;
      },
      [](Model::Brush&) {
        return true;
      },
      [](Model::BezierPatch&) {
        return true;
      }));
}

bool MapDocument::removeProperty(const std::string& key) {
  const auto entityNodes = allSelectedEntityNodes();
  return applyAndSwap(
    *this, "Remove Property", entityNodes,
    findContainingLinkedGroupsToUpdate(*m_world, entityNodes),
    kdl::overload(
      [](Model::Layer&) {
        return true;
      },
      [](Model::Group&) {
        return true;
      },
      [&](Model::Entity& entity) {
        entity.removeProperty(m_world->entityPropertyConfig(), key);
        return true;
      },
      [](Model::Brush&) {
        return true;
      },
      [](Model::BezierPatch&) {
        return true;
      }));
}

bool MapDocument::convertEntityColorRange(const std::string& key, Assets::ColorRange::Type range) {
  const auto entityNodes = allSelectedEntityNodes();
  return applyAndSwap(
    *this, "Convert Color", entityNodes, findContainingLinkedGroupsToUpdate(*m_world, entityNodes),
    kdl::overload(
      [](Model::Layer&) {
        return true;
      },
      [](Model::Group&) {
        return true;
      },
      [&](Model::Entity& entity) {
        if (const auto* oldValue = entity.property(key)) {
          entity.addOrUpdateProperty(
            m_world->entityPropertyConfig(), key, Model::convertEntityColor(*oldValue, range));
        }
        return true;
      },
      [](Model::Brush&) {
        return true;
      },
      [](Model::BezierPatch&) {
        return true;
      }));
}

bool MapDocument::updateSpawnflag(
  const std::string& key, const size_t flagIndex, const bool setFlag) {
  const auto entityNodes = allSelectedEntityNodes();
  return applyAndSwap(
    *this, setFlag ? "Set Spawnflag" : "Unset Spawnflag", entityNodes,
    findContainingLinkedGroupsToUpdate(*m_world, entityNodes),
    kdl::overload(
      [](Model::Layer&) {
        return true;
      },
      [](Model::Group&) {
        return true;
      },
      [&](Model::Entity& entity) {
        const auto* strValue = entity.property(key);
        int intValue = strValue ? kdl::str_to_int(*strValue).value_or(0) : 0;
        const int flagValue = (1 << flagIndex);

        intValue = setFlag ? intValue | flagValue : intValue & ~flagValue;
        entity.addOrUpdateProperty(
          m_world->entityPropertyConfig(), key, kdl::str_to_string(intValue));

        return true;
      },
      [](Model::Brush&) {
        return true;
      },
      [](Model::BezierPatch&) {
        return true;
      }));
}

/**
 * Search the given linked groups for an entity node at the given node path, and return its
 * unprotected value for the given property key.
 */
static std::optional<std::string> findUnprotectedPropertyValue(
  const std::string& key, const Model::NodePath& nodePath,
  const std::vector<Model::GroupNode*> linkedGroups) {
  for (const auto* linkedGroup : linkedGroups) {
    if (const auto* node = linkedGroup->resolvePath(nodePath)) {
      if (const auto* entityNode = dynamic_cast<const Model::EntityNode*>(node)) {
        if (!kdl::vec_contains(entityNode->entity().protectedProperties(), key)) {
          if (const auto* value = entityNode->entity().property(key)) {
            return *value;
          }
        }
      }
    }
  }
  return std::nullopt;
}

/**
 * Find the unprotected property value of the given key in the corresponding linked nodes of the
 * given entity nodes. This value is used to restore the original value when a property is set from
 * protected to unprotected.
 */
static std::optional<std::string> findUnprotectedPropertyValue(
  const std::string& key, const Model::EntityNodeBase& entityNode, Model::WorldNode& worldNode) {
  if (const auto* containingLinkedGroup = Model::findContainingLinkedGroup(entityNode)) {
    if (const auto linkedGroupId = containingLinkedGroup->group().linkedGroupId()) {
      const auto linkedGroups = Model::findLinkedGroups(worldNode, *linkedGroupId);
      const auto pathFromContainingLinkedGroup = entityNode.pathFrom(*containingLinkedGroup);
      if (
        const auto newValue =
          findUnprotectedPropertyValue(key, pathFromContainingLinkedGroup, linkedGroups)) {
        return newValue;
      }
    }
  }
  return std::nullopt;
}

bool MapDocument::setProtectedProperty(const std::string& key, const bool value) {
  const auto entityNodes = allSelectedEntityNodes();

  auto nodesToUpdate = std::vector<std::pair<Model::Node*, Model::NodeContents>>{};
  for (auto* entityNode : entityNodes) {
    auto entity = entityNode->entity();
    auto protectedProperties = entity.protectedProperties();
    if (value && !kdl::vec_contains(protectedProperties, key)) {
      protectedProperties.push_back(key);
    } else if (!value && kdl::vec_contains(protectedProperties, key)) {
      if (const auto newValue = findUnprotectedPropertyValue(key, *entityNode, *m_world.get())) {
        entity.addOrUpdateProperty(m_world->entityPropertyConfig(), key, *newValue);
      }

      protectedProperties = kdl::vec_erase(std::move(protectedProperties), key);
    }
    entity.setProtectedProperties(std::move(protectedProperties));
    nodesToUpdate.emplace_back(entityNode, std::move(entity));
  }

  return swapNodeContents(
    "Set Protected Property", nodesToUpdate,
    findContainingLinkedGroupsToUpdate(*m_world, entityNodes));
}

bool MapDocument::clearProtectedProperties() {
  const auto entityNodes = allSelectedEntityNodes();

  auto nodesToUpdate = std::vector<std::pair<Model::Node*, Model::NodeContents>>{};
  for (auto* entityNode : entityNodes) {
    if (entityNode->entity().protectedProperties().empty()) {
      continue;
    }

    const auto* containingLinkedGroup = Model::findContainingLinkedGroup(*entityNode);
    if (containingLinkedGroup == nullptr) {
      continue;
    }

    const auto& linkedGroupId = containingLinkedGroup->group().linkedGroupId();
    if (!linkedGroupId.has_value()) {
      continue;
    }

    const auto linkedGroups = Model::findLinkedGroups(*m_world.get(), *linkedGroupId);
    const auto pathFromContainingLinkedGroup = entityNode->pathFrom(*containingLinkedGroup);

    auto entity = entityNode->entity();
    for (const auto& key : entity.protectedProperties()) {
      if (
        const auto newValue =
          findUnprotectedPropertyValue(key, pathFromContainingLinkedGroup, linkedGroups)) {
        entity.addOrUpdateProperty(m_world->entityPropertyConfig(), key, *newValue);
      }
    }

    entity.setProtectedProperties({});
    nodesToUpdate.emplace_back(entityNode, std::move(entity));
  }

  return swapNodeContents(
    "Clear Protected Properties", nodesToUpdate,
    findContainingLinkedGroupsToUpdate(*m_world, entityNodes));
}

bool MapDocument::canClearProtectedProperties() const {
  const auto entityNodes = allSelectedEntityNodes();
  if (entityNodes.empty() || (entityNodes.size() == 1u && entityNodes.front() == m_world.get())) {
    return false;
  }

  const auto linkedGroupsToUpdate = findContainingLinkedGroupsToUpdate(*m_world, entityNodes);
  return checkLinkedGroupsToUpdate(kdl::vec_transform(linkedGroupsToUpdate, [](const auto& p) {
    return p.first;
  }));
}

bool MapDocument::extrudeBrushes(const std::vector<vm::polygon3>& faces, const vm::vec3& delta) {
  const auto nodes = m_selectedNodes.nodes();
  return applyAndSwap(
    *this, "Resize Brushes", nodes, findContainingLinkedGroupsToUpdate(*m_world, nodes),
    kdl::overload(
      [](Model::Layer&) {
        return true;
      },
      [](Model::Group&) {
        return true;
      },
      [](Model::Entity&) {
        return true;
      },
      [&](Model::Brush& brush) {
        const auto faceIndex = brush.findFace(faces);
        if (!faceIndex) {
          // we allow resizing only some of the brushes
          return true;
        }

        return brush.moveBoundary(m_worldBounds, *faceIndex, delta, pref(Preferences::TextureLock))
          .visit(kdl::overload(
            [&]() {
              return m_worldBounds.contains(brush.bounds());
            },
            [&](const Model::BrushError e) {
              error() << "Could not resize brush: " << e;
              return false;
            }));
      },
      [](Model::BezierPatch&) {
        return true;
      }));
}

bool MapDocument::setFaceAttributes(const Model::BrushFaceAttributes& attributes) {
  Model::ChangeBrushFaceAttributesRequest request;
  request.setAll(attributes);
  return setFaceAttributes(request);
}

bool MapDocument::setFaceAttributesExceptContentFlags(
  const Model::BrushFaceAttributes& attributes) {
  Model::ChangeBrushFaceAttributesRequest request;
  request.setAllExceptContentFlags(attributes);
  return setFaceAttributes(request);
}

bool MapDocument::setFaceAttributes(const Model::ChangeBrushFaceAttributesRequest& request) {
  return applyAndSwap(
    *this, request.name(), allSelectedBrushFaces(), [&](Model::BrushFace& brushFace) {
      request.evaluate(brushFace);
      return true;
    });
}

bool MapDocument::copyTexCoordSystemFromFace(
  const Model::TexCoordSystemSnapshot& coordSystemSnapshot,
  const Model::BrushFaceAttributes& attribs, const vm::plane3& sourceFacePlane,
  const Model::WrapStyle wrapStyle) {
  return applyAndSwap(
    *this, "Copy Texture Alignment", m_selectedBrushFaces, [&](Model::BrushFace& face) {
      face.copyTexCoordSystemFromFace(coordSystemSnapshot, attribs, sourceFacePlane, wrapStyle);
      return true;
    });
}

bool MapDocument::moveTextures(
  const vm::vec3f& cameraUp, const vm::vec3f& cameraRight, const vm::vec2f& delta) {
  return applyAndSwap(*this, "Move Textures", m_selectedBrushFaces, [&](Model::BrushFace& face) {
    face.moveTexture(vm::vec3(cameraUp), vm::vec3(cameraRight), delta);
    return true;
  });
}

bool MapDocument::rotateTextures(const float angle) {
  return applyAndSwap(*this, "Rotate Textures", m_selectedBrushFaces, [&](Model::BrushFace& face) {
    face.rotateTexture(angle);
    return true;
  });
}

bool MapDocument::shearTextures(const vm::vec2f& factors) {
  return applyAndSwap(*this, "Shear Textures", m_selectedBrushFaces, [&](Model::BrushFace& face) {
    face.shearTexture(factors);
    return true;
  });
}

bool MapDocument::flipTextures(
  const vm::vec3f& cameraUp, const vm::vec3f& cameraRight,
  const vm::direction cameraRelativeFlipDirection) {
  const bool isHFlip =
    (cameraRelativeFlipDirection == vm::direction::left ||
     cameraRelativeFlipDirection == vm::direction::right);
  return applyAndSwap(
    *this, isHFlip ? "Flip Textures Horizontally" : "Flip Textures Vertically",
    m_selectedBrushFaces, [&](Model::BrushFace& face) {
      face.flipTexture(vm::vec3(cameraUp), vm::vec3(cameraRight), cameraRelativeFlipDirection);
      return true;
    });
}

bool MapDocument::snapVertices(const FloatType snapTo) {
  size_t succeededBrushCount = 0;
  size_t failedBrushCount = 0;

  const auto allSelectedBrushes = allSelectedBrushNodes();
  const bool applyAndSwapSuccess = applyAndSwap(
    *this, "Snap Brush Vertices", allSelectedBrushes,
    findContainingLinkedGroupsToUpdate(*m_world, allSelectedBrushes),
    kdl::overload(
      [](Model::Layer&) {
        return true;
      },
      [](Model::Group&) {
        return true;
      },
      [](Model::Entity&) {
        return true;
      },
      [&](Model::Brush& originalBrush) {
        if (originalBrush.canSnapVertices(m_worldBounds, snapTo)) {
          originalBrush.snapVertices(m_worldBounds, snapTo, pref(Preferences::UVLock))
            .and_then([&]() {
              succeededBrushCount += 1;
            })
            .handle_errors([&](const Model::BrushError e) {
              error() << "Could not snap vertices: " << e;
              failedBrushCount += 1;
            });
        } else {
          failedBrushCount += 1;
        }
        return true;
      },
      [](Model::BezierPatch&) {
        return true;
      }));

  if (!applyAndSwapSuccess) {
    return false;
  }
  if (succeededBrushCount > 0) {
    info(kdl::str_to_string(
      "Snapped vertices of ", succeededBrushCount, " ",
      kdl::str_plural(succeededBrushCount, "brush", "brushes")));
  }
  if (failedBrushCount > 0) {
    info(kdl::str_to_string(
      "Failed to snap vertices of ", failedBrushCount, " ",
      kdl::str_plural(failedBrushCount, "brush", "brushes")));
  }

  return true;
}

MapDocument::MoveVerticesResult MapDocument::moveVertices(
  std::vector<vm::vec3> vertexPositions, const vm::vec3& delta) {
  auto newVertexPositions = std::vector<vm::vec3>{};
  auto newNodes = applyToNodeContents(
    m_selectedNodes.nodes(),
    kdl::overload(
      [](Model::Layer&) {
        return true;
      },
      [](Model::Group&) {
        return true;
      },
      [](Model::Entity&) {
        return true;
      },
      [&](Model::Brush& brush) {
        const auto verticesToMove = kdl::vec_filter(vertexPositions, [&](const auto& vertex) {
          return brush.hasVertex(vertex);
        });
        if (verticesToMove.empty()) {
          return true;
        }

        if (!brush.canMoveVertices(m_worldBounds, verticesToMove, delta)) {
          return false;
        }

        return brush.moveVertices(m_worldBounds, verticesToMove, delta, pref(Preferences::UVLock))
          .and_then([&]() {
            auto newPositions = brush.findClosestVertexPositions(verticesToMove + delta);
            newVertexPositions =
              kdl::vec_concat(std::move(newVertexPositions), std::move(newPositions));
          })
          .handle_errors([&](const Model::BrushError e) {
            error() << "Could not move brush vertices: " << e;
          });
      },
      [](Model::BezierPatch&) {
        return true;
      }));

  if (newNodes) {
    kdl::vec_sort_and_remove_duplicates(newVertexPositions);

    const auto commandName =
      kdl::str_plural(vertexPositions.size(), "Move Brush Vertex", "Move Brush Vertices");
    auto linkedGroupsToUpdate =
      findContainingLinkedGroupsToUpdate(*m_world, kdl::vec_transform(*newNodes, [](const auto& p) {
        return p.first;
      }));
    const auto result = executeAndStore(std::make_unique<BrushVertexCommand>(
      commandName, std::move(*newNodes), std::move(vertexPositions), std::move(newVertexPositions),
      std::move(linkedGroupsToUpdate)));

    const auto* moveVerticesResult = dynamic_cast<BrushVertexCommandResult*>(result.get());
    ensure(
      moveVerticesResult != nullptr, "command processor returned unexpected command result type");

    return MoveVerticesResult(
      moveVerticesResult->success(), moveVerticesResult->hasRemainingVertices());
  }

  return MoveVerticesResult(false, false);
}

bool MapDocument::moveEdges(std::vector<vm::segment3> edgePositions, const vm::vec3& delta) {
  auto newEdgePositions = std::vector<vm::segment3>{};
  auto newNodes = applyToNodeContents(
    m_selectedNodes.nodes(),
    kdl::overload(
      [](Model::Layer&) {
        return true;
      },
      [](Model::Group&) {
        return true;
      },
      [](Model::Entity&) {
        return true;
      },
      [&](Model::Brush& brush) {
        const auto edgesToMove = kdl::vec_filter(edgePositions, [&](const auto& edge) {
          return brush.hasEdge(edge);
        });
        if (edgesToMove.empty()) {
          return true;
        }

        if (!brush.canMoveEdges(m_worldBounds, edgesToMove, delta)) {
          return false;
        }

        return brush.moveEdges(m_worldBounds, edgesToMove, delta, pref(Preferences::UVLock))
          .and_then([&]() {
            auto newPositions =
              brush.findClosestEdgePositions(kdl::vec_transform(edgesToMove, [&](const auto& edge) {
                return edge.translate(delta);
              }));
            newEdgePositions =
              kdl::vec_concat(std::move(newEdgePositions), std::move(newPositions));
          })
          .handle_errors([&](const Model::BrushError e) {
            error() << "Could not move brush edges: " << e;
          });
      },
      [](Model::BezierPatch&) {
        return true;
      }));

  if (newNodes) {
    kdl::vec_sort_and_remove_duplicates(newEdgePositions);

    const auto commandName =
      kdl::str_plural(edgePositions.size(), "Move Brush Edge", "Move Brush Edges");
    auto linkedGroupsToUpdate =
      findContainingLinkedGroupsToUpdate(*m_world, kdl::vec_transform(*newNodes, [](const auto& p) {
        return p.first;
      }));
    return executeAndStore(std::make_unique<BrushEdgeCommand>(
                             commandName, std::move(*newNodes), std::move(edgePositions),
                             std::move(newEdgePositions), std::move(linkedGroupsToUpdate)))
      ->success();
  }

  return false;
}

bool MapDocument::moveFaces(std::vector<vm::polygon3> facePositions, const vm::vec3& delta) {
  auto newFacePositions = std::vector<vm::polygon3>{};
  auto newNodes = applyToNodeContents(
    m_selectedNodes.nodes(),
    kdl::overload(
      [](Model::Layer&) {
        return true;
      },
      [](Model::Group&) {
        return true;
      },
      [](Model::Entity&) {
        return true;
      },
      [&](Model::Brush& brush) {
        const auto facesToMove = kdl::vec_filter(facePositions, [&](const auto& face) {
          return brush.hasFace(face);
        });
        if (facesToMove.empty()) {
          return true;
        }

        if (!brush.canMoveFaces(m_worldBounds, facesToMove, delta)) {
          return false;
        }

        return brush.moveFaces(m_worldBounds, facesToMove, delta, pref(Preferences::UVLock))
          .and_then([&]() {
            auto newPositions =
              brush.findClosestFacePositions(kdl::vec_transform(facesToMove, [&](const auto& face) {
                return face.translate(delta);
              }));
            newFacePositions =
              kdl::vec_concat(std::move(newFacePositions), std::move(newPositions));
          })
          .handle_errors([&](const Model::BrushError e) {
            error() << "Could not move brush faces: " << e;
          });
      },
      [](Model::BezierPatch&) {
        return true;
      }));

  if (newNodes) {
    kdl::vec_sort_and_remove_duplicates(newFacePositions);

    const auto commandName =
      kdl::str_plural(facePositions.size(), "Move Brush Face", "Move Brush Faces");
    auto linkedGroupsToUpdate =
      findContainingLinkedGroupsToUpdate(*m_world, kdl::vec_transform(*newNodes, [](const auto& p) {
        return p.first;
      }));
    return executeAndStore(std::make_unique<BrushFaceCommand>(
                             commandName, std::move(*newNodes), std::move(facePositions),
                             std::move(newFacePositions), std::move(linkedGroupsToUpdate)))
      ->success();
  }

  return false;
}

bool MapDocument::addVertex(const vm::vec3& vertexPosition) {
  auto newNodes = applyToNodeContents(
    m_selectedNodes.nodes(), kdl::overload(
                               [](Model::Layer&) {
                                 return true;
                               },
                               [](Model::Group&) {
                                 return true;
                               },
                               [](Model::Entity&) {
                                 return true;
                               },
                               [&](Model::Brush& brush) {
                                 if (!brush.canAddVertex(m_worldBounds, vertexPosition)) {
                                   return false;
                                 }

                                 return brush.addVertex(m_worldBounds, vertexPosition)
                                   .handle_errors([&](const Model::BrushError e) {
                                     error() << "Could not add brush vertex: " << e;
                                   });
                               },
                               [](Model::BezierPatch&) {
                                 return true;
                               }));

  if (newNodes) {
    auto linkedGroupsToUpdate =
      findContainingLinkedGroupsToUpdate(*m_world, kdl::vec_transform(*newNodes, [](const auto& p) {
        return p.first;
      }));
    return executeAndStore(std::make_unique<BrushVertexCommand>(
                             "Add Brush Vertex", std::move(*newNodes), std::vector<vm::vec3>{},
                             std::vector<vm::vec3>{vertexPosition},
                             std::move(linkedGroupsToUpdate)))
      ->success();
  }

  return false;
}

bool MapDocument::removeVertices(
  const std::string& commandName, std::vector<vm::vec3> vertexPositions) {
  auto newNodes = applyToNodeContents(
    m_selectedNodes.nodes(), kdl::overload(
                               [](Model::Layer&) {
                                 return true;
                               },
                               [](Model::Group&) {
                                 return true;
                               },
                               [](Model::Entity&) {
                                 return true;
                               },
                               [&](Model::Brush& brush) {
                                 const auto verticesToRemove =
                                   kdl::vec_filter(vertexPositions, [&](const auto& vertex) {
                                     return brush.hasVertex(vertex);
                                   });
                                 if (verticesToRemove.empty()) {
                                   return true;
                                 }

                                 if (!brush.canRemoveVertices(m_worldBounds, verticesToRemove)) {
                                   return false;
                                 }

                                 return brush.removeVertices(m_worldBounds, verticesToRemove)
                                   .handle_errors([&](const Model::BrushError e) {
                                     error() << "Could not remove brush vertices: " << e;
                                   });
                               },
                               [](Model::BezierPatch&) {
                                 return true;
                               }));

  if (newNodes) {
    auto linkedGroupsToUpdate =
      findContainingLinkedGroupsToUpdate(*m_world, kdl::vec_transform(*newNodes, [](const auto& p) {
        return p.first;
      }));
    return executeAndStore(std::make_unique<BrushVertexCommand>(
                             commandName, std::move(*newNodes), std::move(vertexPositions),
                             std::vector<vm::vec3>{}, std::move(linkedGroupsToUpdate)))
      ->success();
  }

  return false;
}

void MapDocument::printVertices() {
  if (hasSelectedBrushFaces()) {
    for (const auto& handle : m_selectedBrushFaces) {
      std::stringstream str;
      str.precision(17);
      for (const Model::BrushVertex* vertex : handle.face().vertices()) {
        str << "(" << vertex->position() << ") ";
      }
      info(str.str());
    }
  } else if (selectedNodes().hasBrushes()) {
    for (const Model::BrushNode* brushNode : selectedNodes().brushes()) {
      const Model::Brush& brush = brushNode->brush();

      std::stringstream str;
      str.precision(17);
      for (const Model::BrushVertex* vertex : brush.vertices()) {
        str << vertex->position() << " ";
      }
      info(str.str());
    }
  }
}

class ThrowExceptionCommand : public UndoableCommand {
public:
  static const CommandType Type;
  using Ptr = std::shared_ptr<ThrowExceptionCommand>;

public:
  ThrowExceptionCommand()
    : UndoableCommand(Type, "Throw Exception", false) {}

private:
  std::unique_ptr<CommandResult> doPerformDo(MapDocumentCommandFacade*) override {
    throw CommandProcessorException();
  }

  std::unique_ptr<CommandResult> doPerformUndo(MapDocumentCommandFacade*) override {
    return std::make_unique<CommandResult>(true);
  }

  bool doCollateWith(UndoableCommand*) override { return false; }
};

const ThrowExceptionCommand::CommandType ThrowExceptionCommand::Type = Command::freeType();

bool MapDocument::throwExceptionDuringCommand() {
  const auto result = executeAndStore(std::make_unique<ThrowExceptionCommand>());
  return result->success();
}

bool MapDocument::canUndoCommand() const {
  return doCanUndoCommand();
}

bool MapDocument::canRedoCommand() const {
  return doCanRedoCommand();
}

const std::string& MapDocument::undoCommandName() const {
  return doGetUndoCommandName();
}

const std::string& MapDocument::redoCommandName() const {
  return doGetRedoCommandName();
}

void MapDocument::undoCommand() {
  doUndoCommand();
  // Undo/redo in the repeat system is not supported for now, so just clear the repeat stack
  m_repeatStack->clear();
}

void MapDocument::redoCommand() {
  doRedoCommand();
  m_repeatStack->clear();
}

bool MapDocument::canRepeatCommands() const {
  return m_repeatStack->size() > 0u;
}

void MapDocument::repeatCommands() {
  m_repeatStack->repeat();
}

void MapDocument::clearRepeatableCommands() {
  m_repeatStack->clear();
}

void MapDocument::startTransaction(const std::string& name) {
  debug("Starting transaction '" + name + "'");
  doStartTransaction(name);
  m_repeatStack->startTransaction();
}

void MapDocument::rollbackTransaction() {
  debug("Rolling back transaction");
  doRollbackTransaction();
  m_repeatStack->rollbackTransaction();
}

void MapDocument::commitTransaction() {
  debug("Committing transaction");
  doCommitTransaction();
  m_repeatStack->commitTransaction();
}

void MapDocument::cancelTransaction() {
  debug("Cancelling transaction");
  doRollbackTransaction();
  m_repeatStack->rollbackTransaction();
  doCommitTransaction();
  m_repeatStack->commitTransaction();
}

std::unique_ptr<CommandResult> MapDocument::execute(std::unique_ptr<Command>&& command) {
  return doExecute(std::move(command));
}

std::unique_ptr<CommandResult> MapDocument::executeAndStore(
  std::unique_ptr<UndoableCommand>&& command) {
  return doExecuteAndStore(std::move(command));
}

void MapDocument::commitPendingAssets() {
  m_textureManager->commitChanges();
}

void MapDocument::pick(const vm::ray3& pickRay, Model::PickResult& pickResult) const {
  if (m_world != nullptr)
    m_world->pick(*m_editorContext, pickRay, pickResult);
}

std::vector<Model::Node*> MapDocument::findNodesContaining(const vm::vec3& point) const {
  std::vector<Model::Node*> result;
  if (m_world != nullptr) {
    m_world->findNodesContaining(point, result);
  }
  return result;
}

void MapDocument::createWorld(
  const Model::MapFormat mapFormat, const vm::bbox3& worldBounds,
  std::shared_ptr<Model::Game> game) {
  m_worldBounds = worldBounds;
  m_game = game;
  m_world = m_game->newMap(mapFormat, m_worldBounds, logger());
  performSetCurrentLayer(m_world->defaultLayer());

  updateGameSearchPaths();
  setPath(IO::Path(DefaultDocumentName));
}

void MapDocument::loadWorld(
  const Model::MapFormat mapFormat, const vm::bbox3& worldBounds, std::shared_ptr<Model::Game> game,
  const IO::Path& path) {
  m_worldBounds = worldBounds;
  m_game = game;
  m_world = m_game->loadMap(mapFormat, m_worldBounds, path, logger());
  performSetCurrentLayer(m_world->defaultLayer());

  updateGameSearchPaths();
  setPath(path);
}

void MapDocument::clearWorld() {
  m_world.reset();
  m_currentLayer = nullptr;
}

Assets::EntityDefinitionFileSpec MapDocument::entityDefinitionFile() const {
  if (m_world != nullptr) {
    return m_game->extractEntityDefinitionFile(m_world->entity());
  } else {
    return Assets::EntityDefinitionFileSpec();
  }
}

std::vector<Assets::EntityDefinitionFileSpec> MapDocument::allEntityDefinitionFiles() const {
  return m_game->allEntityDefinitionFiles();
}

void MapDocument::setEntityDefinitionFile(const Assets::EntityDefinitionFileSpec& spec) {
  // to avoid backslashes being misinterpreted as escape sequences
  const std::string formatted = kdl::str_replace_every(spec.asString(), "\\", "/");

  auto entity = m_world->entity();
  entity.addOrUpdateProperty(
    m_world->entityPropertyConfig(), Model::EntityPropertyKeys::EntityDefinitions, formatted);
  swapNodeContents(
    "Set Entity Definitions", {{world(), Model::NodeContents(std::move(entity))}}, {});
}

void MapDocument::setEntityDefinitions(const std::vector<Assets::EntityDefinition*>& definitions) {
  m_entityDefinitionManager->setDefinitions(definitions);
}

std::vector<IO::Path> MapDocument::enabledTextureCollections() const {
  return m_game->extractTextureCollections(m_world->entity());
}

std::vector<IO::Path> MapDocument::availableTextureCollections() const {
  return m_game->findTextureCollections();
}

void MapDocument::setEnabledTextureCollections(const std::vector<IO::Path>& paths) {
  auto entity = m_world->entity();
  m_game->updateTextureCollections(entity, paths);
  swapNodeContents(
    "Set Texture Collections", {{world(), Model::NodeContents(std::move(entity))}}, {});
}

void MapDocument::reloadTextureCollections() {
  const auto nodes = std::vector<Model::Node*>{m_world.get()};
  NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
  NotifyBeforeAndAfter notifyTextureCollections(
    textureCollectionsWillChangeNotifier, textureCollectionsDidChangeNotifier);

  info("Reloading texture collections");
  reloadTextures();
  setTextures();
  initializeAllNodeTags(this);
}

void MapDocument::reloadEntityDefinitions() {
  const auto nodes = std::vector<Model::Node*>{m_world.get()};
  NotifyBeforeAndAfter notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
  NotifyBeforeAndAfter notifyEntityDefinitions(
    entityDefinitionsWillChangeNotifier, entityDefinitionsDidChangeNotifier);

  info("Reloading entity definitions");
}

void MapDocument::loadAssets() {
  loadEntityDefinitions();
  setEntityDefinitions();
  loadEntityModels();
  loadTextures();
  setTextures();
}

void MapDocument::unloadAssets() {
  unloadEntityDefinitions();
  unloadEntityModels();
  unloadTextures();
}

void MapDocument::loadEntityDefinitions() {
  const Assets::EntityDefinitionFileSpec spec = entityDefinitionFile();
  try {
    const IO::Path path = m_game->findEntityDefinitionFile(spec, externalSearchPaths());
    IO::SimpleParserStatus status(logger());
    m_entityDefinitionManager->loadDefinitions(path, *m_game, status);
    info("Loaded entity definition file " + path.lastComponent().asString());

    createEntityDefinitionActions();
  } catch (const Exception& e) {
    if (spec.builtin()) {
      error() << "Could not load builtin entity definition file '" << spec.path()
              << "': " << e.what();
    } else {
      error() << "Could not load external entity definition file '" << spec.path()
              << "': " << e.what();
    }
  }
}

void MapDocument::unloadEntityDefinitions() {
  unsetEntityDefinitions();
  m_entityDefinitionManager->clear();
  m_entityDefinitionActions.clear();
}

void MapDocument::loadEntityModels() {
  m_entityModelManager->setLoader(m_game.get());
  setEntityModels();
}

void MapDocument::unloadEntityModels() {
  clearEntityModels();
  m_entityModelManager->setLoader(nullptr);
}

void MapDocument::reloadTextures() {
  unloadTextures();
  m_game->reloadShaders();
  loadTextures();
}

void MapDocument::loadTextures() {
  try {
    const IO::Path docDir = m_path.isEmpty() ? IO::Path() : m_path.deleteLastComponent();
    m_game->loadTextureCollections(m_world->entity(), docDir, *m_textureManager, logger());
  } catch (const Exception& e) { error(e.what()); }
}

void MapDocument::unloadTextures() {
  unsetTextures();
  m_textureManager->clear();
}

static auto makeSetTexturesVisitor(Assets::TextureManager& manager) {
  return kdl::overload(
    [](auto&& thisLambda, Model::WorldNode* world) {
      world->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, Model::LayerNode* layer) {
      layer->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, Model::GroupNode* group) {
      group->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, Model::EntityNode* entity) {
      entity->visitChildren(thisLambda);
    },
    [&](Model::BrushNode* brushNode) {
      const Model::Brush& brush = brushNode->brush();
      for (size_t i = 0u; i < brush.faceCount(); ++i) {
        const Model::BrushFace& face = brush.face(i);
        Assets::Texture* texture = manager.texture(face.attributes().textureName());
        brushNode->setFaceTexture(i, texture);
      }
    },
    [&](Model::PatchNode* patchNode) {
      auto* texture = manager.texture(patchNode->patch().textureName());
      patchNode->setTexture(texture);
    });
}

static auto makeUnsetTexturesVisitor() {
  return kdl::overload(
    [](auto&& thisLambda, Model::WorldNode* world) {
      world->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, Model::LayerNode* layer) {
      layer->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, Model::GroupNode* group) {
      group->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, Model::EntityNode* entity) {
      entity->visitChildren(thisLambda);
    },
    [](Model::BrushNode* brushNode) {
      const Model::Brush& brush = brushNode->brush();
      for (size_t i = 0u; i < brush.faceCount(); ++i) {
        brushNode->setFaceTexture(i, nullptr);
      }
    },
    [](Model::PatchNode* patchNode) {
      patchNode->setTexture(nullptr);
    });
}

void MapDocument::setTextures() {
  m_world->accept(makeSetTexturesVisitor(*m_textureManager));
  textureUsageCountsDidChangeNotifier();
}

void MapDocument::setTextures(const std::vector<Model::Node*>& nodes) {
  Model::Node::visitAll(nodes, makeSetTexturesVisitor(*m_textureManager));
  textureUsageCountsDidChangeNotifier();
}

void MapDocument::setTextures(const std::vector<Model::BrushFaceHandle>& faceHandles) {
  for (const auto& faceHandle : faceHandles) {
    Model::BrushNode* node = faceHandle.node();
    const Model::BrushFace& face = faceHandle.face();
    Assets::Texture* texture = m_textureManager->texture(face.attributes().textureName());
    node->setFaceTexture(faceHandle.faceIndex(), texture);
  }
  textureUsageCountsDidChangeNotifier();
}

void MapDocument::unsetTextures() {
  m_world->accept(makeUnsetTexturesVisitor());
  textureUsageCountsDidChangeNotifier();
}

void MapDocument::unsetTextures(const std::vector<Model::Node*>& nodes) {
  Model::Node::visitAll(nodes, makeUnsetTexturesVisitor());
  textureUsageCountsDidChangeNotifier();
}

static auto makeSetEntityDefinitionsVisitor(Assets::EntityDefinitionManager& manager) {
  // this helper lambda must be captured by value
  const auto setEntityDefinition = [&](auto* node) {
    auto* definition = manager.definition(node);
    node->setDefinition(definition);
  };

  return kdl::overload(
    [=](auto&& thisLambda, Model::WorldNode* world) {
      setEntityDefinition(world);
      world->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, Model::LayerNode* layer) {
      layer->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, Model::GroupNode* group) {
      group->visitChildren(thisLambda);
    },
    [=](Model::EntityNode* entity) {
      setEntityDefinition(entity);
    },
    [](Model::BrushNode*) {}, [](Model::PatchNode*) {});
}

static auto makeUnsetEntityDefinitionsVisitor() {
  return kdl::overload(
    [](auto&& thisLambda, Model::WorldNode* world) {
      world->setDefinition(nullptr);
      world->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, Model::LayerNode* layer) {
      layer->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, Model::GroupNode* group) {
      group->visitChildren(thisLambda);
    },
    [](Model::EntityNode* entity) {
      entity->setDefinition(nullptr);
    },
    [](Model::BrushNode*) {}, [](Model::PatchNode*) {});
}

void MapDocument::setEntityDefinitions() {
  m_world->accept(makeSetEntityDefinitionsVisitor(*m_entityDefinitionManager));
}

void MapDocument::setEntityDefinitions(const std::vector<Model::Node*>& nodes) {
  Model::Node::visitAll(nodes, makeSetEntityDefinitionsVisitor(*m_entityDefinitionManager));
}

void MapDocument::unsetEntityDefinitions() {
  m_world->accept(makeUnsetEntityDefinitionsVisitor());
}

void MapDocument::unsetEntityDefinitions(const std::vector<Model::Node*>& nodes) {
  Model::Node::visitAll(nodes, makeUnsetEntityDefinitionsVisitor());
}

void MapDocument::reloadEntityDefinitionsInternal() {
  unloadEntityDefinitions();
  clearEntityModels();
  loadEntityDefinitions();
  setEntityDefinitions();
  setEntityModels();
}

void MapDocument::clearEntityModels() {
  unsetEntityModels();
  m_entityModelManager->clear();
}

static auto makeSetEntityModelsVisitor(Logger& logger, Assets::EntityModelManager& manager) {
  return kdl::overload(
    [](auto&& thisLambda, Model::WorldNode* world) {
      world->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, Model::LayerNode* layer) {
      layer->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, Model::GroupNode* group) {
      group->visitChildren(thisLambda);
    },
    [&](Model::EntityNode* entityNode) {
      const auto modelSpec =
        Assets::safeGetModelSpecification(logger, entityNode->entity().classname(), [&]() {
          return entityNode->entity().modelSpecification();
        });
      const auto* frame = manager.frame(modelSpec);
      entityNode->setModelFrame(frame);
    },
    [](Model::BrushNode*) {}, [](Model::PatchNode*) {});
}

static auto makeUnsetEntityModelsVisitor() {
  return kdl::overload(
    [](auto&& thisLambda, Model::WorldNode* world) {
      world->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, Model::LayerNode* layer) {
      layer->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, Model::GroupNode* group) {
      group->visitChildren(thisLambda);
    },
    [](Model::EntityNode* entity) {
      entity->setModelFrame(nullptr);
    },
    [](Model::BrushNode*) {}, [](Model::PatchNode*) {});
}

void MapDocument::setEntityModels() {
  m_world->accept(makeSetEntityModelsVisitor(*this, *m_entityModelManager));
}

void MapDocument::setEntityModels(const std::vector<Model::Node*>& nodes) {
  Model::Node::visitAll(nodes, makeSetEntityModelsVisitor(*this, *m_entityModelManager));
}

void MapDocument::unsetEntityModels() {
  m_world->accept(makeUnsetEntityModelsVisitor());
}

void MapDocument::unsetEntityModels(const std::vector<Model::Node*>& nodes) {
  Model::Node::visitAll(nodes, makeUnsetEntityModelsVisitor());
}

std::vector<IO::Path> MapDocument::externalSearchPaths() const {
  std::vector<IO::Path> searchPaths;
  if (!m_path.isEmpty() && m_path.isAbsolute()) {
    searchPaths.push_back(m_path.deleteLastComponent());
  }

  const IO::Path gamePath = m_game->gamePath();
  if (!gamePath.isEmpty()) {
    searchPaths.push_back(gamePath);
  }

  searchPaths.push_back(IO::SystemPaths::appDirectory());
  return searchPaths;
}

void MapDocument::updateGameSearchPaths() {
  const std::vector<IO::Path> additionalSearchPaths = IO::Path::asPaths(mods());
  m_game->setAdditionalSearchPaths(additionalSearchPaths, logger());
}

std::vector<std::string> MapDocument::mods() const {
  return m_game->extractEnabledMods(m_world->entity());
}

void MapDocument::setMods(const std::vector<std::string>& mods) {
  auto entity = m_world->entity();
  if (mods.empty()) {
    entity.removeProperty(m_world->entityPropertyConfig(), Model::EntityPropertyKeys::Mods);
  } else {
    const std::string newValue = kdl::str_join(mods, ";");
    entity.addOrUpdateProperty(
      m_world->entityPropertyConfig(), Model::EntityPropertyKeys::Mods, newValue);
  }
  swapNodeContents("Set Enabled Mods", {{world(), Model::NodeContents(std::move(entity))}}, {});
}

std::string MapDocument::defaultMod() const {
  return m_game->defaultMod();
}

/**
 * Note if bounds.source is SoftMapBoundsType::Game, bounds.bounds is ignored.
 */
void MapDocument::setSoftMapBounds(const Model::Game::SoftMapBounds& bounds) {
  auto entity = world()->entity();
  switch (bounds.source) {
    case Model::Game::SoftMapBoundsType::Map:
      if (!bounds.bounds.has_value()) {
        // Set the worldspawn key EntityPropertyKeys::SoftMaxMapSize's value to the empty string
        // to indicate that we are overriding the Game's bounds with unlimited.
        entity.addOrUpdateProperty(
          m_world->entityPropertyConfig(), Model::EntityPropertyKeys::SoftMapBounds,
          Model::EntityPropertyValues::NoSoftMapBounds);
      } else {
        entity.addOrUpdateProperty(
          m_world->entityPropertyConfig(), Model::EntityPropertyKeys::SoftMapBounds,
          IO::serializeSoftMapBoundsString(*bounds.bounds));
      }
      break;
    case Model::Game::SoftMapBoundsType::Game:
      // Unset the map's setting
      entity.removeProperty(
        m_world->entityPropertyConfig(), Model::EntityPropertyKeys::SoftMapBounds);
      break;
      switchDefault();
  }
  swapNodeContents("Set Soft Map Bounds", {{world(), Model::NodeContents(std::move(entity))}}, {});
}

Model::Game::SoftMapBounds MapDocument::softMapBounds() const {
  if (!m_world) {
    return {Model::Game::SoftMapBoundsType::Game, std::nullopt};
  }
  return m_game->extractSoftMapBounds(m_world->entity());
}

void MapDocument::setIssueHidden(Model::Issue* issue, const bool hidden) {
  doSetIssueHidden(issue, hidden);
}

void MapDocument::registerIssueGenerators() {
  ensure(m_world != nullptr, "world is null");
  ensure(m_game.get() != nullptr, "game is null");

  m_world->registerIssueGenerator(new Model::MissingClassnameIssueGenerator());
  m_world->registerIssueGenerator(new Model::MissingDefinitionIssueGenerator());
  m_world->registerIssueGenerator(new Model::MissingModIssueGenerator(m_game));
  m_world->registerIssueGenerator(new Model::EmptyGroupIssueGenerator());
  m_world->registerIssueGenerator(new Model::EmptyBrushEntityIssueGenerator());
  m_world->registerIssueGenerator(new Model::PointEntityWithBrushesIssueGenerator());
  m_world->registerIssueGenerator(new Model::LinkSourceIssueGenerator());
  m_world->registerIssueGenerator(new Model::LinkTargetIssueGenerator());
  m_world->registerIssueGenerator(new Model::NonIntegerVerticesIssueGenerator());
  m_world->registerIssueGenerator(new Model::MixedBrushContentsIssueGenerator());
  m_world->registerIssueGenerator(new Model::WorldBoundsIssueGenerator(worldBounds()));
  m_world->registerIssueGenerator(new Model::SoftMapBoundsIssueGenerator(m_game, m_world.get()));
  m_world->registerIssueGenerator(new Model::EmptyPropertyKeyIssueGenerator());
  m_world->registerIssueGenerator(new Model::EmptyPropertyValueIssueGenerator());
  m_world->registerIssueGenerator(
    new Model::LongPropertyKeyIssueGenerator(m_game->maxPropertyLength()));
  m_world->registerIssueGenerator(
    new Model::LongPropertyValueIssueGenerator(m_game->maxPropertyLength()));
  m_world->registerIssueGenerator(new Model::PropertyKeyWithDoubleQuotationMarksIssueGenerator());
  m_world->registerIssueGenerator(new Model::PropertyValueWithDoubleQuotationMarksIssueGenerator());
  m_world->registerIssueGenerator(new Model::InvalidTextureScaleIssueGenerator());
}

void MapDocument::registerSmartTags() {
  ensure(m_game.get() != nullptr, "game is null");

  m_tagManager->clearSmartTags();
  m_tagManager->registerSmartTags(m_game->smartTags());
}

const std::vector<Model::SmartTag>& MapDocument::smartTags() const {
  return m_tagManager->smartTags();
}

bool MapDocument::isRegisteredSmartTag(const std::string& name) const {
  return m_tagManager->isRegisteredSmartTag(name);
}

const Model::SmartTag& MapDocument::smartTag(const std::string& name) const {
  return m_tagManager->smartTag(name);
}

bool MapDocument::isRegisteredSmartTag(const size_t index) const {
  return m_tagManager->isRegisteredSmartTag(index);
}

const Model::SmartTag& MapDocument::smartTag(const size_t index) const {
  return m_tagManager->smartTag(index);
}

static auto makeInitializeNodeTagsVisitor(Model::TagManager& tagManager) {
  return kdl::overload(
    [&](auto&& thisLambda, Model::WorldNode* world) {
      world->initializeTags(tagManager);
      world->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, Model::LayerNode* layer) {
      layer->initializeTags(tagManager);
      layer->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, Model::GroupNode* group) {
      group->initializeTags(tagManager);
      group->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, Model::EntityNode* entity) {
      entity->initializeTags(tagManager);
      entity->visitChildren(thisLambda);
    },
    [&](Model::BrushNode* brush) {
      brush->initializeTags(tagManager);
    },
    [&](Model::PatchNode* patch) {
      patch->initializeTags(tagManager);
    });
}

static auto makeClearNodeTagsVisitor() {
  return kdl::overload(
    [](auto&& thisLambda, Model::WorldNode* world) {
      world->clearTags();
      world->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, Model::LayerNode* layer) {
      layer->clearTags();
      layer->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, Model::GroupNode* group) {
      group->clearTags();
      group->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, Model::EntityNode* entity) {
      entity->clearTags();
      entity->visitChildren(thisLambda);
    },
    [](Model::BrushNode* brush) {
      brush->clearTags();
    },
    [](Model::PatchNode* patch) {
      patch->clearTags();
    });
}

void MapDocument::initializeAllNodeTags(MapDocument* document) {
  assert(document == this);
  unused(document);
  m_world->accept(makeInitializeNodeTagsVisitor(*m_tagManager));
}

void MapDocument::initializeNodeTags(const std::vector<Model::Node*>& nodes) {
  Model::Node::visitAll(nodes, makeInitializeNodeTagsVisitor(*m_tagManager));
}

void MapDocument::clearNodeTags(const std::vector<Model::Node*>& nodes) {
  Model::Node::visitAll(nodes, makeClearNodeTagsVisitor());
}

void MapDocument::updateNodeTags(const std::vector<Model::Node*>& nodes) {
  for (auto* node : nodes) {
    node->updateTags(*m_tagManager);
  }
}

void MapDocument::updateFaceTags(const std::vector<Model::BrushFaceHandle>& faceHandles) {
  for (const auto& faceHandle : faceHandles) {
    Model::BrushNode* node = faceHandle.node();
    node->updateFaceTags(faceHandle.faceIndex(), *m_tagManager);
  }
}

void MapDocument::updateAllFaceTags() {
  m_world->accept(kdl::overload(
    [](auto&& thisLambda, Model::WorldNode* world) {
      world->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, Model::LayerNode* layer) {
      layer->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, Model::GroupNode* group) {
      group->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, Model::EntityNode* entity) {
      entity->visitChildren(thisLambda);
    },
    [&](Model::BrushNode* brush) {
      brush->initializeTags(*m_tagManager);
    },
    [](Model::PatchNode*) {}));
}

bool MapDocument::persistent() const {
  return m_path.isAbsolute() && IO::Disk::fileExists(IO::Disk::fixPath(m_path));
}

std::string MapDocument::filename() const {
  if (m_path.isEmpty()) {
    return "";
  }
  return m_path.lastComponent().asString();
}

const IO::Path& MapDocument::path() const {
  return m_path;
}

void MapDocument::setPath(const IO::Path& path) {
  m_path = path;
}

bool MapDocument::modified() const {
  return m_modificationCount != m_lastSaveModificationCount;
}

size_t MapDocument::modificationCount() const {
  return m_modificationCount;
}

void MapDocument::setLastSaveModificationCount() {
  m_lastSaveModificationCount = m_modificationCount;
  documentModificationStateDidChangeNotifier();
}

void MapDocument::clearModificationCount() {
  m_lastSaveModificationCount = m_modificationCount = 0;
  documentModificationStateDidChangeNotifier();
}

void MapDocument::connectObservers() {
  m_notifierConnection +=
    textureCollectionsWillChangeNotifier.connect(this, &MapDocument::textureCollectionsWillChange);
  m_notifierConnection +=
    textureCollectionsDidChangeNotifier.connect(this, &MapDocument::textureCollectionsDidChange);

  m_notifierConnection +=
    entityDefinitionsWillChangeNotifier.connect(this, &MapDocument::entityDefinitionsWillChange);
  m_notifierConnection +=
    entityDefinitionsDidChangeNotifier.connect(this, &MapDocument::entityDefinitionsDidChange);

  m_notifierConnection += modsWillChangeNotifier.connect(this, &MapDocument::modsWillChange);
  m_notifierConnection += modsDidChangeNotifier.connect(this, &MapDocument::modsDidChange);

  PreferenceManager& prefs = PreferenceManager::instance();
  m_notifierConnection +=
    prefs.preferenceDidChangeNotifier.connect(this, &MapDocument::preferenceDidChange);
  m_notifierConnection +=
    m_editorContext->editorContextDidChangeNotifier.connect(editorContextDidChangeNotifier);
  m_notifierConnection += commandDoneNotifier.connect(this, &MapDocument::commandDone);
  m_notifierConnection += commandUndoneNotifier.connect(this, &MapDocument::commandUndone);
  m_notifierConnection += transactionDoneNotifier.connect(this, &MapDocument::transactionDone);
  m_notifierConnection += transactionUndoneNotifier.connect(this, &MapDocument::transactionUndone);

  // tag management
  m_notifierConnection +=
    documentWasNewedNotifier.connect(this, &MapDocument::initializeAllNodeTags);
  m_notifierConnection +=
    documentWasLoadedNotifier.connect(this, &MapDocument::initializeAllNodeTags);
  m_notifierConnection += nodesWereAddedNotifier.connect(this, &MapDocument::initializeNodeTags);
  m_notifierConnection += nodesWillBeRemovedNotifier.connect(this, &MapDocument::clearNodeTags);
  m_notifierConnection += nodesDidChangeNotifier.connect(this, &MapDocument::updateNodeTags);
  m_notifierConnection += brushFacesDidChangeNotifier.connect(this, &MapDocument::updateFaceTags);
  m_notifierConnection += modsDidChangeNotifier.connect(this, &MapDocument::updateAllFaceTags);
  m_notifierConnection +=
    textureCollectionsDidChangeNotifier.connect(this, &MapDocument::updateAllFaceTags);
}

void MapDocument::textureCollectionsWillChange() {
  unsetTextures();
}

void MapDocument::textureCollectionsDidChange() {
  loadTextures();
  setTextures();
}

void MapDocument::entityDefinitionsWillChange() {
  unloadEntityDefinitions();
  clearEntityModels();
}

void MapDocument::entityDefinitionsDidChange() {
  loadEntityDefinitions();
  setEntityDefinitions();
  setEntityModels();
}

void MapDocument::modsWillChange() {
  unsetEntityModels();
  unsetEntityDefinitions();
  clearEntityModels();
}

void MapDocument::modsDidChange() {
  updateGameSearchPaths();
  setEntityDefinitions();
  setEntityModels();
}

void MapDocument::preferenceDidChange(const IO::Path& path) {
  if (isGamePathPreference(path)) {
    const Model::GameFactory& gameFactory = Model::GameFactory::instance();
    const IO::Path newGamePath = gameFactory.gamePath(m_game->gameName());
    m_game->setGamePath(newGamePath, logger());

    clearEntityModels();
    setEntityModels();

    reloadTextures();
    setTextures();
  } else if (
    path == Preferences::TextureMinFilter.path() || path == Preferences::TextureMagFilter.path()) {
    m_entityModelManager->setTextureMode(
      pref(Preferences::TextureMinFilter), pref(Preferences::TextureMagFilter));
    m_textureManager->setTextureMode(
      pref(Preferences::TextureMinFilter), pref(Preferences::TextureMagFilter));
  }
}

void MapDocument::commandDone(Command* command) {
  debug() << "Command '" << command->name() << "' executed";
}

void MapDocument::commandUndone(UndoableCommand* command) {
  debug() << "Command '" << command->name() << "' undone";
}

void MapDocument::transactionDone(const std::string& name) {
  debug() << "Transaction '" << name << "' executed";
}

void MapDocument::transactionUndone(const std::string& name) {
  debug() << "Transaction '" << name << "' undone";
}

Transaction::Transaction(std::weak_ptr<MapDocument> document, const std::string& name)
  : m_document(kdl::mem_lock(document).get())
  , m_cancelled(false) {
  begin(name);
}

Transaction::Transaction(std::shared_ptr<MapDocument> document, const std::string& name)
  : m_document(document.get())
  , m_cancelled(false) {
  begin(name);
}

Transaction::Transaction(MapDocument* document, const std::string& name)
  : m_document(document)
  , m_cancelled(false) {
  begin(name);
}

Transaction::~Transaction() {
  if (!m_cancelled)
    commit();
}

void Transaction::rollback() {
  m_document->rollbackTransaction();
}

void Transaction::cancel() {
  m_document->cancelTransaction();
  m_cancelled = true;
}

void Transaction::begin(const std::string& name) {
  m_document->startTransaction(name);
}

void Transaction::commit() {
  m_document->commitTransaction();
}
} // namespace View
} // namespace TrenchBroom
