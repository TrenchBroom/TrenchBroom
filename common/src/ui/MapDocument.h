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
#include "mdl/PortalFile.h"
#include "mdl/Selection.h"
#include "ui/Actions.h"
#include "ui/CachingLogger.h"
#include "ui/VertexHandleManager.h"

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
class EditorContext;
class Entity;
struct EntityDefinition;
class EntityDefinitionFileSpec;
class EntityDefinitionManager;
class EntityModelManager;
class Game;
class Issue;
class Material;
class MaterialManager;
class PickResult;
class PointTrace;
class PortalFile;
class ResourceId;
class ResourceManager;
class SmartTag;
class TagManager;
class UVCoordSystemSnapshot;
class WorldNode;
enum class MapFormat;
enum class WrapStyle;
struct ProcessContext;
} // namespace tb::mdl

namespace tb::ui
{
class Command;
class CommandResult;
class Grid;
enum class PasteType;
class RepeatStack;
struct SelectionChange;
class UndoableCommand;
class ViewEffectsService;
enum class MapTextEncoding;
enum class TransactionScope;
class AsyncTaskRunner;

struct PointFile
{
  mdl::PointTrace trace;
  std::filesystem::path path;
};

struct PortalFile
{
  mdl::PortalFile portalFile;
  std::filesystem::path path;
};

class MapDocument : public mdl::MapFacade, public CachingLogger
{
public:
  static const vm::bbox3d DefaultWorldBounds;
  static const std::string DefaultDocumentName;

protected:
  kdl::task_manager& m_taskManager;

  vm::bbox3d m_worldBounds = DefaultWorldBounds;
  std::shared_ptr<mdl::Game> m_game;
  std::unique_ptr<mdl::WorldNode> m_world;

  std::optional<PointFile> m_pointFile;
  std::optional<PortalFile> m_portalFile;

  std::unique_ptr<mdl::ResourceManager> m_resourceManager;
  std::unique_ptr<mdl::EntityDefinitionManager> m_entityDefinitionManager;
  std::unique_ptr<mdl::EntityModelManager> m_entityModelManager;
  std::unique_ptr<mdl::MaterialManager> m_materialManager;
  std::unique_ptr<mdl::TagManager> m_tagManager;

  std::unique_ptr<mdl::EditorContext> m_editorContext;
  std::unique_ptr<Grid> m_grid;

  using ActionList = std::vector<Action>;
  ActionList m_tagActions;
  ActionList m_entityDefinitionActions;

  std::filesystem::path m_path = DefaultDocumentName;
  size_t m_lastSaveModificationCount = 0;
  size_t m_modificationCount = 0;

  mdl::Selection m_selection;

  VertexHandleManager m_vertexHandles;
  EdgeHandleManager m_edgeHandles;
  FaceHandleManager m_faceHandles;

  mdl::LayerNode* m_currentLayer = nullptr;
  std::string m_currentMaterialName = mdl::BrushFaceAttributes::NoMaterialName;
  std::optional<vm::bbox3d> m_lastSelectionBounds;
  mutable std::optional<vm::bbox3d> m_selectionBounds;

  ViewEffectsService* m_viewEffectsService = nullptr;

  /*
   * All actions pushed to this stack can be repeated later. The stack must be
   * primed to be cleared whenever the selection changes. The effect is that
   * changing the selection automatically begins a new "macro", but at the same
   * time the current repeat stack can still be repeated after the selection
   * was changed.
   */
  std::unique_ptr<RepeatStack> m_repeatStack;

public: // notification
  Notifier<Command&> commandDoNotifier;
  Notifier<Command&> commandDoneNotifier;
  Notifier<Command&> commandDoFailedNotifier;
  Notifier<UndoableCommand&> commandUndoNotifier;
  Notifier<UndoableCommand&> commandUndoneNotifier;
  Notifier<UndoableCommand&> commandUndoFailedNotifier;
  Notifier<const std::string&> transactionDoneNotifier;
  Notifier<const std::string&> transactionUndoneNotifier;

