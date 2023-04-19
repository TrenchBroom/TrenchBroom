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

#include "NotifierConnection.h"
#include "View/ActionContext.h"
#include "View/CameraLinkHelper.h"
#include "View/MapView.h"
#include "View/RenderView.h"
#include "View/ToolBoxConnector.h"

#include <memory>
#include <utility>
#include <vector>

class QMenu;
class QShortcut;
class QString;
class QAction;

namespace TrenchBroom
{
class Logger;

namespace Assets
{
class BrushEntityDefinition;
class EntityDefinition;
enum class EntityDefinitionType;
class PointEntityDefinition;
} // namespace Assets

namespace IO
{
class Path;
}

namespace Model
{
class GroupNode;
class Node;
class NodeCollection;
class SmartTag;
} // namespace Model

namespace Renderer
{
class Camera;
class Compass;
class MapRenderer;
class PrimitiveRenderer;
class RenderBatch;
class RenderContext;
enum class RenderMode;
} // namespace Renderer

namespace View
{
class Action;
class AnimationManager;
class Command;
class MapDocument;
class MapViewToolBox;
class Selection;
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
  Logger* m_logger;
  std::weak_ptr<MapDocument> m_document;
  MapViewToolBox& m_toolBox;

  std::unique_ptr<AnimationManager> m_animationManager;

private:
  Renderer::MapRenderer& m_renderer;
  std::unique_ptr<Renderer::Compass> m_compass;
  std::unique_ptr<Renderer::PrimitiveRenderer> m_portalFileRenderer;

  /**
   * Tracks whether this map view has most recently gotten the focus. This is tracked and
   * updated by a MapViewActivationTracker instance.
   */
  bool m_isCurrent;

  SignalDelayer* m_updateActionStatesSignalDelayer;

  NotifierConnection m_notifierConnection;

private: // shortcuts
  std::vector<std::pair<QShortcut*, const Action*>> m_shortcuts;

protected:
  MapViewBase(
    Logger* logger,
    std::weak_ptr<MapDocument> document,
    MapViewToolBox& toolBox,
    Renderer::MapRenderer& renderer,
    GLContextManager& contextManager);

  void setCompass(std::unique_ptr<Renderer::Compass> compass);

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

private:
  void bindEvents();
  void connectObservers();

  void createActionsAndUpdatePicking();

  void nodesDidChange(const std::vector<Model::Node*>& nodes);
  void toolChanged(Tool& tool);
  void commandDone(Command& command);
  void commandUndone(UndoableCommand& command);
  void selectionDidChange(const Selection& selection);
  void textureCollectionsDidChange();
  void entityDefinitionsDidChange();
  void modsDidChange();
  void editorContextDidChange();
  void gridDidChange();
  void pointFileDidChange();
  void portalFileDidChange();
  void preferenceDidChange(const IO::Path& path);
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
  vm::vec3 moveDirection(vm::direction direction) const;

  void duplicateAndMoveObjects(vm::direction direction);
  void duplicateObjects();

  void rotateObjects(vm::rotation_axis axis, bool clockwise);
  vm::vec3 rotationAxis(vm::rotation_axis axis, bool clockwise) const;

  void flipObjects(vm::direction direction);
  bool canFlipObjects() const;

public: // texture actions
  enum class TextureActionMode
  {
    Normal,
    Coarse,
    Fine
  };

  void moveTextures(vm::direction direction, TextureActionMode mode);
  vm::vec2f moveTextureOffset(vm::direction direction, TextureActionMode mode) const;
  float moveTextureDistance(TextureActionMode mode) const;
  void rotateTextures(bool clockwise, TextureActionMode mode);
  float rotateTextureAngle(bool clockwise, TextureActionMode mode) const;
  void flipTextures(vm::direction direction);
  void resetTextures();
  void resetTexturesToWorld();

public: // tool mode actions
  void createComplexBrush();
  void createPrimitiveBrush();
  void toggleClipSide();
  void performClip();

public: // misc actions
  void resetCameraZoom();
  void cancel();
  void deactivateTool();

public: // reparenting objects
  void addSelectedObjectsToGroup();
  void removeSelectedObjectsFromGroup();
  Model::Node* findNewGroupForObjects(const std::vector<Model::Node*>& nodes) const;

  void mergeSelectedGroups();
  Model::GroupNode* findGroupToMergeGroupsInto(
    const Model::NodeCollection& selectedNodes) const;

