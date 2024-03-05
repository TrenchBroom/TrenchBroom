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

#pragma once

#include "FloatType.h"
#include "Model/Game.h"
#include "Model/MapFacade.h"
#include "Model/NodeCollection.h"
#include "Model/NodeContents.h"
#include "Model/PointTrace.h"
#include "Model/PortalFile.h"
#include "Notifier.h"
#include "NotifierConnection.h"
#include "Result.h"
#include "View/CachingLogger.h"

#include "vm/bbox.h"
#include "vm/forward.h"
#include "vm/util.h"

#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace TrenchBroom
{
class Color;
} // namespace TrenchBroom

namespace TrenchBroom::Assets
{
class EntityDefinition;
class EntityDefinitionFileSpec;
class EntityDefinitionManager;
class EntityModelManager;
class Texture;
class TextureManager;
} // namespace TrenchBroom::Assets

namespace TrenchBroom::Model
{
class Brush;
class BrushFace;
class BrushFaceHandle;
class BrushFaceAttributes;
class EditorContext;
class Entity;
class Game;
class Issue;
enum class MapFormat;
class PickResult;
class PointTrace;
class PortalFile;
class SmartTag;
class TagManager;
class TexCoordSystemSnapshot;
class WorldNode;
enum class WrapStyle;
} // namespace TrenchBroom::Model

namespace TrenchBroom::View
{
class Action;
class Command;
class CommandResult;
class Grid;
enum class PasteType;
class RepeatStack;
class Selection;
class UndoableCommand;
class ViewEffectsService;
enum class MapTextEncoding;
enum class TransactionScope;

struct PointFile
{
  Model::PointTrace trace;
  std::filesystem::path path;
};

struct PortalFile
{
  Model::PortalFile portalFile;
  std::filesystem::path path;
};

class MapDocument : public Model::MapFacade, public CachingLogger
{
public:
  static const vm::bbox3 DefaultWorldBounds;
  static const std::string DefaultDocumentName;

protected:
  vm::bbox3 m_worldBounds;
  std::shared_ptr<Model::Game> m_game;
  std::unique_ptr<Model::WorldNode> m_world;

  std::optional<PointFile> m_pointFile;
  std::optional<PortalFile> m_portalFile;

  std::unique_ptr<Assets::EntityDefinitionManager> m_entityDefinitionManager;
  std::unique_ptr<Assets::EntityModelManager> m_entityModelManager;
  std::unique_ptr<Assets::TextureManager> m_textureManager;
  std::unique_ptr<Model::TagManager> m_tagManager;

  std::unique_ptr<Model::EditorContext> m_editorContext;
  std::unique_ptr<Grid> m_grid;

  using ActionList = std::vector<std::unique_ptr<Action>>;
  ActionList m_tagActions;
  ActionList m_entityDefinitionActions;

  std::filesystem::path m_path;
  size_t m_lastSaveModificationCount;
  size_t m_modificationCount;

  Model::NodeCollection m_selectedNodes;
  std::vector<Model::BrushFaceHandle> m_selectedBrushFaces;

  Model::LayerNode* m_currentLayer;
  std::string m_currentTextureName;
  vm::bbox3 m_lastSelectionBounds;
  mutable vm::bbox3 m_selectionBounds;
  mutable bool m_selectionBoundsValid;

  ViewEffectsService* m_viewEffectsService;

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
  Notifier<const Model::LayerNode*> currentLayerDidChangeNotifier;
  Notifier<const std::string&> currentTextureNameDidChangeNotifier;

  Notifier<> selectionWillChangeNotifier;
  Notifier<const Selection&> selectionDidChangeNotifier;

  Notifier<const std::vector<Model::Node*>&> nodesWereAddedNotifier;
  Notifier<const std::vector<Model::Node*>&> nodesWillBeRemovedNotifier;
  Notifier<const std::vector<Model::Node*>&> nodesWereRemovedNotifier;
  Notifier<const std::vector<Model::Node*>&> nodesWillChangeNotifier;
  Notifier<const std::vector<Model::Node*>&> nodesDidChangeNotifier;

  Notifier<const std::vector<Model::Node*>&> nodeVisibilityDidChangeNotifier;
  Notifier<const std::vector<Model::Node*>&> nodeLockingDidChangeNotifier;

  Notifier<Model::GroupNode*> groupWasOpenedNotifier;
  Notifier<Model::GroupNode*> groupWasClosedNotifier;

  Notifier<const std::vector<Model::BrushFaceHandle>&> brushFacesDidChangeNotifier;