  Notifier<MapDocument*> documentWillBeClearedNotifier;
  Notifier<MapDocument*> documentWasClearedNotifier;
  Notifier<MapDocument*> documentWasNewedNotifier;
  Notifier<MapDocument*> documentWasLoadedNotifier;
  Notifier<MapDocument*> documentWasSavedNotifier;
  Notifier<> documentModificationStateDidChangeNotifier;

  Notifier<> editorContextDidChangeNotifier;
  Notifier<const mdl::LayerNode*> currentLayerDidChangeNotifier;
  Notifier<const std::string&> currentMaterialNameDidChangeNotifier;

  Notifier<> selectionWillChangeNotifier;
  Notifier<const SelectionChange&> selectionDidChangeNotifier;

  Notifier<const std::vector<mdl::Node*>&> nodesWereAddedNotifier;
  Notifier<const std::vector<mdl::Node*>&> nodesWillBeRemovedNotifier;
  Notifier<const std::vector<mdl::Node*>&> nodesWereRemovedNotifier;
  Notifier<const std::vector<mdl::Node*>&> nodesWillChangeNotifier;
  Notifier<const std::vector<mdl::Node*>&> nodesDidChangeNotifier;

  Notifier<const std::vector<mdl::Node*>&> nodeVisibilityDidChangeNotifier;
  Notifier<const std::vector<mdl::Node*>&> nodeLockingDidChangeNotifier;

  Notifier<mdl::GroupNode*> groupWasOpenedNotifier;
  Notifier<mdl::GroupNode*> groupWasClosedNotifier;

  Notifier<const std::vector<mdl::BrushFaceHandle>&> brushFacesDidChangeNotifier;

  Notifier<const std::vector<mdl::ResourceId>> resourcesWereProcessedNotifier;

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

protected:
  explicit MapDocument(kdl::task_manager& taskManager);

public:
  ~MapDocument() override;

public: // accessors and such
  kdl::task_manager& taskManager();

  Logger& logger();

  std::shared_ptr<mdl::Game> game() const override;
  const vm::bbox3d& worldBounds() const;
  mdl::WorldNode* world() const;

  bool isGamePathPreference(const std::filesystem::path& path) const;

  mdl::LayerNode* currentLayer() const override;

protected:
  mdl::LayerNode* performSetCurrentLayer(mdl::LayerNode* currentLayer);

public:
  void setCurrentLayer(mdl::LayerNode* currentLayer);
  bool canSetCurrentLayer(mdl::LayerNode* currentLayer) const;

  mdl::GroupNode* currentGroup() const override;
  /**
   * Returns the current group if one is open, otherwise the world.
   */
  mdl::Node* currentGroupOrWorld() const override;
  /**
   * Suggests a parent to use for new nodes.
   *
   * If reference nodes are given, return the parent (either a group, if there is one,
   * otherwise the layer) of the first node in the given vector.
   *
   * Otherwise, returns the current group if one is open, otherwise the current layer.
   */
  mdl::Node* parentForNodes(
    const std::vector<mdl::Node*>& referenceNodes =
      std::vector<mdl::Node*>()) const override;

  mdl::EditorContext& editorContext() const;

  mdl::EntityDefinitionManager& entityDefinitionManager() override;
  mdl::EntityModelManager& entityModelManager() override;
  mdl::MaterialManager& materialManager() override;

  Grid& grid() const;

  mdl::PointTrace* pointFile();
  const mdl::PortalFile* portalFile() const;

  void setViewEffectsService(ViewEffectsService* viewEffectsService);

public: // tag and entity definition actions
  template <typename ActionVisitor>
  void visitTagActions(const ActionVisitor& visitor) const
  {
    visitActions(visitor, m_tagActions);
  }

  template <typename ActionVisitor>
  void visitEntityDefinitionActions(const ActionVisitor& visitor) const
  {
    visitActions(visitor, m_entityDefinitionActions);
  }

private: // tag and entity definition actions
  template <typename ActionVisitor>
  void visitActions(const ActionVisitor& visitor, const ActionList& actions) const
  {
    for (const auto& action : actions)
    {
      visitor(action);
    }
  }

