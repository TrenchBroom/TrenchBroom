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
#include "mdl/ColorRange.h"
#include "mdl/Game.h"
#include "mdl/MapFacade.h"
#include "mdl/NodeContents.h"
#include "mdl/PointTrace.h"
#include "mdl/Selection.h"
#include "mdl/VertexHandleManager.h"

#include "vm/bbox.h"
#include "vm/ray.h"
#include "vm/util.h"

#include <filesystem>
#include <map>
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
} // namespace tb

namespace tb::mdl
{
class Brush;
class BrushFace;
class BrushFaceAttributes;
class BrushFaceHandle;
class Command;
class CommandProcessor;
class CommandResult;
class EditorContext;
class Entity;
struct EntityDefinition;
class EntityDefinitionFileSpec;
class EntityDefinitionManager;
class EntityModelManager;
class Game;
class Grid;
class Issue;
class Material;
class MaterialManager;
enum class PasteType;
class PickResult;
class PointTrace;
class Portals;
class ResourceId;
class ResourceManager;
struct SelectionChange;
class SmartTag;
class TagManager;
enum class TransactionScope;
class UndoableCommand;
class UVCoordSystemSnapshot;
class WorldNode;
enum class MapFormat;
enum class WrapStyle;
struct ProcessContext;

struct PointFile
{
  PointTrace trace;
  std::filesystem::path path;
};

struct PortalFile
{
  std::vector<vm::polygon3f> portals;
  std::filesystem::path path;
};

class Map
{
public:
  static const vm::bbox3d DefaultWorldBounds;
  static const std::string DefaultDocumentName;

private:
  kdl::task_manager& m_taskManager;

  vm::bbox3d m_worldBounds = DefaultWorldBounds;
  std::shared_ptr<Game> m_game;
  std::unique_ptr<WorldNode> m_world;

  std::optional<PointFile> m_pointFile;
  std::optional<PortalFile> m_portalFile;

  std::unique_ptr<ResourceManager> m_resourceManager;
  std::unique_ptr<EntityDefinitionManager> m_entityDefinitionManager;
  std::unique_ptr<EntityModelManager> m_entityModelManager;
  std::unique_ptr<MaterialManager> m_materialManager;
  std::unique_ptr<TagManager> m_tagManager;

  std::unique_ptr<EditorContext> m_editorContext;
  std::unique_ptr<Grid> m_grid;

  std::filesystem::path m_path = DefaultDocumentName;
  size_t m_lastSaveModificationCount = 0;
  size_t m_modificationCount = 0;

  mutable std::optional<Selection> m_cachedSelection;
  mutable std::optional<vm::bbox3d> m_cachedSelectionBounds;
  std::optional<vm::bbox3d> m_lastSelectionBounds;

  VertexHandleManager m_vertexHandles;
  EdgeHandleManager m_edgeHandles;
  FaceHandleManager m_faceHandles;

  std::string m_currentMaterialName = BrushFaceAttributes::NoMaterialName;

  std::unique_ptr<CommandProcessor> m_commandProcessor;

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
  Notifier<Map&> mapWasNewedNotifier;
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

  Notifier<> pointFileWasLoadedNotifier;
  Notifier<> pointFileWasUnloadedNotifier;

  Notifier<> portalFileWasLoadedNotifier;
  Notifier<> portalFileWasUnloadedNotifier;

private:
  NotifierConnection m_notifierConnection;

public:
  explicit Map(kdl::task_manager& taskManager);
  ~Map();

public: // accessors and such
  kdl::task_manager& taskManager();

  Logger& logger();

  std::shared_ptr<Game> game() const;
  const vm::bbox3d& worldBounds() const;
  WorldNode* world() const;

  bool isGamePathPreference(const std::filesystem::path& path) const;

  LayerNode* currentLayer() const;

public:
  void setCurrentLayer(LayerNode* currentLayer);
  bool canSetCurrentLayer(LayerNode* currentLayer) const;