  Notifier<> textureCollectionsWillChangeNotifier;
  Notifier<> textureCollectionsDidChangeNotifier;

  Notifier<> textureUsageCountsDidChangeNotifier;

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
  MapDocument();

public:
  ~MapDocument() override;

public: // accessors and such
  Logger& logger();

  std::shared_ptr<Model::Game> game() const override;
  const vm::bbox3& worldBounds() const;
  Model::WorldNode* world() const;

  bool isGamePathPreference(const std::filesystem::path& path) const;

  Model::LayerNode* currentLayer() const override;

protected:
  Model::LayerNode* performSetCurrentLayer(Model::LayerNode* currentLayer);

public:
  void setCurrentLayer(Model::LayerNode* currentLayer);
  bool canSetCurrentLayer(Model::LayerNode* currentLayer) const;

  Model::GroupNode* currentGroup() const override;
  /**
   * Returns the current group if one is open, otherwise the world.
   */
  Model::Node* currentGroupOrWorld() const override;
  /**
   * Suggests a parent to use for new nodes.
   *
   * If reference nodes are given, return the parent (either a group, if there is one,
   * otherwise the layer) of the first node in the given vector.
   *
   * Otherwise, returns the current group if one is open, otherwise the current layer.
   */
  Model::Node* parentForNodes(
    const std::vector<Model::Node*>& referenceNodes =
      std::vector<Model::Node*>()) const override;

  Model::EditorContext& editorContext() const;

  Assets::EntityDefinitionManager& entityDefinitionManager() override;
  Assets::EntityModelManager& entityModelManager() override;
  Assets::TextureManager& textureManager() override;

  Grid& grid() const;

  Model::PointTrace* pointFile();
  const Model::PortalFile* portalFile() const;

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
    for (const std::unique_ptr<Action>& action : actions)
    {
      visitor(*action);
    }
  }

  void createTagActions();
  void clearTagActions();
  void createEntityDefinitionActions();

public: // new, load, save document
  Result<void> newDocument(
    Model::MapFormat mapFormat,
    const vm::bbox3& worldBounds,
    std::shared_ptr<Model::Game> game);
  Result<void> loadDocument(
    Model::MapFormat mapFormat,
    const vm::bbox3& worldBounds,
    std::shared_ptr<Model::Game> game,
    const std::filesystem::path& path);
  void saveDocument();
  void saveDocumentAs(const std::filesystem::path& path);
  void saveDocumentTo(const std::filesystem::path& path);
  Result<void> exportDocumentAs(const IO::ExportOptions& options);

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
  bool pasteNodes(const std::vector<Model::Node*>& nodes);
  bool pasteBrushFaces(const std::vector<Model::BrushFace>& faces);

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
  std::vector<Model::EntityNodeBase*> allSelectedEntityNodes() const override;

  /**
   * For commands that modify brushes, this returns all brushes that should be acted on,
   * based on the current selection.
   *
   * - selected groups implicitly act on any contained brushes
   *
   * If multiple linked groups are selected, returns brushes from all of them, so
   * attempting to perform commands on all of them will be blocked as a conflict.
   */
  std::vector<Model::BrushNode*> allSelectedBrushNodes() const;
  bool hasAnySelectedBrushNodes() const;
  const Model::NodeCollection& selectedNodes() const override;

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
   * linked groups in a link set and applying textures.)
   */
  std::vector<Model::BrushFaceHandle> allSelectedBrushFaces() const override;
  std::vector<Model::BrushFaceHandle> selectedBrushFaces() const override;

  const vm::bbox3& referenceBounds() const override;
  const vm::bbox3& lastSelectionBounds() const override;
  const vm::bbox3& selectionBounds() const override;
  const std::string& currentTextureName() const override;
  void setCurrentTextureName(const std::string& currentTextureName);

  void selectAllNodes() override;
  void selectSiblings() override;
  void selectEntitiesWithSameClassname() override;
  void selectTouching(bool del) override;
  void selectInside(bool del) override;
  void selectInverse() override;
  void selectNodesWithFilePosition(const std::vector<size_t>& positions) override;
  void selectNodes(const std::vector<Model::Node*>& nodes) override;
  void selectBrushFaces(const std::vector<Model::BrushFaceHandle>& handles) override;
  void convertToFaceSelection() override;
  void selectFacesWithTexture(const Assets::Texture* texture);
  void selectTall(vm::axis::type cameraAxis);

  void deselectAll() override;
  void deselectNodes(const std::vector<Model::Node*>& nodes) override;
  void deselectBrushFaces(const std::vector<Model::BrushFaceHandle>& handles) override;

