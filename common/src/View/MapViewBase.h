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
#include "View/GLAttribs.h"
#include "View/InputState.h"
#include "View/MapView.h"
#include "View/RenderView.h"
#include "View/ToolBoxConnector.h"
#include "View/UndoableCommand.h"
#include "View/ViewTypes.h"

#include <memory>

namespace TrenchBroom {
    class Logger;
    
    namespace IO {
        class Path;
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
        class AnimationManager;
        class Command;
        class FlyModeHelper;
        class GLContextManager;
        class MapViewToolBox;
        class MovementRestriction;
        class Selection;
        class Tool;
        
        class MapViewBase : public MapView, public RenderView, public ToolBoxConnector, public CameraLinkableView {
        public:
            static const wxString &glRendererString();
            static const wxString &glVendorString();
            static const wxString &glVersionString();
        protected:
            static const wxLongLong DefaultCameraAnimationDuration;
            
            Logger* m_logger;
            MapDocumentWPtr m_document;
            MapViewToolBox& m_toolBox;
            
            AnimationManager* m_animationManager;
        private:
            Renderer::MapRenderer& m_renderer;
            Renderer::Compass* m_compass;
            std::unique_ptr<Renderer::PrimitiveRenderer> m_portalFileRenderer;
        protected:
            MapViewBase(wxWindow* parent, Logger* logger, MapDocumentWPtr document, MapViewToolBox& toolBox, Renderer::MapRenderer& renderer, GLContextManager& contextManager);
            
            void setCompass(Renderer::Compass* compass);
        public:
            virtual ~MapViewBase() override;
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
        private: // interaction events
            void bindEvents();
            
            void OnMoveObjectsForward(wxCommandEvent& event);
            void OnMoveObjectsBackward(wxCommandEvent& event);
            void OnMoveObjectsLeft(wxCommandEvent& event);
            void OnMoveObjectsRight(wxCommandEvent& event);
            void OnMoveObjectsUp(wxCommandEvent& event);
            void OnMoveObjectsDown(wxCommandEvent& event);
            
            void OnDuplicateObjectsForward(wxCommandEvent& event);
            void OnDuplicateObjectsBackward(wxCommandEvent& event);
            void OnDuplicateObjectsLeft(wxCommandEvent& event);
            void OnDuplicateObjectsRight(wxCommandEvent& event);
            void OnDuplicateObjectsUp(wxCommandEvent& event);
            void OnDuplicateObjectsDown(wxCommandEvent& event);
            
            void OnRollObjectsCW(wxCommandEvent& event);
            void OnRollObjectsCCW(wxCommandEvent& event);
            void OnPitchObjectsCW(wxCommandEvent& event);
            void OnPitchObjectsCCW(wxCommandEvent& event);
            void OnYawObjectsCW(wxCommandEvent& event);
            void OnYawObjectsCCW(wxCommandEvent& event);
            
            void OnFlipObjectsH(wxCommandEvent& event);
            void OnFlipObjectsV(wxCommandEvent& event);
            
            void duplicateAndMoveObjects(vm::direction direction);
            void duplicateObjects();
            void moveObjects(vm::direction direction);
            vm::vec3 moveDirection(vm::direction direction) const;
            void rotateObjects(vm::rotation_axis axis, bool clockwise);
            vm::vec3 rotationAxis(vm::rotation_axis axis, bool clockwise) const;
        private: // tool mode events
            void OnToggleRotateObjectsTool(wxCommandEvent& event);
            void OnMoveRotationCenterForward(wxCommandEvent& event);
            void OnMoveRotationCenterBackward(wxCommandEvent& event);
            void OnMoveRotationCenterLeft(wxCommandEvent& event);
            void OnMoveRotationCenterRight(wxCommandEvent& event);
            void OnMoveRotationCenterUp(wxCommandEvent& event);
            void OnMoveRotationCenterDown(wxCommandEvent& event);
            void moveRotationCenter(vm::direction direction);
            
            void OnToggleScaleObjectsTool(wxCommandEvent& event);
            void OnToggleShearObjectsTool(wxCommandEvent& event);
            
            void OnToggleClipSide(wxCommandEvent& event);
            void OnPerformClip(wxCommandEvent& event);
            
