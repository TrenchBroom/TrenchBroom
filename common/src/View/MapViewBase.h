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

#ifndef TrenchBroom_MapViewBase
#define TrenchBroom_MapViewBase

#include "Assets/EntityDefinition.h"
#include "Model/ModelTypes.h"
#include "Model/NodeCollection.h"
#include "Renderer/RenderContext.h"
#include "View/ActionContext.h"
#include "View/CameraLinkHelper.h"
#include "View/InputState.h"
#include "View/MapView.h"
#include "View/RenderView.h"
#include "View/ToolBoxConnector.h"
#include "View/UndoableCommand.h"
#include "View/ViewTypes.h"

#include <functional>
#include <memory>
#include <utility>
#include <vector>

class QMenu;
class QShortcut;
class QString;
class QAction;

namespace TrenchBroom {
    class Logger;

    namespace IO {
        class Path;
    }

    namespace Model {
        class SmartTag;
    }

    namespace Renderer {
        class Camera;
        class Compass;
        class MapRenderer;
        class PrimitiveRenderer;
        class RenderBatch;
        class RenderContext;
        class Vbo;
    }

    namespace View {
        struct ActionInfo;
        class AnimationManager;
        class Command;
        class FlyModeHelper;
        class GLContextManager;
        class MapViewToolBox;
        class MovementRestriction;
        class Selection;
        class Tool;

        class MapViewBase : public RenderView, public MapView, public ToolBoxConnector, public CameraLinkableView {
            Q_OBJECT
        public:
            static const QString& glRendererString();
            static const QString& glVendorString();
            static const QString& glVersionString();
        protected:
            static const int DefaultCameraAnimationDuration;

            Logger* m_logger;
            MapDocumentWPtr m_document;
            MapViewToolBox& m_toolBox;

            AnimationManager* m_animationManager;
        private:
            Renderer::MapRenderer& m_renderer;
            Renderer::Compass* m_compass;
            std::unique_ptr<Renderer::PrimitiveRenderer> m_portalFileRenderer;
        private: // shortcuts
            std::vector<std::pair<QShortcut*, ActionInfo>> m_actionInfoList;
            std::vector<QShortcut*> m_2DOnlyShortcuts;
            std::vector<QShortcut*> m_3DOnlyShortcuts;
        protected:
            MapViewBase(QWidget* parent, Logger* logger, MapDocumentWPtr document, MapViewToolBox& toolBox, Renderer::MapRenderer& renderer, GLContextManager& contextManager);

            void setCompass(Renderer::Compass* compass);
        public:
            ~MapViewBase() override;
        private:
            void bindObservers();
            void unbindObservers();

            void nodesDidChange(const Model::NodeList& nodes);
            void toolChanged(Tool* tool);
            void commandDone(Command::Ptr command);
            void commandUndone(UndoableCommand::Ptr command);
            void selectionDidChange(const Selection& selection);
            void textureCollectionsDidChange();
            void entityDefinitionsDidChange();
            void modsDidChange();
            void editorContextDidChange();
            void mapViewConfigDidChange();
            void gridDidChange();
            void pointFileDidChange();
            void portalFileDidChange();
            void preferenceDidChange(const IO::Path& path);
            void documentDidChange(MapDocument* document);
        private: // shortcut setup
            using Callback = void (MapViewBase::*)();
            QShortcut* createAndRegisterShortcut(const ActionInfo& info, Callback callback);
            QShortcut* createAndRegisterShortcut(const ActionInfo& info, const std::function<void()>& callback);
            void createAndRegister2D3DShortcut(const ActionInfo& info, Callback callback2D, Callback callback3D);
            void createActions();
            void registerBinding(QShortcut* action, const ActionInfo& info);
            void updateBindings();
        private: // interaction events
            void bindEvents();