protected:
  void updateLastSelectionBounds();
  void invalidateSelectionBounds();

private:
  void validateSelectionBounds() const;
  void clearSelection();

public: // adding, removing, reparenting, and duplicating nodes, declared in MapFacade
        // interface
  std::vector<Model::Node*> addNodes(
    const std::map<Model::Node*, std::vector<Model::Node*>>& nodes) override;
  void removeNodes(const std::vector<Model::Node*>& nodes) override;

private:
  std::map<Model::Node*, std::vector<Model::Node*>> collectRemovableParents(
    const std::map<Model::Node*, std::vector<Model::Node*>>& nodes) const;

  struct CompareByAncestry;
  std::vector<Model::Node*> removeImplicitelyRemovedNodes(
    std::vector<Model::Node*> nodes) const;

  void closeRemovedGroups(
    const std::map<Model::Node*, std::vector<Model::Node*>>& toRemove);

public:
  bool reparentNodes(
    const std::map<Model::Node*, std::vector<Model::Node*>>& nodesToAdd) override;

private:
  bool checkReparenting(
    const std::map<Model::Node*, std::vector<Model::Node*>>& nodesToAdd) const;

public:
  void deleteObjects() override;
  void duplicateObjects() override;

public: // entity management
  Model::EntityNode* createPointEntity(
    const Assets::PointEntityDefinition* definition, const vm::vec3& delta) override;
  Model::EntityNode* createBrushEntity(
    const Assets::BrushEntityDefinition* definition) override;

public: // group management
  Model::GroupNode* groupSelection(const std::string& name);
  void mergeSelectedGroupsWithGroup(Model::GroupNode* group);

public:
  void ungroupSelection();
  void renameGroups(const std::string& name);

  void openGroup(Model::GroupNode* group);
  void closeGroup();

  /**
   * Creates a new group that is linked to the currently selected group and returns the
   * newly created group.
   *
   * If the current selection does not consist of exactly one group, then null is
   * returned.
   */
  Model::GroupNode* createLinkedDuplicate();
  bool canCreateLinkedDuplicate() const;

  /**
   * Selects all groups linked to the currently selected groups.
   *
   * Nothing happens if the current selection does not consist of only groups.
   */
  void selectLinkedGroups();
  bool canSelectLinkedGroups() const;

  void linkGroups(const std::vector<Model::GroupNode*>& groupNodes);
  void unlinkGroups(const std::vector<Model::GroupNode*>& groupNodes);

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

  bool canUpdateLinkedGroups(const std::vector<Model::Node*>& nodes) const;

protected:
  void setHasPendingChanges(
    const std::vector<Model::GroupNode*>& groupNodes, bool hasPendingChanges);
  bool updateLinkedGroups();

private:
  void separateSelectedLinkedGroups(bool relinkGroups);

public: // layer management
  void renameLayer(Model::LayerNode* layer, const std::string& name);

private:
  enum class MoveDirection
  {
    Up,
    Down
  };
  bool moveLayerByOne(Model::LayerNode* layerNode, MoveDirection direction);

public:
  void moveLayer(Model::LayerNode* layer, int offset);
  bool canMoveLayer(Model::LayerNode* layer, int offset) const;
  void moveSelectionToLayer(Model::LayerNode* layer);
  bool canMoveSelectionToLayer(Model::LayerNode* layer) const;
  void hideLayers(const std::vector<Model::LayerNode*>& layers);
  bool canHideLayers(const std::vector<Model::LayerNode*>& layers) const;
  void isolateLayers(const std::vector<Model::LayerNode*>& layers);
  bool canIsolateLayers(const std::vector<Model::LayerNode*>& layers) const;
  void setOmitLayerFromExport(Model::LayerNode* layerNode, bool omitFromExport);
  void selectAllInLayers(const std::vector<Model::LayerNode*>& layers);
  bool canSelectAllInLayers(const std::vector<Model::LayerNode*>& layers) const;

public: // modifying transient node attributes, declared in MapFacade interface
  void isolate();
  void hide(
    std::vector<Model::Node*> nodes) override; // Don't take the nodes by reference!
  void hideSelection();
  void show(const std::vector<Model::Node*>& nodes) override;
  void showAll();
  void ensureVisible(const std::vector<Model::Node*>& nodes);
  void resetVisibility(const std::vector<Model::Node*>& nodes) override;

  void lock(const std::vector<Model::Node*>& nodes) override;
  void unlock(const std::vector<Model::Node*>& nodes) override;
  void ensureUnlocked(const std::vector<Model::Node*>& nodes);
  void resetLock(const std::vector<Model::Node*>& nodes) override;