            void OnMoveVerticesForward(wxCommandEvent& event);
            void OnMoveVerticesBackward(wxCommandEvent& event);
            void OnMoveVerticesLeft(wxCommandEvent& event);
            void OnMoveVerticesRight(wxCommandEvent& event);
            void OnMoveVerticesUp(wxCommandEvent& event);
            void OnMoveVerticesDown(wxCommandEvent& event);
            void moveVertices(vm::direction direction);
            
            void OnCancel(wxCommandEvent& event);
            bool cancel();
            
            void OnDeactivateTool(wxCommandEvent& event);
        private: // group management
            void OnGroupSelectedObjects(wxCommandEvent& event);
            void OnUngroupSelectedObjects(wxCommandEvent& event);
            void OnRenameGroups(wxCommandEvent& event);
        private: // reparenting objects
            void OnAddObjectsToGroup(wxCommandEvent& event);
            void OnRemoveObjectsFromGroup(wxCommandEvent& event);
            Model::Node* findNewGroupForObjects(const Model::NodeList& nodes) const;
            
            void OnMergeGroups(wxCommandEvent& event);
            Model::Group* findGroupToMergeGroupsInto(const Model::NodeCollection& selectedNodes) const;

            /**
             * Checks whether the given node can be reparented under the given new parent.
             *
             * @param node the node to reparent
             * @param newParent the new parent node
             * @return true if the given node can be reparented under the given new parent, and false otherwise
             */
            bool canReparentNode(const Model::Node* node, const Model::Node* newParent) const;
            
            void OnMoveBrushesTo(wxCommandEvent& event);
            Model::Node* findNewParentEntityForBrushes(const Model::NodeList& nodes) const;
            
            bool canReparentNodes(const Model::NodeList& nodes, const Model::Node* newParent) const;
            void reparentNodes(const Model::NodeList& nodes, Model::Node* newParent, bool preserveEntities);
            Model::NodeList collectReparentableNodes(const Model::NodeList& nodes, const Model::Node* newParent) const;
            
            void OnCreatePointEntity(wxCommandEvent& event);
            void OnCreateBrushEntity(wxCommandEvent& event);
            
            Assets::EntityDefinition* findEntityDefinition(Assets::EntityDefinition::Type type, size_t index) const;
            void createPointEntity(const Assets::PointEntityDefinition* definition);
            void createBrushEntity(const Assets::BrushEntityDefinition* definition);
            bool canCreateBrushEntity();
        private: // other events
            void OnSetFocus(wxFocusEvent& event);
            void OnKillFocus(wxFocusEvent& event);
            void OnActivateFrame(wxActivateEvent& event);
        protected: // accelerator table management
            void updateAcceleratorTable();
        private:
            void updateAcceleratorTable(bool hasFocus);
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
        private: // implement RenderView interface
            void doInitializeGL(bool firstInitialization) override;
            bool doShouldRenderFocusIndicator() const override;
            void doRender() override;

            void setupGL(Renderer::RenderContext& renderContext);
            void renderCoordinateSystem(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void renderPointFile(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);

            void renderPortalFile(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void invalidatePortalFileRenderer();
            void validatePortalFileRenderer(Renderer::RenderContext& renderContext);

            void renderCompass(Renderer::RenderBatch& renderBatch);
        private: // implement ToolBoxConnector
            void doShowPopupMenu() override;
            wxMenu* makeEntityGroupsMenu(Assets::EntityDefinition::Type type, int id);
            
            void OnUpdatePopupMenuItem(wxUpdateUIEvent& event);
            void updateGroupObjectsMenuItem(wxUpdateUIEvent& event) const;
            void updateUngroupObjectsMenuItem(wxUpdateUIEvent& event) const;
            void updateMergeGroupsMenuItem(wxUpdateUIEvent& event) const;
            void updateRenameGroupsMenuItem(wxUpdateUIEvent& event) const;
            void updateMoveBrushesToWorldMenuItem(wxUpdateUIEvent& event) const;
        private: // subclassing interface
            virtual vm::vec3 doGetMoveDirection(vm::direction direction) const = 0;
            virtual vm::vec3 doComputePointEntityPosition(const vm::bbox3& bounds) const = 0;

            virtual ActionContext doGetActionContext() const = 0;
            virtual wxAcceleratorTable doCreateAccelerationTable(ActionContext context) const = 0;
            virtual bool doCancel() = 0;
            
            virtual Renderer::RenderContext::RenderMode doGetRenderMode() = 0;
            virtual Renderer::Camera& doGetCamera() = 0;
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
