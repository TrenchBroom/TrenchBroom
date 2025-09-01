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

#pragma once

#include "Notifier.h"
#include "NotifierConnection.h"
#include "Result.h"
#include "io/ExportOptions.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/NodeContents.h"
#include "mdl/ResourceId.h"
#include "mdl/Selection.h"

#include "vm/bbox.h"

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace kdl
{
class task_manager;
} // namespace kdl

namespace tb
{
class Color;
class Logger;
} // namespace tb

namespace tb::mdl
{
enum class MapFormat;
enum class MapTextEncoding;
enum class PasteType;
enum class TransactionScope;
enum class WrapStyle;

class BrushFaceAttributes;
class ChangeBrushFaceAttributesRequest;
class Command;
class CommandProcessor;
class CommandResult;
class EdgeHandleManager;
class EditorContext;
class EntityDefinitionManager;
class EntityModelManager;
class FaceHandleManager;
class Game;
class Grid;
class GroupNode;
class Issue;
class LayerNode;
class MaterialManager;
class Node;
class PickResult;
class PointTrace;
class RepeatStack;
class ResourceManager;
class SmartTag;
class TagManager;
class UndoableCommand;
class UVCoordSystemSnapshot;
class VertexHandleManager;
class WorldNode;

struct ProcessContext;
struct SelectionChange;
struct SoftMapBounds;

class Map
{
public:
  static const vm::bbox3d DefaultWorldBounds;
  static const std::string DefaultDocumentName;

private:
  Logger& m_logger;

  kdl::task_manager& m_taskManager;

  std::unique_ptr<ResourceManager> m_resourceManager;
  std::unique_ptr<EntityDefinitionManager> m_entityDefinitionManager;
  std::unique_ptr<EntityModelManager> m_entityModelManager;
  std::unique_ptr<MaterialManager> m_materialManager;
  std::unique_ptr<TagManager> m_tagManager;

  std::unique_ptr<EditorContext> m_editorContext;
  std::unique_ptr<Grid> m_grid;

  std::unique_ptr<Game> m_game;
  vm::bbox3d m_worldBounds;
  std::unique_ptr<WorldNode> m_world;

  std::unique_ptr<VertexHandleManager> m_vertexHandles;
  std::unique_ptr<EdgeHandleManager> m_edgeHandles;
  std::unique_ptr<FaceHandleManager> m_faceHandles;

  std::string m_currentMaterialName;

  /*
   * All actions pushed to this stack can be repeated later. The stack must be
   * primed to be cleared whenever the selection changes. The effect is that
   * changing the selection automatically begins a new "macro", but at the same
   * time the current repeat stack can still be repeated after the selection
   * was changed.
   */
  std::unique_ptr<RepeatStack> m_repeatStack;

  std::unique_ptr<CommandProcessor> m_commandProcessor;

  std::filesystem::path m_path = DefaultDocumentName;
  size_t m_lastSaveModificationCount = 0;
  size_t m_modificationCount = 0;

  mutable std::optional<Selection> m_cachedSelection;
  mutable std::optional<vm::bbox3d> m_cachedSelectionBounds;
  std::optional<vm::bbox3d> m_lastSelectionBounds;

public: // notification
  Notifier<Command&> commandDoNotifier;
  Notifier<Command&> commandDoneNotifier;
  Notifier<Command&> commandDoFailedNotifier;
  Notifier<UndoableCommand&> commandUndoNotifier;
  Notifier<UndoableCommand&> commandUndoneNotifier;
  Notifier<UndoableCommand&> commandUndoFailedNotifier;
  Notifier<const std::string&> transactionDoneNotifier;
  Notifier<const std::string&> transactionUndoneNotifier;

  Notifier<Map&> mapWillBeClearedNotifier;
  Notifier<Map&> mapWasClearedNotifier;
  Notifier<Map&> mapWasCreatedNotifier;
  Notifier<Map&> mapWasLoadedNotifier;
  Notifier<Map&> mapWasSavedNotifier;
  Notifier<> modificationStateDidChangeNotifier;

  Notifier<> editorContextDidChangeNotifier;
  Notifier<const LayerNode*> currentLayerDidChangeNotifier;
  Notifier<const std::string&> currentMaterialNameDidChangeNotifier;

  Notifier<> selectionWillChangeNotifier;
  Notifier<const SelectionChange&> selectionDidChangeNotifier;

  Notifier<const std::vector<Node*>&> nodesWereAddedNotifier;
  Notifier<const std::vector<Node*>&> nodesWillBeRemovedNotifier;
  Notifier<const std::vector<Node*>&> nodesWereRemovedNotifier;
  Notifier<const std::vector<Node*>&> nodesWillChangeNotifier;
  Notifier<const std::vector<Node*>&> nodesDidChangeNotifier;