  GroupNode* currentGroup() const;
  /**
   * Returns the current group if one is open, otherwise the world.
   */
  Node* currentGroupOrWorld() const;
  /**
   * Suggests a parent to use for new nodes.
   *
   * If reference nodes are given, return the parent (either a group, if there is one,
   * otherwise the layer) of the first node in the given vector.
   *
   * Otherwise, returns the current group if one is open, otherwise the current layer.
   */
  Node* parentForNodes(
    const std::vector<Node*>& referenceNodes = std::vector<Node*>()) const;

  EditorContext& editorContext() const;

  EntityDefinitionManager& entityDefinitionManager();
  EntityModelManager& entityModelManager();
  MaterialManager& materialManager();

  Grid& grid() const;

  PointTrace* pointTrace();
  const std::vector<vm::polygon3f>* portals() const;

public: // new, load, save map
  Result<void> newMap(
    MapFormat mapFormat, const vm::bbox3d& worldBounds, std::shared_ptr<Game> game);
  Result<void> loadMap(
    MapFormat mapFormat,
    const vm::bbox3d& worldBounds,
    std::shared_ptr<Game> game,
    const std::filesystem::path& path);
  void saveMap();
  void saveMapAs(const std::filesystem::path& path);
  void saveMapTo(const std::filesystem::path& path);
  Result<void> exportMapAs(const io::ExportOptions& options);

private:
  void doSaveMap(const std::filesystem::path& path);
  void clearMap();

public: // text encoding
  mdl::MapTextEncoding encoding() const;

public: // copy and paste
  std::string serializeSelectedNodes();
  std::string serializeSelectedBrushFaces();

  PasteType paste(const std::string& str);

private:
  bool pasteNodes(const std::vector<Node*>& nodes);
  bool pasteBrushFaces(const std::vector<BrushFace>& faces);

public: // point file management
  void loadPointFile(std::filesystem::path path);
  bool isPointFileLoaded() const;
  bool canReloadPointFile() const;
  void reloadPointFile();
  void unloadPointFile();

public: // portal file management
  void loadPortalFile(std::filesystem::path path);
  bool isPortalFileLoaded() const;
  bool canReloadPortalFile() const;
  void reloadPortalFile();
  void unloadPortalFile();

public: // selection
  const Selection& selection() const;

  VertexHandleManager& vertexHandles();
  EdgeHandleManager& edgeHandles();
  FaceHandleManager& faceHandles();

  const vm::bbox3d referenceBounds() const;
  const std::optional<vm::bbox3d>& lastSelectionBounds() const;
  const std::optional<vm::bbox3d>& selectionBounds() const;
  const std::string& currentMaterialName() const;
  void setCurrentMaterialName(const std::string& currentMaterialName);

  void selectAllNodes();
  void selectSiblings();
  void selectTouching(bool del);
  void selectInside(bool del);
  void selectInverse();
  void selectNodesWithFilePosition(const std::vector<size_t>& positions);
  void selectNodes(const std::vector<Node*>& nodes);
  void selectBrushFaces(const std::vector<BrushFaceHandle>& handles);
  void convertToFaceSelection();
  void selectFacesWithMaterial(const Material* material);
  void selectBrushesWithMaterial(const Material* material);
  void selectTall(vm::axis::type cameraAxis);

  void deselectAll();
  void deselectNodes(const std::vector<Node*>& nodes);
  void deselectBrushFaces(const std::vector<BrushFaceHandle>& handles);

public: // adding, removing, reparenting, and duplicating nodes, declared in MapFacade
        // interface
  std::vector<Node*> addNodes(const std::map<Node*, std::vector<Node*>>& nodes);
  void removeNodes(const std::vector<Node*>& nodes);

private:
  std::map<Node*, std::vector<Node*>> collectRemovableParents(
    const std::map<Node*, std::vector<Node*>>& nodes) const;

  struct CompareByAncestry;
  std::vector<Node*> removeImplicitelyRemovedNodes(std::vector<Node*> nodes) const;