private:
  void downgradeShownToInherit(const std::vector<Model::Node*>& nodes);
  void downgradeUnlockedToInherit(const std::vector<Model::Node*>& nodes);

public: // modifying objects, declared in MapFacade interface
  bool swapNodeContents(
    const std::string& commandName,
    std::vector<std::pair<Model::Node*, Model::NodeContents>> nodesToSwap,
    std::vector<Model::GroupNode*> changedLinkedGroups);
  bool swapNodeContents(
    const std::string& commandName,
    std::vector<std::pair<Model::Node*, Model::NodeContents>> nodesToSwap);
  bool transformObjects(const std::string& commandName, const vm::mat4x4& transformation);

  bool translateObjects(const vm::vec3& delta) override;
  bool rotateObjects(
    const vm::vec3& center, const vm::vec3& axis, FloatType angle) override;
  bool scaleObjects(const vm::bbox3& oldBBox, const vm::bbox3& newBBox) override;
  bool scaleObjects(const vm::vec3& center, const vm::vec3& scaleFactors) override;
  bool shearObjects(
    const vm::bbox3& box, const vm::vec3& sideToShear, const vm::vec3& delta) override;
  bool flipObjects(const vm::vec3& center, vm::axis::type axis) override;

public: // CSG operations, declared in MapFacade interface
  bool createBrush(const std::vector<vm::vec3>& points);
  bool csgConvexMerge();
  bool csgSubtract();
  bool csgIntersect();
  bool csgHollow();

public: // Clipping operations, declared in MapFacade interface
  bool clipBrushes(const vm::vec3& p1, const vm::vec3& p2, const vm::vec3& p3);

public: // modifying entity properties, declared in MapFacade interface
  bool setProperty(
    const std::string& key,
    const std::string& value,
    bool defaultToProtected = false) override;
  bool renameProperty(const std::string& oldKey, const std::string& newKey) override;
  bool removeProperty(const std::string& key) override;

  bool convertEntityColorRange(
    const std::string& key, Assets::ColorRange::Type range) override;
  bool updateSpawnflag(const std::string& key, size_t flagIndex, bool setFlag) override;

  bool setProtectedProperty(const std::string& key, bool value);
  bool clearProtectedProperties();
  bool canClearProtectedProperties() const;

  void setDefaultProperties(Model::SetDefaultPropertyMode mode);

public: // brush resizing, declared in MapFacade interface
  bool extrudeBrushes(
    const std::vector<vm::polygon3>& faces, const vm::vec3& delta) override;

public:
  bool setFaceAttributes(const Model::BrushFaceAttributes& attributes) override;
  bool setFaceAttributesExceptContentFlags(
    const Model::BrushFaceAttributes& attributes) override;
  bool setFaceAttributes(const Model::ChangeBrushFaceAttributesRequest& request) override;
  bool copyTexCoordSystemFromFace(
    const Model::TexCoordSystemSnapshot& coordSystemSnapshot,
    const Model::BrushFaceAttributes& attribs,
    const vm::plane3& sourceFacePlane,
    Model::WrapStyle wrapStyle);
  bool moveTextures(
    const vm::vec3f& cameraUp,
    const vm::vec3f& cameraRight,
    const vm::vec2f& delta) override;
  bool rotateTextures(float angle) override;
  bool shearTextures(const vm::vec2f& factors) override;
  bool flipTextures(
    const vm::vec3f& cameraUp,
    const vm::vec3f& cameraRight,
    vm::direction cameraRelativeFlipDirection);

public: // modifying vertices, declared in MapFacade interface
  bool snapVertices(FloatType snapTo) override;

  MoveVerticesResult moveVertices(
    std::vector<vm::vec3> vertexPositions, const vm::vec3& delta) override;
  bool moveEdges(std::vector<vm::segment3> edgePositions, const vm::vec3& delta) override;
  bool moveFaces(std::vector<vm::polygon3> facePositions, const vm::vec3& delta) override;

  bool addVertex(const vm::vec3& vertexPosition);
  bool removeVertices(
    const std::string& commandName, std::vector<vm::vec3> vertexPositions);

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
  void commitPendingAssets();