  void createTagActions();
  void clearTagActions();
  void createEntityDefinitionActions();

public: // new, load, save document
  Result<void> newDocument(
    mdl::MapFormat mapFormat,
    const vm::bbox3d& worldBounds,
    std::shared_ptr<mdl::Game> game);
  Result<void> loadDocument(
    mdl::MapFormat mapFormat,
    const vm::bbox3d& worldBounds,
    std::shared_ptr<mdl::Game> game,
    const std::filesystem::path& path);
  void saveDocument();
  void saveDocumentAs(const std::filesystem::path& path);
  void saveDocumentTo(const std::filesystem::path& path);
  Result<void> exportDocumentAs(const io::ExportOptions& options);

private:
  void doSaveDocument(const std::filesystem::path& path);
  void clearDocument();

public: // text encoding
  MapTextEncoding encoding() const;

public: // copy and paste
  std::string serializeSelectedNodes();
  std::string serializeSelectedBrushFaces();

  PasteType paste(const std::string& str);

private:
  bool pasteNodes(const std::vector<mdl::Node*>& nodes);
  bool pasteBrushFaces(const std::vector<mdl::BrushFace>& faces);

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
  bool hasSelection() const override;
  bool hasSelectedNodes() const override;
  bool hasSelectedBrushFaces() const override;
  bool hasAnySelectedBrushFaces() const override;

  /**
   * For commands that modify entities, this returns all entities that should be acted on,
   * based on the current selection.
   *
   * - selected brushes/patches act on their parent entities
   * - selected groups implicitly act on any contained entities
   *
   * If multiple linked groups are selected, returns entities from all of them, so
   * attempting to perform commands on all of them will be blocked as a conflict.
   */
  std::vector<mdl::EntityNodeBase*> allSelectedEntityNodes() const override;

  /**
   * For commands that modify brushes, this returns all brushes that should be acted on,
   * based on the current selection.
   *
   * - selected groups implicitly act on any contained brushes
   *
   * If multiple linked groups are selected, returns brushes from all of them, so
   * attempting to perform commands on all of them will be blocked as a conflict.
   */
  std::vector<mdl::BrushNode*> allSelectedBrushNodes() const;
  bool hasAnySelectedBrushNodes() const;
  const mdl::Selection& selection() const override;

  /**
   * For commands that modify brush faces, this returns all that should be acted on, based
   * on the current selection.
   *
   * - if brush faces are explicitly selected (hasSelectedBrushFaces()), use those
   * - selected groups implicitly act on any contained brushes
   * - selected brushes implicitly act on their faces
   *
   * Unlike allSelectedBrushNodes()/allSelectedEntityNodes(), if multiple groups in a link
   * set are selected, only return one representative face per brush, so that user actions
   * can be performed without generating conflicts. (e.g. this allows selecting 2 closed
   * linked groups in a link set and applying materials.)
   */
  std::vector<mdl::BrushFaceHandle> allSelectedBrushFaces() const override;
  std::vector<mdl::BrushFaceHandle> selectedBrushFaces() const override;

  VertexHandleManager& vertexHandles();
  EdgeHandleManager& edgeHandles();
  FaceHandleManager& faceHandles();

  const vm::bbox3d referenceBounds() const override;
  const std::optional<vm::bbox3d>& lastSelectionBounds() const override;
  const std::optional<vm::bbox3d>& selectionBounds() const override;
  const std::string& currentMaterialName() const override;
  void setCurrentMaterialName(const std::string& currentMaterialName);

  void selectAllNodes() override;
  void selectSiblings() override;
  void selectTouching(bool del) override;
  void selectInside(bool del) override;
  void selectInverse() override;
  void selectNodesWithFilePosition(const std::vector<size_t>& positions) override;
  void selectNodes(const std::vector<mdl::Node*>& nodes) override;
  void selectBrushFaces(const std::vector<mdl::BrushFaceHandle>& handles) override;
  void convertToFaceSelection() override;
  void selectFacesWithMaterial(const mdl::Material* material);
  void selectBrushesWithMaterial(const mdl::Material* material);
  void selectTall(vm::axis::type cameraAxis);