  void closeRemovedGroups(const std::map<Node*, std::vector<Node*>>& toRemove);

public:
  bool reparentNodes(const std::map<Node*, std::vector<Node*>>& nodesToAdd);

private:
  bool checkReparenting(const std::map<Node*, std::vector<Node*>>& nodesToAdd) const;

public:
  void remove();
  void duplicate();

public: // entity management
  EntityNode* createPointEntity(
    const EntityDefinition& definition, const vm::vec3d& delta);
  EntityNode* createBrushEntity(const EntityDefinition& definition);

public: // group management
  GroupNode* groupSelection(const std::string& name);
  void mergeSelectedGroupsWithGroup(GroupNode* group);

public:
  void ungroupSelection();
  void renameGroups(const std::string& name);

  void openGroup(GroupNode* group);
  void closeGroup();

  /**
   * Creates a new group that is linked to the currently selected group and returns the
   * newly created group.
   *
   * If the current selection does not consist of exactly one group, then null is
   * returned.
   */
  GroupNode* createLinkedDuplicate();
  bool canCreateLinkedDuplicate() const;

  /**
   * Selects all groups linked to the currently selected groups.
   *
   * Nothing happens if the current selection does not consist of only groups.
   */
  void selectLinkedGroups();
  bool canSelectLinkedGroups() const;

  void linkGroups(const std::vector<GroupNode*>& groupNodes);
  void unlinkGroups(const std::vector<GroupNode*>& groupNodes);

  /**
   * Unlinks the selected linked groups.
   *
   * For every set of selected linked groups that belong to the same link set, the
   * selected groups will be added to a new link set with the effect that these groups
   * will still be linked to each other, but they will no longer be linked to any other
   * member of their original link set that was not selected.
   */
  void separateLinkedGroups();
  bool canSeparateLinkedGroups() const;

  bool canUpdateLinkedGroups(const std::vector<Node*>& nodes) const;

protected:
  void setHasPendingChanges(
    const std::vector<GroupNode*>& groupNodes, bool hasPendingChanges);
  bool updateLinkedGroups();

private:
  void separateSelectedLinkedGroups(bool relinkGroups);

public: // layer management
  void renameLayer(LayerNode* layer, const std::string& name);

private:
  enum class MoveDirection
  {
    Up,
    Down
  };
  bool moveLayerByOne(LayerNode* layerNode, MoveDirection direction);

public:
  void moveLayer(LayerNode* layer, int offset);
  bool canMoveLayer(LayerNode* layer, int offset) const;
  void moveSelectionToLayer(LayerNode* layer);
  bool canMoveSelectionToLayer(LayerNode* layer) const;
  void hideLayers(const std::vector<LayerNode*>& layers);
  bool canHideLayers(const std::vector<LayerNode*>& layers) const;
  void isolateLayers(const std::vector<LayerNode*>& layers);
  bool canIsolateLayers(const std::vector<LayerNode*>& layers) const;
  void setOmitLayerFromExport(LayerNode* layerNode, bool omitFromExport);
  void selectAllInLayers(const std::vector<LayerNode*>& layers);
  bool canSelectAllInLayers(const std::vector<LayerNode*>& layers) const;

public: // modifying transient node attributes, declared in MapFacade interface
  void isolate();
  void hide(std::vector<Node*> nodes); // Don't take the nodes by reference!
  void hideSelection();
  void show(const std::vector<Node*>& nodes);
  void showAll();
  void ensureVisible(const std::vector<Node*>& nodes);
  void resetVisibility(const std::vector<Node*>& nodes);

  void lock(const std::vector<Node*>& nodes);
  void unlock(const std::vector<Node*>& nodes);
  void ensureUnlocked(const std::vector<Node*>& nodes);
  void resetLock(const std::vector<Node*>& nodes);

private:
  void downgradeShownToInherit(const std::vector<Node*>& nodes);
  void downgradeUnlockedToInherit(const std::vector<Node*>& nodes);

public: // modifying objects, declared in MapFacade interface
  bool swapNodeContents(
    const std::string& commandName,
    std::vector<std::pair<Node*, NodeContents>> nodesToSwap,
    std::vector<GroupNode*> changedLinkedGroups);
  bool swapNodeContents(
    const std::string& commandName,
    std::vector<std::pair<Node*, NodeContents>> nodesToSwap);
  bool transform(const std::string& commandName, const vm::mat4x4d& transformation);