            void OnMoveObjectsForward();
            void OnMoveObjectsBackward();
            void OnMoveObjectsLeft();
            void OnMoveObjectsRight();
            void OnMoveObjectsUp();
            void OnMoveObjectsDown();

            void OnDuplicateObjectsForward();
            void OnDuplicateObjectsBackward();
            void OnDuplicateObjectsLeft();
            void OnDuplicateObjectsRight();
            void OnDuplicateObjectsUp();
            void OnDuplicateObjectsDown();

            void OnRollObjectsCW();
            void OnRollObjectsCCW();
            void OnPitchObjectsCW();
            void OnPitchObjectsCCW();
            void OnYawObjectsCW();
            void OnYawObjectsCCW();

            void OnFlipObjectsH();
            void OnFlipObjectsV();

            void duplicateAndMoveObjects(vm::direction direction);
            void duplicateObjects();
            void moveObjects(vm::direction direction);
            vm::vec3 moveDirection(vm::direction direction) const;
            void rotateObjects(vm::rotation_axis axis, bool clockwise);
            vm::vec3 rotationAxis(vm::rotation_axis axis, bool clockwise) const;
        private: // tool mode events
            void OnToggleRotateObjectsTool();
            void OnMoveRotationCenterForward();
            void OnMoveRotationCenterBackward();
            void OnMoveRotationCenterLeft();
            void OnMoveRotationCenterRight();
            void OnMoveRotationCenterUp();
            void OnMoveRotationCenterDown();
            void moveRotationCenter(vm::direction direction);

            void OnToggleScaleObjectsTool();
            void OnToggleShearObjectsTool();

            void OnToggleClipSide();
            void OnPerformClip();

            void OnMoveVerticesForward();
            void OnMoveVerticesBackward();
            void OnMoveVerticesLeft();
            void OnMoveVerticesRight();
            void OnMoveVerticesUp();
            void OnMoveVerticesDown();
            void moveVertices(vm::direction direction);

            void OnCancel();
            bool cancel();

            void OnDeactivateTool();
        private: // group management
            void OnGroupSelectedObjects();
            void OnUngroupSelectedObjects();
            void OnRenameGroups();
        private: // reparenting objects
            void OnAddObjectsToGroup();
            void OnRemoveObjectsFromGroup();
            Model::Node* findNewGroupForObjects(const Model::NodeList& nodes) const;

            void OnMergeGroups();
            Model::Group* findGroupToMergeGroupsInto(const Model::NodeCollection& selectedNodes) const;

            /**
             * Checks whether the given node can be reparented under the given new parent.
             *
             * @param node the node to reparent
             * @param newParent the new parent node
             * @return true if the given node can be reparented under the given new parent, and false otherwise
             */
            bool canReparentNode(const Model::Node* node, const Model::Node* newParent) const;

            void OnMoveBrushesTo();
            Model::Node* findNewParentEntityForBrushes(const Model::NodeList& nodes) const;

            bool canReparentNodes(const Model::NodeList& nodes, const Model::Node* newParent) const;
            /**
             * Reparents nodes, and deselects everything as a side effect.
             *
             * @param nodes the nodes to reparent
             * @param newParent the new parent
             * @param preserveEntities if true, if `nodes` contains brushes belonging to an entity, the whole
             *                         entity and all brushes it contains are also reparented.
             *                         if false, only the brushes listed in `nodes` are reparented, not any
             *                         parent entities.
             */
            void reparentNodes(const Model::NodeList& nodes, Model::Node* newParent, bool preserveEntities);
            Model::NodeList collectReparentableNodes(const Model::NodeList& nodes, const Model::Node* newParent) const;

            void OnCreatePointEntity();
            void OnCreateBrushEntity();

            Assets::EntityDefinition* findEntityDefinition(Assets::EntityDefinition::Type type, size_t index) const;
            void createPointEntity(const Assets::PointEntityDefinition* definition);
            void createBrushEntity(const Assets::BrushEntityDefinition* definition);
            bool canCreateBrushEntity();
        private: // tags
            void OnToggleTagVisible(const Model::SmartTag& tag);