  /**
   * Checks whether the given node can be reparented under the given new parent.
   *
   * @param node the node to reparent
   * @param newParent the new parent node
   * @return true if the given node can be reparented under the given new parent, and
   * false otherwise
   */
  bool canReparentNode(const Model::Node* node, const Model::Node* newParent) const;

  void moveSelectedBrushesToEntity();
  Model::Node* findNewParentEntityForBrushes(
    const std::vector<Model::Node*>& nodes) const;

  bool canReparentNodes(
    const std::vector<Model::Node*>& nodes, const Model::Node* newParent) const;
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
    const std::vector<Model::Node*>& nodes,
    Model::Node* newParent,
    bool preserveEntities);
  std::vector<Model::Node*> collectReparentableNodes(
    const std::vector<Model::Node*>& nodes, const Model::Node* newParent) const;

  void createPointEntity();
  void createBrushEntity();

  Assets::EntityDefinition* findEntityDefinition(
    Assets::EntityDefinitionType type, size_t index) const;
  void createPointEntity(const Assets::PointEntityDefinition* definition);
  void createBrushEntity(const Assets::BrushEntityDefinition* definition);
  bool canCreateBrushEntity();

public: // tags
  void toggleTagVisible(const Model::SmartTag& tag);
  void enableTag(const Model::SmartTag& tag);
  void disableTag(const Model::SmartTag& tag);

public: // make structural
  void makeStructural();

public: // entity definitions
  void toggleEntityDefinitionVisible(const Assets::EntityDefinition* definition);
  void createEntity(const Assets::EntityDefinition* definition);

public: // view filters
  void toggleShowEntityClassnames();
  void toggleShowGroupBounds();
  void toggleShowBrushEntityBounds();
  void toggleShowPointEntityBounds();
  void toggleShowPointEntities();
  void toggleShowPointEntityModels();
  void toggleShowBrushes();
  void showTextures();
  void hideTextures();
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

private: // implement ViewEffectsService interface
  void doFlashSelection() override;

private: // implement MapView interface
  void doInstallActivationTracker(MapViewActivationTracker& activationTracker) override;
  bool doGetIsCurrent() const override;
  MapViewBase* doGetFirstMapViewBase() override;
  bool doCancelMouseDrag() override;
  void doRefreshViews() override;

protected: // RenderView overrides
  void initializeGL() override;

private: // implement RenderView interface
  bool doShouldRenderFocusIndicator() const override;
  void doRender() override;

  void setupGL(Renderer::RenderContext& renderContext);
  void renderCoordinateSystem(
    Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
  void renderSoftMapBounds(
    Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
  void renderPointFile(
    Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);

  void renderPortalFile(
    Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
  void invalidatePortalFileRenderer();
  void validatePortalFileRenderer(Renderer::RenderContext& renderContext);

  void renderCompass(Renderer::RenderBatch& renderBatch);
  void renderFPS(
    Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);

public: // implement InputEventProcessor interface
  void processEvent(const KeyEvent& event) override;
  void processEvent(const MouseEvent& event) override;
  void processEvent(const CancelEvent& event) override;

private: // implement ToolBoxConnector
  void doShowPopupMenu() override;
public slots:
  void showPopupMenuLater();

protected: // QWidget overrides
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dragLeaveEvent(QDragLeaveEvent* event) override;
  void dragMoveEvent(QDragMoveEvent* event) override;
  void dropEvent(QDropEvent* event) override;

private:
  QMenu* makeEntityGroupsMenu(Assets::EntityDefinitionType type);

  bool canMergeGroups() const;
  bool canMakeStructural() const;

private: // subclassing interface
  virtual vm::vec3 doGetMoveDirection(vm::direction direction) const = 0;
  virtual size_t doGetFlipAxis(vm::direction direction) const = 0;
  virtual vm::vec3 doComputePointEntityPosition(const vm::bbox3& bounds) const = 0;

  virtual ActionContext::Type doGetActionContext() const = 0;
  virtual ActionView doGetActionView() const = 0;
  virtual bool doCancel() = 0;

  virtual Renderer::RenderMode doGetRenderMode() = 0;
  virtual Renderer::Camera& doGetCamera() = 0;
  virtual void doPreRender();
  virtual void doRenderGrid(
    Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) = 0;
  virtual void doRenderMap(
    Renderer::MapRenderer& renderer,
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch) = 0;
  virtual void doRenderTools(
    MapViewToolBox& toolBox,
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch) = 0;
  virtual void doRenderExtras(
    Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
  virtual void doRenderSoftWorldBounds(
    Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) = 0;

  virtual bool doBeforePopupMenu();
  virtual void doAfterPopupMenu();
};
} // namespace View
} // namespace TrenchBroom