  bool translate(const vm::vec3d& delta);
  bool rotate(const vm::vec3d& center, const vm::vec3d& axis, double angle);
  bool scale(const vm::bbox3d& oldBBox, const vm::bbox3d& newBBox);
  bool scale(const vm::vec3d& center, const vm::vec3d& scaleFactors);
  bool shear(const vm::bbox3d& box, const vm::vec3d& sideToShear, const vm::vec3d& delta);
  bool flip(const vm::vec3d& center, vm::axis::type axis);

public: // CSG operations, declared in MapFacade interface
  bool createBrush(const std::vector<vm::vec3d>& points);
  bool csgConvexMerge();
  bool csgSubtract();
  bool csgIntersect();
  bool csgHollow();

public: // Clipping operations, declared in MapFacade interface
  bool clipBrushes(const vm::vec3d& p1, const vm::vec3d& p2, const vm::vec3d& p3);

public: // modifying entity properties, declared in MapFacade interface
  bool setProperty(
    const std::string& key, const std::string& value, bool defaultToProtected = false);
  bool renameProperty(const std::string& oldKey, const std::string& newKey);
  bool removeProperty(const std::string& key);

  bool convertEntityColorRange(const std::string& key, ColorRange::Type range);
  bool updateSpawnflag(const std::string& key, size_t flagIndex, bool setFlag);

  bool setProtectedProperty(const std::string& key, bool value);
  bool clearProtectedProperties();
  bool canClearProtectedProperties() const;

  void setDefaultProperties(SetDefaultPropertyMode mode);

public: // brush resizing, declared in MapFacade interface
  bool extrudeBrushes(const std::vector<vm::polygon3d>& faces, const vm::vec3d& delta);

public:
  bool setFaceAttributes(const BrushFaceAttributes& attributes);
  bool setFaceAttributesExceptContentFlags(const BrushFaceAttributes& attributes);
  bool setFaceAttributes(const ChangeBrushFaceAttributesRequest& request);
  bool copyUVFromFace(
    const UVCoordSystemSnapshot& coordSystemSnapshot,
    const BrushFaceAttributes& attribs,
    const vm::plane3d& sourceFacePlane,
    WrapStyle wrapStyle);
  bool translateUV(
    const vm::vec3f& cameraUp, const vm::vec3f& cameraRight, const vm::vec2f& delta);
  bool rotateUV(float angle);
  bool shearUV(const vm::vec2f& factors);
  bool flipUV(
    const vm::vec3f& cameraUp,
    const vm::vec3f& cameraRight,
    vm::direction cameraRelativeFlipDirection);

public: // modifying vertices, declared in MapFacade interface
  bool snapVertices(double snapTo);

  TransformVerticesResult transformVertices(
    std::vector<vm::vec3d> vertexPositions, const vm::mat4x4d& transform);
  bool transformEdges(
    std::vector<vm::segment3d> edgePositions, const vm::mat4x4d& transform);
  bool transformFaces(
    std::vector<vm::polygon3d> facePositions, const vm::mat4x4d& transform);

  bool addVertex(const vm::vec3d& vertexPosition);
  bool removeVertices(
    const std::string& commandName, std::vector<vm::vec3d> vertexPositions);

public: // debug commands
  void printVertices();
  bool throwExceptionDuringCommand();

public: // command processing
  bool canUndoCommand() const;
  bool canRedoCommand() const;
  const std::string& undoCommandName() const;
  const std::string& redoCommandName() const;
  void undoCommand();
  void redoCommand();
  bool canRepeatCommands() const;
  void repeatCommands();
  void clearRepeatableCommands();

public: // transactions
  void startTransaction(std::string name, TransactionScope scope);
  void rollbackTransaction();
  bool commitTransaction();
  void cancelTransaction();