  void deselectAll() override;
  void deselectNodes(const std::vector<mdl::Node*>& nodes) override;
  void deselectBrushFaces(const std::vector<mdl::BrushFaceHandle>& handles) override;

protected:
  void updateLastSelectionBounds();
  void invalidateSelectionBounds();

private:
  void clearSelection();

public: // adding, removing, reparenting, and duplicating nodes, declared in MapFacade
        // interface
  std::vector<mdl::Node*> addNodes(
    const std::map<mdl::Node*, std::vector<mdl::Node*>>& nodes) override;
  void removeNodes(const std::vector<mdl::Node*>& nodes) override;

private:
  std::map<mdl::Node*, std::vector<mdl::Node*>> collectRemovableParents(
    const std::map<mdl::Node*, std::vector<mdl::Node*>>& nodes) const;

  struct CompareByAncestry;
  std::vector<mdl::Node*> removeImplicitelyRemovedNodes(
    std::vector<mdl::Node*> nodes) const;

  void closeRemovedGroups(const std::map<mdl::Node*, std::vector<mdl::Node*>>& toRemove);

public:
  bool reparentNodes(
    const std::map<mdl::Node*, std::vector<mdl::Node*>>& nodesToAdd) override;

private:
  bool checkReparenting(
    const std::map<mdl::Node*, std::vector<mdl::Node*>>& nodesToAdd) const;

public:
  void remove() override;
  void duplicate() override;

public: // entity management
  mdl::EntityNode* createPointEntity(
    const mdl::EntityDefinition& definition, const vm::vec3d& delta) override;
  mdl::EntityNode* createBrushEntity(const mdl::EntityDefinition& definition) override;

public: // group management
  mdl::GroupNode* groupSelection(const std::string& name);
  void mergeSelectedGroupsWithGroup(mdl::GroupNode* group);

public:
  void ungroupSelection();
  void renameGroups(const std::string& name);

  void openGroup(mdl::GroupNode* group);
  void closeGroup();

  /**
   * Creates a new group that is linked to the currently selected group and returns the
   * newly created group.
   *
   * If the current selection does not consist of exactly one group, then null is
   * returned.
   */
  mdl::GroupNode* createLinkedDuplicate();
  bool canCreateLinkedDuplicate() const;

  /**
   * Selects all groups linked to the currently selected groups.
   *
   * Nothing happens if the current selection does not consist of only groups.
   */
  void selectLinkedGroups();
  bool canSelectLinkedGroups() const;

  void linkGroups(const std::vector<mdl::GroupNode*>& groupNodes);
  void unlinkGroups(const std::vector<mdl::GroupNode*>& groupNodes);

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

  bool canUpdateLinkedGroups(const std::vector<mdl::Node*>& nodes) const;

protected:
  void setHasPendingChanges(
    const std::vector<mdl::GroupNode*>& groupNodes, bool hasPendingChanges);
  bool updateLinkedGroups();

private:
  void separateSelectedLinkedGroups(bool relinkGroups);

public: // layer management
  void renameLayer(mdl::LayerNode* layer, const std::string& name);

private:
  enum class MoveDirection
  {
    Up,
    Down
  };
  bool moveLayerByOne(mdl::LayerNode* layerNode, MoveDirection direction);

public:
  void moveLayer(mdl::LayerNode* layer, int offset);
  bool canMoveLayer(mdl::LayerNode* layer, int offset) const;
  void moveSelectionToLayer(mdl::LayerNode* layer);
  bool canMoveSelectionToLayer(mdl::LayerNode* layer) const;
  void hideLayers(const std::vector<mdl::LayerNode*>& layers);
  bool canHideLayers(const std::vector<mdl::LayerNode*>& layers) const;
  void isolateLayers(const std::vector<mdl::LayerNode*>& layers);
  bool canIsolateLayers(const std::vector<mdl::LayerNode*>& layers) const;
  void setOmitLayerFromExport(mdl::LayerNode* layerNode, bool omitFromExport);
  void selectAllInLayers(const std::vector<mdl::LayerNode*>& layers);
  bool canSelectAllInLayers(const std::vector<mdl::LayerNode*>& layers) const;

public: // modifying transient node attributes, declared in MapFacade interface
  void isolate();
  void hide(std::vector<mdl::Node*> nodes) override; // Don't take the nodes by reference!
  void hideSelection();
  void show(const std::vector<mdl::Node*>& nodes) override;
  void showAll();
  void ensureVisible(const std::vector<mdl::Node*>& nodes);
  void resetVisibility(const std::vector<mdl::Node*>& nodes) override;