  Notifier<const std::vector<Node*>&> nodeVisibilityDidChangeNotifier;
  Notifier<const std::vector<Node*>&> nodeLockingDidChangeNotifier;

  Notifier<GroupNode&> groupWasOpenedNotifier;
  Notifier<GroupNode&> groupWasClosedNotifier;

  Notifier<const std::vector<BrushFaceHandle>&> brushFacesDidChangeNotifier;

  Notifier<const std::vector<ResourceId>> resourcesWereProcessedNotifier;

  Notifier<> materialCollectionsWillChangeNotifier;
  Notifier<> materialCollectionsDidChangeNotifier;

  Notifier<> materialUsageCountsDidChangeNotifier;

  Notifier<> entityDefinitionsWillChangeNotifier;
  Notifier<> entityDefinitionsDidChangeNotifier;

  Notifier<> modsWillChangeNotifier;
  Notifier<> modsDidChangeNotifier;

private:
  NotifierConnection m_notifierConnection;

public: // misc
  explicit Map(kdl::task_manager& taskManager, Logger& logger);
  ~Map();

  Logger& logger();

  kdl::task_manager& taskManager();

  EntityDefinitionManager& entityDefinitionManager();
  const EntityDefinitionManager& entityDefinitionManager() const;

  EntityModelManager& entityModelManager();
  const EntityModelManager& entityModelManager() const;

  MaterialManager& materialManager();
  const MaterialManager& materialManager() const;

  TagManager& tagManager();
  const TagManager& tagManager() const;

  EditorContext& editorContext();
  const EditorContext& editorContext() const;

  Grid& grid();
  const Grid& grid() const;

  const Game* game() const;
  const vm::bbox3d& worldBounds() const;
  WorldNode* world() const;

  MapTextEncoding encoding() const;

  VertexHandleManager& vertexHandles();
  const VertexHandleManager& vertexHandles() const;

  EdgeHandleManager& edgeHandles();
  const EdgeHandleManager& edgeHandles() const;

  FaceHandleManager& faceHandles();
  const FaceHandleManager& faceHandles() const;

  const std::string& currentMaterialName() const;
  void setCurrentMaterialName(const std::string& currentMaterialName);

public: // persistence
  Result<void> create(
    MapFormat mapFormat, const vm::bbox3d& worldBounds, std::unique_ptr<Game> game);
  Result<void> load(
    MapFormat mapFormat,
    const vm::bbox3d& worldBounds,
    std::unique_ptr<Game> game,
    const std::filesystem::path& path);
  Result<void> reload();
  void save();
  void saveAs(const std::filesystem::path& path);
  void saveTo(const std::filesystem::path& path);
  Result<void> exportAs(const io::ExportOptions& options) const;

  void clear();

  bool persistent() const;
  std::string filename() const;
  const std::filesystem::path& path() const;

  bool modified() const;
  size_t modificationCount() const;

  void incModificationCount(size_t delta = 1);
  void decModificationCount(size_t delta = 1);

private:
  void setPath(const std::filesystem::path& path);
  void setLastSaveModificationCount();
  void clearModificationCount();

public: // selection management
  const Selection& selection() const;

  void selectAllNodes();
  void selectNodes(const std::vector<Node*>& nodes);

  void selectSiblingNodes();
  void selectTouchingNodes(bool del);
  void selectTouchingNodes(vm::axis::type cameraAxis, bool del);
  void selectContainedNodes(bool del);
  void selectNodesWithFilePosition(const std::vector<size_t>& positions);
  void selectBrushesWithMaterial(const Material* material);
  void invertNodeSelection();

  void selectAllInLayers(const std::vector<LayerNode*>& layers);
  bool canSelectAllInLayers(const std::vector<LayerNode*>& layers) const;

  bool canSelectLinkedGroups() const;
  void selectLinkedGroups();

  void selectBrushFaces(const std::vector<BrushFaceHandle>& handles);
  void selectBrushFacesWithMaterial(const Material* material);
  void convertToFaceSelection();

  void deselectAll();
  void deselectNodes(const std::vector<Node*>& nodes);
  void deselectBrushFaces(const std::vector<BrushFaceHandle>& handles);

  const vm::bbox3d referenceBounds() const;
  const std::optional<vm::bbox3d>& lastSelectionBounds() const;
  const std::optional<vm::bbox3d>& selectionBounds() const;

public: // node management
  bool updateNodeContents(
    const std::string& commandName,
    std::vector<std::pair<Node*, NodeContents>> nodesToSwap,
    std::vector<GroupNode*> changedLinkedGroups);
  bool updateNodeContents(
    const std::string& commandName,
    std::vector<std::pair<Node*, NodeContents>> nodesToSwap);

public: // world management
  SoftMapBounds softMapBounds() const;
  void setSoftMapBounds(const SoftMapBounds& bounds);

