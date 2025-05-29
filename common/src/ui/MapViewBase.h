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

#include "NotifierConnection.h"
#include "ui/ActionContext.h"
#include "ui/CameraLinkHelper.h"
#include "ui/MapView.h"
#include "ui/RenderView.h"
#include "ui/ToolBoxConnector.h"

#include <filesystem>
#include <memory>
#include <utility>
#include <vector>

class QMenu;
class QShortcut;
class QString;
class QAction;

namespace tb::mdl
{
struct EntityDefinition;
class GroupNode;
class Node;
class NodeCollection;
class SmartTag;
enum class EntityDefinitionType;
} // namespace tb::mdl

namespace tb::render
{
class Camera;
class Compass;
class MapRenderer;
class PrimitiveRenderer;
class RenderBatch;
class RenderContext;
enum class RenderMode;
} // namespace tb::render

namespace tb::ui
{
class Action;
class AnimationManager;
class Command;
class MapDocument;
class MapViewToolBox;
struct SelectionChange;
class SignalDelayer;
class Tool;
class UndoableCommand;

class MapViewBase : public RenderView,
                    public MapView,
                    public ToolBoxConnector,
                    public CameraLinkableView
{
  Q_OBJECT
public:
  static const int DefaultCameraAnimationDuration;

protected:
  std::weak_ptr<MapDocument> m_document;
  MapViewToolBox& m_toolBox;
  render::MapRenderer& m_renderer;

  std::unique_ptr<AnimationManager> m_animationManager;

private:
  std::unique_ptr<render::Compass> m_compass;
  std::unique_ptr<render::PrimitiveRenderer> m_portalFileRenderer;

  /**
   * Tracks whether this map view has most recently gotten the focus. This is tracked and
   * updated by a MapViewActivationTracker instance.
   */
  bool m_isCurrent = false;

  SignalDelayer* m_updateActionStatesSignalDelayer = nullptr;

  NotifierConnection m_notifierConnection;

private: // shortcuts
  std::vector<std::pair<QShortcut*, const Action*>> m_shortcuts;

protected:
  MapViewBase(
    std::weak_ptr<MapDocument> document,
    MapViewToolBox& toolBox,
    render::MapRenderer& renderer,
    GLContextManager& contextManager);

  void setCompass(std::unique_ptr<render::Compass> compass);

  /**
   * Perform tasks that are needed for a fully initialized MapViewBase.
   *
   * This must be called exactly once, at the end of subclasses's constructors.
   * (Does virtual function calls, so we can't call it in the MapViewBase constructor.)
   *
   * On normal app startup, these tasks are handled by documentDidChange(),
   * but when changing map view layouts (e.g. 1 pane to 2 pane) there are
   * no document notifications to handle these tasks, so it must be done by the
   * constructor.
   */
  void mapViewBaseVirtualInit();

public:
  ~MapViewBase() override;

public:
  void setIsCurrent(bool isCurrent);

  virtual render::Camera& camera() = 0;

private:
  void bindEvents();
  void connectObservers();

  void createActionsAndUpdatePicking();

  void nodesDidChange(const std::vector<mdl::Node*>& nodes);
  void toolChanged(Tool& tool);
  void commandDone(Command& command);
  void commandUndone(UndoableCommand& command);
  void selectionDidChange(const SelectionChange& selectionChange);
  void materialCollectionsDidChange();
  void entityDefinitionsDidChange();
  void modsDidChange();
  void editorContextDidChange();
  void gridDidChange();
  void pointFileDidChange();
  void portalFileDidChange();
  void preferenceDidChange(const std::filesystem::path& path);
  void documentDidChange(MapDocument* document);

private: // shortcut setup
  void createActions();
  void updateActionBindings();
  void updateActionStates();
  void updateActionStatesDelayed();

public:
  void triggerAction(const Action& action);
  void triggerAmbiguousAction(const QString& label);

public: // move, rotate, flip actions
  void move(vm::direction direction);
  void moveVertices(vm::direction direction);
  void moveRotationCenter(vm::direction direction);
  void moveObjects(vm::direction direction);
  virtual vm::vec3d moveDirection(vm::direction direction) const = 0;

  void duplicateAndMoveObjects(vm::direction direction);
  void duplicateObjects();

  void rotate(vm::rotation_axis axis, bool clockwise);
  vm::vec3d rotationAxis(vm::rotation_axis axis, bool clockwise) const;

  void flip(vm::direction direction);
  bool canFlip() const;
  virtual size_t flipAxis(vm::direction direction) const = 0;

public: // UV actions
  enum class UVActionMode
  {
    Normal,
    Coarse,
    Fine
  };

  void moveUV(vm::direction direction, UVActionMode mode);
  vm::vec2f moveUVOffset(vm::direction direction, UVActionMode mode) const;
  float moveUVDistance(UVActionMode mode) const;

  void rotateUV(bool clockwise, UVActionMode mode);
  float rotateUVAngle(bool clockwise, UVActionMode mode) const;

  void flipUV(vm::direction direction);
  void resetUV();
  void resetUVToWorld();

public: // tool mode actions
  void assembleBrush();

  void toggleClipSide();
  void performClip();

public: // misc actions
  void resetCameraZoom();
  void cancel();
  void deactivateCurrentTool();

public: // reparenting objects
  void addSelectedObjectsToGroup();
  void removeSelectedObjectsFromGroup();
  mdl::Node* findNewGroupForObjects(const std::vector<mdl::Node*>& nodes) const;

  void mergeSelectedGroups();
  mdl::GroupNode* findGroupToMergeGroupsInto(
    const mdl::NodeCollection& selectedNodes) const;

  /**
   * Checks whether the given node can be reparented under the given new parent.
   *
   * @param node the node to reparent
   * @param newParent the new parent node
   * @return true if the given node can be reparented under the given new parent, and
   * false otherwise
   */
  bool canReparentNode(const mdl::Node* node, const mdl::Node* newParent) const;

  void moveSelectedBrushesToEntity();
  mdl::Node* findNewParentEntityForBrushes(const std::vector<mdl::Node*>& nodes) const;

  bool canReparentNodes(
    const std::vector<mdl::Node*>& nodes, const mdl::Node* newParent) const;
  /**
   * Reparents nodes, and deselects everything as a side effect.
   *
   * @param nodes the nodes to reparent
   * @param newParent the new parent
   * @param preserveEntities if true, if `nodes` contains brushes belonging to an entity,
   * the whole entity and all brushes it contains are also reparented. if false, only the
   * brushes listed in `nodes` are reparented, not any parent entities.
   */
  void reparentNodes(
    const std::vector<mdl::Node*>& nodes, mdl::Node* newParent, bool preserveEntities);
  std::vector<mdl::Node*> collectReparentableNodes(
    const std::vector<mdl::Node*>& nodes, const mdl::Node* newParent) const;

  void createPointEntity();
  void createBrushEntity();
  virtual vm::vec3d computePointEntityPosition(const vm::bbox3d& bounds) const = 0;

  const mdl::EntityDefinition* findEntityDefinition(
    mdl::EntityDefinitionType type, size_t index) const;
  void createPointEntity(const mdl::EntityDefinition& definition);
  void createBrushEntity(const mdl::EntityDefinition& definition);
  bool canCreateBrushEntity();

public: // tags
  void toggleTagVisible(const mdl::SmartTag& tag);
  void enableTag(const mdl::SmartTag& tag);
  void disableTag(const mdl::SmartTag& tag);

public: // make structural
  void makeStructural();

public: // entity definitions
  void toggleEntityDefinitionVisible(const mdl::EntityDefinition& definition);
  void createEntity(const mdl::EntityDefinition& definition);

public: // view filters
  void toggleShowEntityClassnames();
  void toggleShowGroupBounds();
  void toggleShowBrushEntityBounds();
  void toggleShowPointEntityBounds();
  void toggleShowPointEntities();
  void toggleShowPointEntityModels();
  void toggleShowBrushes();
  void showMaterials();
  void hideMaterials();
  void hideFaces();
  void toggleShadeFaces();
  void toggleShowFog();
  void toggleShowEdges();
  void showAllEntityLinks();
  void showTransitivelySelectedEntityLinks();
  void showDirectlySelectedEntityLinks();
  void hideAllEntityLinks();

  bool event(QEvent* event) override;
  void focusInEvent(QFocusEvent* event) override;
  void focusOutEvent(QFocusEvent* event) override;

public:
  ActionContext::Type actionContext() const;
  virtual ActionContext::Type viewActionContext() const = 0;

public: // implement ViewEffectsService interface
  void flashSelection() override;

public: // implement MapView interface
  void installActivationTracker(MapViewActivationTracker& activationTracker) override;
  bool isCurrent() const override;
  MapViewBase* firstMapViewBase() override;
  bool cancelMouseDrag() override;
  void refreshViews() override;

protected: // RenderView overrides
  void initializeGL() override;

private: // implement RenderView interface
  bool shouldRenderFocusIndicator() const override;
  void renderContents() override;

  virtual void preRender();
  virtual render::RenderMode renderMode() = 0;

  virtual void renderGrid(
    render::RenderContext& renderContext, render::RenderBatch& renderBatch);
  virtual void renderMap(
    render::MapRenderer& renderer,
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch) = 0;
  virtual void renderTools(
    MapViewToolBox& toolBox,
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch) = 0;

  void setupGL(render::RenderContext& renderContext);
  void renderCoordinateSystem(
    render::RenderContext& renderContext, render::RenderBatch& renderBatch);
  virtual void renderSoftWorldBounds(
    render::RenderContext& renderContext, render::RenderBatch& renderBatch);
  void renderPointFile(
    render::RenderContext& renderContext, render::RenderBatch& renderBatch);

  void renderPortalFile(
    render::RenderContext& renderContext, render::RenderBatch& renderBatch);
  void invalidatePortalFileRenderer();
  void validatePortalFileRenderer(render::RenderContext& renderContext);

  void renderCompass(render::RenderBatch& renderBatch);
  void renderFPS(render::RenderContext& renderContext, render::RenderBatch& renderBatch);

public: // implement InputEventProcessor interface
  void processEvent(const KeyEvent& event) override;
  void processEvent(const MouseEvent& event) override;
  void processEvent(const ScrollEvent& event) override;
  void processEvent(const GestureEvent& event) override;
  void processEvent(const CancelEvent& event) override;

private: // implement ToolBoxConnector
  void doShowPopupMenu() override;
public slots:
  void showPopupMenuLater();

private:
  virtual void beforePopupMenu();

protected: // QWidget overrides
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dragLeaveEvent(QDragLeaveEvent* event) override;
  void dragMoveEvent(QDragMoveEvent* event) override;
  void dropEvent(QDropEvent* event) override;

private:
  QMenu* makeEntityGroupsMenu(mdl::EntityDefinitionType type);

  bool canMergeGroups() const;
  bool canMakeStructural() const;
};

} // namespace tb::ui