  void lock(const std::vector<mdl::Node*>& nodes) override;
  void unlock(const std::vector<mdl::Node*>& nodes) override;
  void ensureUnlocked(const std::vector<mdl::Node*>& nodes);
  void resetLock(const std::vector<mdl::Node*>& nodes) override;

private:
  void downgradeShownToInherit(const std::vector<mdl::Node*>& nodes);
  void downgradeUnlockedToInherit(const std::vector<mdl::Node*>& nodes);

public: // modifying objects, declared in MapFacade interface
  bool swapNodeContents(
    const std::string& commandName,
    std::vector<std::pair<mdl::Node*, mdl::NodeContents>> nodesToSwap,
    std::vector<mdl::GroupNode*> changedLinkedGroups);
  bool swapNodeContents(
    const std::string& commandName,
    std::vector<std::pair<mdl::Node*, mdl::NodeContents>> nodesToSwap);
  bool transform(const std::string& commandName, const vm::mat4x4d& transformation);

  bool translate(const vm::vec3d& delta) override;
  bool rotate(const vm::vec3d& center, const vm::vec3d& axis, double angle) override;
  bool scale(const vm::bbox3d& oldBBox, const vm::bbox3d& newBBox) override;
  bool scale(const vm::vec3d& center, const vm::vec3d& scaleFactors) override;
  bool shear(
    const vm::bbox3d& box, const vm::vec3d& sideToShear, const vm::vec3d& delta) override;
  bool flip(const vm::vec3d& center, vm::axis::type axis) override;

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
    const std::string& key,
    const std::string& value,
    bool defaultToProtected = false) override;
  bool renameProperty(const std::string& oldKey, const std::string& newKey) override;
  bool removeProperty(const std::string& key) override;

  bool convertEntityColorRange(
    const std::string& key, mdl::ColorRange::Type range) override;
  bool updateSpawnflag(const std::string& key, size_t flagIndex, bool setFlag) override;

  bool setProtectedProperty(const std::string& key, bool value);
  bool clearProtectedProperties();
  bool canClearProtectedProperties() const;

  void setDefaultProperties(mdl::SetDefaultPropertyMode mode);

public: // brush resizing, declared in MapFacade interface
  bool extrudeBrushes(
    const std::vector<vm::polygon3d>& faces, const vm::vec3d& delta) override;

public:
  bool setFaceAttributes(const mdl::BrushFaceAttributes& attributes) override;
  bool setFaceAttributesExceptContentFlags(
    const mdl::BrushFaceAttributes& attributes) override;
  bool setFaceAttributes(const mdl::ChangeBrushFaceAttributesRequest& request) override;
  bool copyUVFromFace(
    const mdl::UVCoordSystemSnapshot& coordSystemSnapshot,
    const mdl::BrushFaceAttributes& attribs,
    const vm::plane3d& sourceFacePlane,
    mdl::WrapStyle wrapStyle);
  bool translateUV(
    const vm::vec3f& cameraUp,
    const vm::vec3f& cameraRight,
    const vm::vec2f& delta) override;
  bool rotateUV(float angle) override;
  bool shearUV(const vm::vec2f& factors) override;
  bool flipUV(
    const vm::vec3f& cameraUp,
    const vm::vec3f& cameraRight,
    vm::direction cameraRelativeFlipDirection);

public: // modifying vertices, declared in MapFacade interface
  bool snapVertices(double snapTo) override;

  TransformVerticesResult transformVertices(
    std::vector<vm::vec3d> vertexPositions, const vm::mat4x4d& transform) override;
  bool transformEdges(
    std::vector<vm::segment3d> edgePositions, const vm::mat4x4d& transform) override;
  bool transformFaces(
    std::vector<vm::polygon3d> facePositions, const vm::mat4x4d& transform) override;

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

  virtual bool isCurrentDocumentStateObservable() const = 0;

private:
  std::unique_ptr<CommandResult> execute(std::unique_ptr<Command>&& command);
  std::unique_ptr<CommandResult> executeAndStore(
    std::unique_ptr<UndoableCommand>&& command);

private: // subclassing interface for command processing
  virtual bool doCanUndoCommand() const = 0;
  virtual bool doCanRedoCommand() const = 0;
  virtual const std::string& doGetUndoCommandName() const = 0;
  virtual const std::string& doGetRedoCommandName() const = 0;
  virtual void doUndoCommand() = 0;
  virtual void doRedoCommand() = 0;