  std::vector<std::filesystem::path> externalSearchPaths() const;
  void updateGameSearchPaths();

  std::vector<std::string> mods() const;
  void setMods(const std::vector<std::string>& mods);
  std::string defaultMod() const;

private:
  void setWorld(
    const vm::bbox3d& worldBounds,
    std::unique_ptr<WorldNode> worldNode,
    std::unique_ptr<Game> game,
    const std::filesystem::path& path);
  void clearWorld();

public: // picking
  void pick(const vm::ray3d& pickRay, PickResult& pickResult) const;
  std::vector<Node*> findNodesContaining(const vm::vec3d& point) const;

public: // tag management
  void registerSmartTags();
  const std::vector<SmartTag>& smartTags() const;
  bool isRegisteredSmartTag(const std::string& name) const;
  const SmartTag& smartTag(const std::string& name) const;
  bool isRegisteredSmartTag(size_t index) const;
  const SmartTag& smartTag(size_t index) const;

private:
  void initializeAllNodeTags();
  void initializeNodeTags(const std::vector<Node*>& nodes);
  void clearNodeTags(const std::vector<Node*>& nodes);
  void updateNodeTags(const std::vector<Node*>& nodes);

  void updateFaceTags(const std::vector<BrushFaceHandle>& faces);
  void updateAllFaceTags();

  void updateFaceTagsAfterResourcesWhereProcessed(
    const std::vector<ResourceId>& resourceIds);

private: // validation
  void registerValidators();

public:
  void setIssueHidden(const Issue& issue, bool hidden);

private:
  void loadAssets();
  void clearAssets();

  void loadEntityDefinitions();
  void clearEntityDefinitions();

  void reloadMaterials();
  void loadMaterials();
  void clearMaterials();

  void setMaterials();
  void setMaterials(const std::vector<Node*>& nodes);
  void setMaterials(const std::vector<BrushFaceHandle>& faceHandles);
  void unsetMaterials();
  void unsetMaterials(const std::vector<Node*>& nodes);

  void setEntityDefinitions();
  void setEntityDefinitions(const std::vector<Node*>& nodes);
  void unsetEntityDefinitions();
  void unsetEntityDefinitions(const std::vector<Node*>& nodes);

  void clearEntityModels();

  void setEntityModels();
  void setEntityModels(const std::vector<Node*>& nodes);
  void unsetEntityModels();
  void unsetEntityModels(const std::vector<Node*>& nodes);

public: // resource processing
  void processResourcesSync(const ProcessContext& processContext);
  void processResourcesAsync(const ProcessContext& processContext);
  bool needsResourceProcessing() const;

public: // command processing
  bool canUndoCommand() const;
  bool canRedoCommand() const;
  const std::string& undoCommandName() const;
  const std::string& redoCommandName() const;
  void undoCommand();
  void redoCommand();

  bool isCommandCollationEnabled() const;
  void setIsCommandCollationEnabled(bool isCommandCollationEnabled);

  using RepeatableCommand = std::function<void()>;
  void pushRepeatableCommand(RepeatableCommand command);
  bool canRepeatCommands() const;
  void repeatCommands();
  void clearRepeatableCommands();

  void startTransaction(std::string name, TransactionScope scope);
  void rollbackTransaction();
  bool commitTransaction();
  void cancelTransaction();

  bool isCurrentDocumentStateObservable() const;

  bool throwExceptionDuringCommand();

  std::unique_ptr<CommandResult> execute(std::unique_ptr<Command>&& command);
  std::unique_ptr<CommandResult> executeAndStore(
    std::unique_ptr<UndoableCommand>&& command);

private: // observers
  void connectObservers();
  void mapWasCreated(Map& map);
  void mapWasLoaded(Map& map);
  void nodesWereAdded(const std::vector<Node*>& nodes);
  void nodesWereRemoved(const std::vector<Node*>& nodes);
  void nodesDidChange(const std::vector<Node*>& nodes);
  void selectionWillChange();
  void selectionDidChange(const SelectionChange& selectionChange);
  void materialCollectionsWillChange();
  void materialCollectionsDidChange();
  void entityDefinitionsWillChange();
  void entityDefinitionsDidChange();
  void modsWillChange();
  void modsDidChange();
  void preferenceDidChange(const std::filesystem::path& path);
  void commandDone(Command& command);
  void commandUndone(UndoableCommand& command);
  void transactionDone(const std::string& name);
  void transactionUndone(const std::string& name);
};

} // namespace tb::mdl