  bool isCurrentDocumentStateObservable() const;

private:
  std::unique_ptr<CommandResult> execute(std::unique_ptr<Command>&& command);
  std::unique_ptr<CommandResult> executeAndStore(
    std::unique_ptr<UndoableCommand>&& command);

public: // asset state management
  void processResourcesSync(const ProcessContext& processContext);
  void processResourcesAsync(const ProcessContext& processContext);
  bool needsResourceProcessing();

public: // picking
  void pick(const vm::ray3d& pickRay, PickResult& pickResult) const;
  std::vector<Node*> findNodesContaining(const vm::vec3d& point) const;

private: // world management
  void setWorld(
    const vm::bbox3d& worldBounds,
    std::unique_ptr<WorldNode> worldNode,
    std::shared_ptr<Game> game,
    const std::filesystem::path& path);
  void clearWorld();

public: // asset management
  EntityDefinitionFileSpec entityDefinitionFile() const;
  std::vector<EntityDefinitionFileSpec> allEntityDefinitionFiles() const;
  void setEntityDefinitionFile(const EntityDefinitionFileSpec& spec);

  // For testing
  void setEntityDefinitions(std::vector<EntityDefinition> definitions);

  void reloadMaterialCollections();
  void reloadEntityDefinitions();

  std::vector<std::filesystem::path> enabledMaterialCollections() const;
  std::vector<std::filesystem::path> disabledMaterialCollections() const;

  void setEnabledMaterialCollections(
    const std::vector<std::filesystem::path>& enabledMaterialCollections);

private:
  void loadAssets();
  void unloadAssets();

  void loadEntityDefinitions();
  void unloadEntityDefinitions();

  void loadEntityModels();
  void unloadEntityModels();

protected:
  void reloadMaterials();
  void loadMaterials();
  void unloadMaterials();

  void setMaterials();
  void setMaterials(const std::vector<Node*>& nodes);
  void setMaterials(const std::vector<BrushFaceHandle>& faceHandles);
  void unsetMaterials();
  void unsetMaterials(const std::vector<Node*>& nodes);

  void setEntityDefinitions();
  void setEntityDefinitions(const std::vector<Node*>& nodes);
  void unsetEntityDefinitions();
  void unsetEntityDefinitions(const std::vector<Node*>& nodes);
  void reloadEntityDefinitionsInternal();

  void clearEntityModels();

  void setEntityModels();
  void setEntityModels(const std::vector<Node*>& nodes);
  void unsetEntityModels();
  void unsetEntityModels(const std::vector<Node*>& nodes);

protected: // search paths and mods
  std::vector<std::filesystem::path> externalSearchPaths() const;
  void updateGameSearchPaths();

public:
  std::vector<std::string> mods() const;
  void setMods(const std::vector<std::string>& mods);
  std::string defaultMod() const;

public: // map soft bounds
  void setSoftMapBounds(const Game::SoftMapBounds& bounds);
  Game::SoftMapBounds softMapBounds() const;

private: // validator management
  void registerValidators();

public:
  void setIssueHidden(const Issue& issue, bool hidden);

public:                     // tag management
  void registerSmartTags(); // public for testing
  const std::vector<SmartTag>& smartTags() const;
  bool isRegisteredSmartTag(const std::string& name) const;
  const SmartTag& smartTag(const std::string& name) const;
  bool isRegisteredSmartTag(size_t index) const;
  const SmartTag& smartTag(size_t index) const;

public: // modification count
  void incModificationCount(size_t delta = 1);
  void decModificationCount(size_t delta = 1);

private:
  void initializeAllNodeTags();
  void initializeNodeTags(const std::vector<Node*>& nodes);
  void clearNodeTags(const std::vector<Node*>& nodes);
  void updateNodeTags(const std::vector<Node*>& nodes);

  void updateFaceTags(const std::vector<BrushFaceHandle>& faces);
  void updateAllFaceTags();

  void updateFaceTagsAfterResourcesWhereProcessed(
    const std::vector<ResourceId>& resourceIds);

public: // document path
  bool persistent() const;
  std::string filename() const;
  const std::filesystem::path& path() const;

private:
  void setPath(const std::filesystem::path& path);

public: // modification count
  bool modified() const;
  size_t modificationCount() const;

private:
  void setLastSaveModificationCount();
  void clearModificationCount();

private: // observers
  void connectObservers();
  void mapWasNewed(Map& map);
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