  virtual void doClearCommandProcessor() = 0;
  virtual void doStartTransaction(std::string name, TransactionScope scope) = 0;
  virtual void doCommitTransaction() = 0;
  virtual void doRollbackTransaction() = 0;

  virtual std::unique_ptr<CommandResult> doExecute(std::unique_ptr<Command> command) = 0;
  virtual std::unique_ptr<CommandResult> doExecuteAndStore(
    std::unique_ptr<UndoableCommand> command) = 0;

public: // asset state management
  void processResourcesSync(const mdl::ProcessContext& processContext);
  void processResourcesAsync(const mdl::ProcessContext& processContext);
  bool needsResourceProcessing();

public: // picking
  void pick(const vm::ray3d& pickRay, mdl::PickResult& pickResult) const;
  std::vector<mdl::Node*> findNodesContaining(const vm::vec3d& point) const;

private: // world management
  void setWorld(
    const vm::bbox3d& worldBounds,
    std::unique_ptr<mdl::WorldNode> worldNode,
    std::shared_ptr<mdl::Game> game,
    const std::filesystem::path& path);
  void clearWorld();

public: // asset management
  mdl::EntityDefinitionFileSpec entityDefinitionFile() const;
  std::vector<mdl::EntityDefinitionFileSpec> allEntityDefinitionFiles() const;
  void setEntityDefinitionFile(const mdl::EntityDefinitionFileSpec& spec);

  // For testing
  void setEntityDefinitions(std::vector<mdl::EntityDefinition> definitions);

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
  void setMaterials(const std::vector<mdl::Node*>& nodes);
  void setMaterials(const std::vector<mdl::BrushFaceHandle>& faceHandles);
  void unsetMaterials();
  void unsetMaterials(const std::vector<mdl::Node*>& nodes);

  void setEntityDefinitions();
  void setEntityDefinitions(const std::vector<mdl::Node*>& nodes);
  void unsetEntityDefinitions();
  void unsetEntityDefinitions(const std::vector<mdl::Node*>& nodes);
  void reloadEntityDefinitionsInternal();

  void clearEntityModels();

  void setEntityModels();
  void setEntityModels(const std::vector<mdl::Node*>& nodes);
  void unsetEntityModels();
  void unsetEntityModels(const std::vector<mdl::Node*>& nodes);

protected: // search paths and mods
  std::vector<std::filesystem::path> externalSearchPaths() const;
  void updateGameSearchPaths();

public:
  std::vector<std::string> mods() const override;
  void setMods(const std::vector<std::string>& mods) override;
  std::string defaultMod() const;

public: // map soft bounds
  void setSoftMapBounds(const mdl::Game::SoftMapBounds& bounds);
  mdl::Game::SoftMapBounds softMapBounds() const;

private: // validator management
  void registerValidators();

public:
  void setIssueHidden(const mdl::Issue& issue, bool hidden);

private:
  virtual void doSetIssueHidden(const mdl::Issue& issue, bool hidden) = 0;

public:                     // tag management
  void registerSmartTags(); // public for testing
  const std::vector<mdl::SmartTag>& smartTags() const;
  bool isRegisteredSmartTag(const std::string& name) const;
  const mdl::SmartTag& smartTag(const std::string& name) const;
  bool isRegisteredSmartTag(size_t index) const;
  const mdl::SmartTag& smartTag(size_t index) const;

private:
  void initializeAllNodeTags(MapDocument* document);
  void initializeNodeTags(const std::vector<mdl::Node*>& nodes);
  void clearNodeTags(const std::vector<mdl::Node*>& nodes);
  void updateNodeTags(const std::vector<mdl::Node*>& nodes);

  void updateFaceTags(const std::vector<mdl::BrushFaceHandle>& faces);
  void updateAllFaceTags();

  void updateFaceTagsAfterResourcesWhereProcessed(
    const std::vector<mdl::ResourceId>& resourceIds);

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

} // namespace tb::ui