            class EnableDisableTagCallback;
            void OnEnableTag(const Model::SmartTag& tag);
            void OnDisableTag(const Model::SmartTag& tag);
        private: // make structural
            void OnMakeStructural();
        private: // entity definitions
            void OnToggleEntityDefinitionVisible();
            void OnCreateEntity();
        private: // view filters
            void OnToggleShowEntityClassnames();
            void OnToggleShowGroupBounds();
            void OnToggleShowBrushEntityBounds();
            void OnToggleShowPointEntityBounds();
            void OnToggleShowPointEntities();
            void OnToggleShowPointEntityModels();
            void OnToggleShowBrushes();
            void OnRenderModeShowTextures();
            void OnRenderModeHideTextures();
            void OnRenderModeHideFaces();
            void OnRenderModeShadeFaces();
            void OnRenderModeUseFog();
            void OnRenderModeShowEdges();
            void OnRenderModeShowAllEntityLinks();
            void OnRenderModeShowTransitiveEntityLinks();
            void OnRenderModeShowDirectEntityLinks();
            void OnRenderModeHideEntityLinks();
        private: // other events
            void onActiveChanged();
        private:
            ActionContext actionContext() const;
        private: // implement ViewEffectsService interface
            void doFlashSelection() override;
        private: // implement MapView interface
            bool doGetIsCurrent() const override;
            void doSetToolBoxDropTarget() override;
            void doClearDropTarget() override;
            bool doCanFlipObjects() const override;
            void doFlipObjects(vm::direction direction) override;
            bool doCancelMouseDrag() override;
            void doRefreshViews() override;
        protected: // RenderView overrides
            void initializeGL() override;
        private: // implement RenderView interface
            bool doShouldRenderFocusIndicator() const override;
            void doRender() override;

            void setupGL(Renderer::RenderContext& renderContext);
            void renderCoordinateSystem(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void renderPointFile(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);

            void renderPortalFile(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void invalidatePortalFileRenderer();
            void validatePortalFileRenderer(Renderer::RenderContext& renderContext);

            void renderCompass(Renderer::RenderBatch& renderBatch);
            void renderFPS(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
        public: // implement InputEventProcessor interface
            void processEvent(const KeyEvent& event) override;
            void processEvent(const MouseEvent& event) override;
            void processEvent(const CancelEvent& event) override;
        private: // implement ToolBoxConnector
            void doShowPopupMenu() override;
        public slots:
            void OnShowPopupMenu();
        public: // QWindow overrides
            bool event(QEvent* event) override;
        private:
            QMenu* makeEntityGroupsMenu(Assets::EntityDefinition::Type type);

            bool canGroupObjects() const;
            bool canUngroupObjects() const;
            bool canMergeGroups() const;
            bool canRenameGroups() const;
            bool canMakeStructural() const;
        private: // subclassing interface
            virtual vm::vec3 doGetMoveDirection(vm::direction direction) const = 0;
            virtual vm::vec3 doComputePointEntityPosition(const vm::bbox3& bounds) const = 0;

            virtual ActionContext doGetActionContext() const = 0;
            virtual ActionView doGetActionView() const = 0;
            virtual bool doCancel() = 0;

            virtual Renderer::RenderContext::RenderMode doGetRenderMode() = 0;
            virtual Renderer::Camera& doGetCamera() = 0;
            virtual void doPreRender();
            virtual void doRenderGrid(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) = 0;
            virtual void doRenderMap(Renderer::MapRenderer& renderer, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) = 0;
            virtual void doRenderTools(MapViewToolBox& toolBox, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) = 0;
            virtual void doRenderExtras(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);

            virtual bool doBeforePopupMenu();
            virtual void doAfterPopupMenu();
        };
    }
}

#endif /* defined(TrenchBroom_MapViewBase) */