public: // picking
  void pick(const vm::ray3& pickRay, Model::PickResult& pickResult) const;
  std::vector<Model::Node*> findNodesContaining(const vm::vec3& point) const;

private: // world management
  Result<void> createWorld(
    Model::MapFormat mapFormat,
    const vm::bbox3& worldBounds,
    std::shared_ptr<Model::Game> game);
  Result<void> loadWorld(
    Model::MapFormat mapFormat,
    const vm::bbox3& worldBounds,
    std::shared_ptr<Model::Game> game,
    const std::filesystem::path& path);
  void clearWorld();

public: // asset management
  Assets::EntityDefinitionFileSpec entityDefinitionFile() const;
  std::vector<Assets::EntityDefinitionFileSpec> allEntityDefinitionFiles() const;
  void setEntityDefinitionFile(const Assets::EntityDefinitionFileSpec& spec);

  // For testing
  void setEntityDefinitions(
    std::vector<std::unique_ptr<Assets::EntityDefinition>> definitions);

  void reloadTextureCollections();
  void reloadEntityDefinitions();

  std::vector<std::filesystem::path> enabledTextureCollections() const;
  std::vector<std::filesystem::path> disabledTextureCollections() const;

  void setEnabledTextureCollections(
    const std::vector<std::filesystem::path>& enabledTextureCollections);

private:
  void loadAssets();
  void unloadAssets();

  void loadEntityDefinitions();
  void unloadEntityDefinitions();

  void loadEntityModels();
  void unloadEntityModels();

protected:
  void reloadTextures();
  void loadTextures();
  void unloadTextures();

  void setTextures();
  void setTextures(const std::vector<Model::Node*>& nodes);
  void setTextures(const std::vector<Model::BrushFaceHandle>& faceHandles);
  void unsetTextures();
  void unsetTextures(const std::vector<Model::Node*>& nodes);

  void setEntityDefinitions();
  void setEntityDefinitions(const std::vector<Model::Node*>& nodes);
  void unsetEntityDefinitions();
  void unsetEntityDefinitions(const std::vector<Model::Node*>& nodes);
  void reloadEntityDefinitionsInternal();

  void clearEntityModels();

  void setEntityModels();
  void setEntityModels(const std::vector<Model::Node*>& nodes);
  void unsetEntityModels();
  void unsetEntityModels(const std::vector<Model::Node*>& nodes);

protected: // search paths and mods
  std::vector<std::filesystem::path> externalSearchPaths() const;
  void updateGameSearchPaths();

public:
  std::vector<std::string> mods() const override;
  void setMods(const std::vector<std::string>& mods) override;
  std::string defaultMod() const;

public: // map soft bounds
  void setSoftMapBounds(const Model::Game::SoftMapBounds& bounds);
  Model::Game::SoftMapBounds softMapBounds() const;

private: // validator management
  void registerValidators();

public:
  void setIssueHidden(const Model::Issue& issue, bool hidden);

private:
  virtual void doSetIssueHidden(const Model::Issue& issue, bool hidden) = 0;

public:                     // tag management
  void registerSmartTags(); // public for testing
  const std::vector<Model::SmartTag>& smartTags() const;
  bool isRegisteredSmartTag(const std::string& name) const;
  const Model::SmartTag& smartTag(const std::string& name) const;
  bool isRegisteredSmartTag(size_t index) const;
  const Model::SmartTag& smartTag(size_t index) const;

private:
  void initializeAllNodeTags(MapDocument* document);
  void initializeNodeTags(const std::vector<Model::Node*>& nodes);
  void clearNodeTags(const std::vector<Model::Node*>& nodes);
  void updateNodeTags(const std::vector<Model::Node*>& nodes);

  void updateFaceTags(const std::vector<Model::BrushFaceHandle>& faces);
  void updateAllFaceTags();

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
  void textureCollectionsWillChange();
  void textureCollectionsDidChange();
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

class Transaction
{
public:
  enum class State
  {
    Running,
    Committed,
    Cancelled,
  };

private:
  MapDocument& m_document;
  std::string m_name;
  State m_state;

public:
  explicit Transaction(std::weak_ptr<MapDocument> document, std::string name = "");
  explicit Transaction(std::shared_ptr<MapDocument> document, std::string name = "");
  explicit Transaction(MapDocument& document, std::string name = "");
  ~Transaction();

  State state() const;

  void finish(bool commit);
  bool commit();
  void rollback();
  void cancel();

private:
  void begin();
};
} // namespace TrenchBroom::View
