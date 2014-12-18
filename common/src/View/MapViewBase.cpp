/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "MapViewBase.h"

#include "Logger.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/Brush.h"
#include "Model/BrushVertex.h"
#include "Model/Entity.h"
#include "Model/PointFile.h"
#include "Renderer/Camera.h"
#include "Renderer/FontDescriptor.h"
#include "Renderer/MapRenderer.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "View/ActionManager.h"
#include "View/Animation.h"
#include "View/CameraAnimation.h"
#include "View/CommandIds.h"
#include "View/FlashSelectionAnimation.h"
#include "View/FlyModeHelper.h"
#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/MapViewToolBox.h"
#include "View/ToolBoxDropTarget.h"
#include "View/wxUtils.h"

namespace TrenchBroom {
    namespace View {
        MapViewBase::MapViewBase(wxWindow* parent, Logger* logger, MapDocumentWPtr document, MapViewToolBox& toolBox, Renderer::MapRenderer& renderer, GLContextManager& contextManager) :
        RenderView(parent, contextManager, buildAttribs()),
        ToolBoxConnector(this),
        m_logger(logger),
        m_document(document),
        m_toolBox(toolBox),
        m_animationManager(new AnimationManager()),
        m_renderer(renderer) {
            setToolBox(toolBox);
            bindEvents();
            bindObservers();
            updateAcceleratorTable(HasFocus());
        }
        
        MapViewBase::~MapViewBase() {
            unbindObservers();
            delete m_animationManager;
        }

        void MapViewBase::setToolBoxDropTarget() {
            SetDropTarget(new ToolBoxDropTarget(this));
        }
        
        void MapViewBase::clearDropTarget() {
            SetDropTarget(NULL);
        }

        void MapViewBase::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->nodesWereAddedNotifier.addObserver(this, &MapViewBase::nodesDidChange);
            document->nodesWereRemovedNotifier.addObserver(this, &MapViewBase::nodesDidChange);
            document->nodesDidChangeNotifier.addObserver(this, &MapViewBase::nodesDidChange);
            document->commandDoneNotifier.addObserver(this, &MapViewBase::commandProcessed);
            document->commandUndoneNotifier.addObserver(this, &MapViewBase::commandProcessed);
            document->selectionDidChangeNotifier.addObserver(this, &MapViewBase::selectionDidChange);
            
            Grid& grid = document->grid();
            grid.gridDidChangeNotifier.addObserver(this, &MapViewBase::gridDidChange);
            
            m_toolBox.toolActivatedNotifier.addObserver(this, &MapViewBase::toolChanged);
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &MapViewBase::preferenceDidChange);
        }
        
        void MapViewBase::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->nodesWereAddedNotifier.removeObserver(this, &MapViewBase::nodesDidChange);
                document->nodesWereRemovedNotifier.removeObserver(this, &MapViewBase::nodesDidChange);
                document->nodesDidChangeNotifier.removeObserver(this, &MapViewBase::nodesDidChange);
                document->commandDoneNotifier.removeObserver(this, &MapViewBase::commandProcessed);
                document->commandUndoneNotifier.removeObserver(this, &MapViewBase::commandProcessed);
                document->selectionDidChangeNotifier.removeObserver(this, &MapViewBase::selectionDidChange);
                
                Grid& grid = document->grid();
                grid.gridDidChangeNotifier.removeObserver(this, &MapViewBase::gridDidChange);
            }
            
            // toolbox has already been deleted at this point
            // m_toolBox.toolActivatedNotifier.removeObserver(this, &MapViewBase::toolChanged);
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &MapViewBase::preferenceDidChange);
        }
        
        void MapViewBase::nodesDidChange(const Model::NodeList& nodes) {
            Refresh();
        }

        void MapViewBase::toolChanged(Tool* tool) {
            updateHits();
            updateAcceleratorTable(HasFocus());
            Refresh();
        }
        
        void MapViewBase::commandProcessed(Command* command) {
            updateHits();
            Refresh();
        }
        
        void MapViewBase::selectionDidChange(const Selection& selection) {
            updateAcceleratorTable(HasFocus());
        }
        
        void MapViewBase::gridDidChange() {
            Refresh();
        }
        
        void MapViewBase::preferenceDidChange(const IO::Path& path) {
            Refresh();
        }

        void MapViewBase::bindEvents() {
            /*
             Bind(wxEVT_KEY_DOWN, &MapViewBase::OnKey, this);
             Bind(wxEVT_KEY_UP, &MapViewBase::OnKey, this);
             */
            
            Bind(wxEVT_SET_FOCUS, &MapViewBase::OnSetFocus, this);
            Bind(wxEVT_KILL_FOCUS, &MapViewBase::OnKillFocus, this);
            
            /*
             Bind(wxEVT_MENU, &MapViewBase::OnToggleClipTool,               this, CommandIds::Actions::ToggleClipTool);
             Bind(wxEVT_MENU, &MapViewBase::OnToggleClipSide,               this, CommandIds::Actions::ToggleClipSide);
             Bind(wxEVT_MENU, &MapViewBase::OnPerformClip,                  this, CommandIds::Actions::PerformClip);
             Bind(wxEVT_MENU, &MapViewBase::OnDeleteLastClipPoint,          this, CommandIds::Actions::DeleteLastClipPoint);
             */
            Bind(wxEVT_MENU, &MapViewBase::OnToggleVertexTool,             this, CommandIds::Actions::ToggleVertexTool);
            /*
             Bind(wxEVT_MENU, &MapViewBase::OnMoveVerticesForward,          this, CommandIds::Actions::MoveVerticesForward);
             Bind(wxEVT_MENU, &MapViewBase::OnMoveVerticesBackward,         this, CommandIds::Actions::MoveVerticesBackward);
             Bind(wxEVT_MENU, &MapViewBase::OnMoveVerticesLeft,             this, CommandIds::Actions::MoveVerticesLeft);
             Bind(wxEVT_MENU, &MapViewBase::OnMoveVerticesRight,            this, CommandIds::Actions::MoveVerticesRight);
             Bind(wxEVT_MENU, &MapViewBase::OnMoveVerticesUp,               this, CommandIds::Actions::MoveVerticesUp);
             Bind(wxEVT_MENU, &MapViewBase::OnMoveVerticesDown,             this, CommandIds::Actions::MoveVerticesDown);
             */
            
            /*
             Bind(wxEVT_MENU, &MapViewBase::OnToggleMovementRestriction,    this, CommandIds::Actions::ToggleMovementRestriction);
             */
            
            Bind(wxEVT_MENU, &MapViewBase::OnDeleteObjects,                this, CommandIds::Actions::DeleteObjects);
            
            Bind(wxEVT_MENU, &MapViewBase::OnMoveObjectsForward,           this, CommandIds::Actions::MoveObjectsForward);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveObjectsBackward,          this, CommandIds::Actions::MoveObjectsBackward);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveObjectsLeft,              this, CommandIds::Actions::MoveObjectsLeft);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveObjectsRight,             this, CommandIds::Actions::MoveObjectsRight);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveObjectsUp,                this, CommandIds::Actions::MoveObjectsUp);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveObjectsDown,              this, CommandIds::Actions::MoveObjectsDown);
            
            Bind(wxEVT_MENU, &MapViewBase::OnDuplicateObjects,             this, CommandIds::Actions::DuplicateObjects);
            Bind(wxEVT_MENU, &MapViewBase::OnDuplicateObjectsForward,      this, CommandIds::Actions::DuplicateObjectsForward);
            Bind(wxEVT_MENU, &MapViewBase::OnDuplicateObjectsBackward,     this, CommandIds::Actions::DuplicateObjectsBackward);
            Bind(wxEVT_MENU, &MapViewBase::OnDuplicateObjectsLeft,         this, CommandIds::Actions::DuplicateObjectsLeft);
            Bind(wxEVT_MENU, &MapViewBase::OnDuplicateObjectsRight,        this, CommandIds::Actions::DuplicateObjectsRight);
            Bind(wxEVT_MENU, &MapViewBase::OnDuplicateObjectsUp,           this, CommandIds::Actions::DuplicateObjectsUp);
            Bind(wxEVT_MENU, &MapViewBase::OnDuplicateObjectsDown,         this, CommandIds::Actions::DuplicateObjectsDown);
            
            Bind(wxEVT_MENU, &MapViewBase::OnRollObjectsCW,                this, CommandIds::Actions::RollObjectsCW);
            Bind(wxEVT_MENU, &MapViewBase::OnRollObjectsCCW,               this, CommandIds::Actions::RollObjectsCCW);
            Bind(wxEVT_MENU, &MapViewBase::OnPitchObjectsCW,               this, CommandIds::Actions::PitchObjectsCW);
            Bind(wxEVT_MENU, &MapViewBase::OnPitchObjectsCCW,              this, CommandIds::Actions::PitchObjectsCCW);
            Bind(wxEVT_MENU, &MapViewBase::OnYawObjectsCW,                 this, CommandIds::Actions::YawObjectsCW);
            Bind(wxEVT_MENU, &MapViewBase::OnYawObjectsCCW,                this, CommandIds::Actions::YawObjectsCCW);
            
            Bind(wxEVT_MENU, &MapViewBase::OnFlipObjectsH,                 this, CommandIds::Actions::FlipObjectsHorizontally);
            Bind(wxEVT_MENU, &MapViewBase::OnFlipObjectsV,                 this, CommandIds::Actions::FlipObjectsVertically);
            
            Bind(wxEVT_MENU, &MapViewBase::OnToggleRotateObjectsTool,      this, CommandIds::Actions::ToggleRotateObjectsTool);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveRotationCenterForward,    this, CommandIds::Actions::MoveRotationCenterForward);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveRotationCenterBackward,   this, CommandIds::Actions::MoveRotationCenterBackward);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveRotationCenterLeft,       this, CommandIds::Actions::MoveRotationCenterLeft);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveRotationCenterRight,      this, CommandIds::Actions::MoveRotationCenterRight);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveRotationCenterUp,         this, CommandIds::Actions::MoveRotationCenterUp);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveRotationCenterDown,       this, CommandIds::Actions::MoveRotationCenterDown);
            
            Bind(wxEVT_MENU, &MapViewBase::OnCancel,                       this, CommandIds::Actions::Cancel);
            
            /*
             Bind(wxEVT_MENU, &MapViewBase::OnPopupReparentBrushes,         this, CommandIds::CreateEntityPopupMenu::ReparentBrushes);
             Bind(wxEVT_MENU, &MapViewBase::OnPopupMoveBrushesToWorld,      this, CommandIds::CreateEntityPopupMenu::MoveBrushesToWorld);
             Bind(wxEVT_MENU, &MapViewBase::OnPopupCreatePointEntity,       this, CommandIds::CreateEntityPopupMenu::LowestPointEntityItem, CommandIds::CreateEntityPopupMenu::HighestPointEntityItem);
             Bind(wxEVT_MENU, &MapViewBase::OnPopupCreateBrushEntity,       this, CommandIds::CreateEntityPopupMenu::LowestBrushEntityItem, CommandIds::CreateEntityPopupMenu::HighestBrushEntityItem);
             
             Bind(wxEVT_UPDATE_UI, &MapViewBase::OnUpdatePopupMenuItem, this, CommandIds::CreateEntityPopupMenu::ReparentBrushes);
             Bind(wxEVT_UPDATE_UI, &MapViewBase::OnUpdatePopupMenuItem, this, CommandIds::CreateEntityPopupMenu::MoveBrushesToWorld);
             Bind(wxEVT_UPDATE_UI, &MapViewBase::OnUpdatePopupMenuItem, this, CommandIds::CreateEntityPopupMenu::LowestPointEntityItem, CommandIds::CreateEntityPopupMenu::HighestPointEntityItem);
             Bind(wxEVT_UPDATE_UI, &MapViewBase::OnUpdatePopupMenuItem, this, CommandIds::CreateEntityPopupMenu::LowestBrushEntityItem, CommandIds::CreateEntityPopupMenu::HighestBrushEntityItem);
             */
            
            wxFrame* frame = findFrame(this);
            frame->Bind(wxEVT_ACTIVATE, &MapViewBase::OnActivateFrame, this);
        }
        
        void MapViewBase::OnDeleteObjects(wxCommandEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            if (document->hasSelectedNodes())
                document->deleteObjects();
        }
        
        void MapViewBase::OnMoveObjectsForward(wxCommandEvent& event) {
            moveObjects(Math::Direction_Forward);
        }
        
        void MapViewBase::OnMoveObjectsBackward(wxCommandEvent& event) {
            moveObjects(Math::Direction_Backward);
        }
        
        void MapViewBase::OnMoveObjectsLeft(wxCommandEvent& event) {
            moveObjects(Math::Direction_Left);
        }
        
        void MapViewBase::OnMoveObjectsRight(wxCommandEvent& event) {
            moveObjects(Math::Direction_Right);
        }
        
        void MapViewBase::OnMoveObjectsUp(wxCommandEvent& event) {
            moveObjects(Math::Direction_Up);
        }
        
        void MapViewBase::OnMoveObjectsDown(wxCommandEvent& event) {
            moveObjects(Math::Direction_Down);
        }
        
        void MapViewBase::OnDuplicateObjects(wxCommandEvent& event) {
            duplicateObjects();
        }
        
        void MapViewBase::OnDuplicateObjectsForward(wxCommandEvent& event) {
            duplicateAndMoveObjects(Math::Direction_Forward);
        }
        
        void MapViewBase::OnDuplicateObjectsBackward(wxCommandEvent& event) {
            duplicateAndMoveObjects(Math::Direction_Backward);
        }
        
        void MapViewBase::OnDuplicateObjectsLeft(wxCommandEvent& event) {
            duplicateAndMoveObjects(Math::Direction_Left);
        }
        
        void MapViewBase::OnDuplicateObjectsRight(wxCommandEvent& event) {
            duplicateAndMoveObjects(Math::Direction_Right);
        }
        
        void MapViewBase::OnDuplicateObjectsUp(wxCommandEvent& event) {
            duplicateAndMoveObjects(Math::Direction_Up);
        }
        
        void MapViewBase::OnDuplicateObjectsDown(wxCommandEvent& event) {
            duplicateAndMoveObjects(Math::Direction_Down);
        }
        
        void MapViewBase::OnRollObjectsCW(wxCommandEvent& event) {
            rotateObjects(Math::RotationAxis_Roll, true);
        }
        
        void MapViewBase::OnRollObjectsCCW(wxCommandEvent& event) {
            rotateObjects(Math::RotationAxis_Roll, false);
        }
        
        void MapViewBase::OnPitchObjectsCW(wxCommandEvent& event) {
            rotateObjects(Math::RotationAxis_Pitch, true);
        }
        
        void MapViewBase::OnPitchObjectsCCW(wxCommandEvent& event) {
            rotateObjects(Math::RotationAxis_Pitch, false);
        }
        
        void MapViewBase::OnYawObjectsCW(wxCommandEvent& event) {
            rotateObjects(Math::RotationAxis_Yaw, true);
        }
        
        void MapViewBase::OnYawObjectsCCW(wxCommandEvent& event) {
            rotateObjects(Math::RotationAxis_Yaw, false);
        }
        
        void MapViewBase::OnFlipObjectsH(wxCommandEvent& event) {
            flipObjects(Math::Direction_Left);
        }
        
        void MapViewBase::OnFlipObjectsV(wxCommandEvent& event) {
            flipObjects(Math::Direction_Up);
        }
        
        void MapViewBase::duplicateAndMoveObjects(const Math::Direction direction) {
            Transaction transaction(m_document);
            duplicateObjects();
            moveObjects(direction);
        }
        
        void MapViewBase::duplicateObjects() {
            MapDocumentSPtr document = lock(m_document);
            if (!document->hasSelectedNodes())
                return;
            
            document->duplicateObjects();
            flashSelection();
        }
        
        void MapViewBase::moveObjects(const Math::Direction direction) {
            MapDocumentSPtr document = lock(m_document);
            if (!document->hasSelectedNodes())
                return;
            
            const Grid& grid = document->grid();
            const Vec3 delta = moveDirection(direction) * static_cast<FloatType>(grid.actualSize());
            document->translateObjects(delta);
        }
        
        Vec3 MapViewBase::moveDirection(const Math::Direction direction) const {
            return doGetMoveDirection(direction);
        }
        
        void MapViewBase::rotateObjects(const Math::RotationAxis axisSpec, const bool clockwise) {
            MapDocumentSPtr document = lock(m_document);
            if (!document->hasSelectedNodes())
                return;
            
            const Vec3 axis = rotationAxis(axisSpec, clockwise);
            const double angle = m_toolBox.rotateObjectsToolActive() ? std::abs(m_toolBox.rotateToolAngle()) : Math::C::piOverTwo();
            
            const Grid& grid = document->grid();
            const Vec3 center = m_toolBox.rotateObjectsToolActive() ? m_toolBox.rotateToolCenter() : grid.referencePoint(document->selectionBounds());
            
            document->rotateObjects(center, axis, angle);
        }
        
        Vec3 MapViewBase::rotationAxis(const Math::RotationAxis axisSpec, const bool clockwise) const {
            Vec3 axis;
            switch (axisSpec) {
                case Math::RotationAxis_Roll:
                    axis = -moveDirection(Math::Direction_Forward);
                    break;
                case Math::RotationAxis_Pitch:
                    axis = moveDirection(Math::Direction_Right);
                    break;
                case Math::RotationAxis_Yaw:
                    axis = Vec3::PosZ;
                    break;
                    DEFAULT_SWITCH()
            }
            
            if (clockwise)
                axis = -axis;
            return axis;
        }
        
        void MapViewBase::flipObjects(const Math::Direction direction) {
            MapDocumentSPtr document = lock(m_document);
            if (!document->hasSelectedNodes())
                return;
            
            const Grid& grid = document->grid();
            const Vec3 center = grid.referencePoint(document->selectionBounds());
            const Math::Axis::Type axis = moveDirection(direction).firstComponent();
            
            document->flipObjects(center, axis);
        }
        
        void MapViewBase::OnToggleRotateObjectsTool(wxCommandEvent& event) {
            m_toolBox.toggleRotateObjectsTool();
        }
        
        void MapViewBase::OnMoveRotationCenterForward(wxCommandEvent& event) {
            moveRotationCenter(Math::Direction_Forward);
        }
        
        void MapViewBase::OnMoveRotationCenterBackward(wxCommandEvent& event) {
            moveRotationCenter(Math::Direction_Backward);
        }
        
        void MapViewBase::OnMoveRotationCenterLeft(wxCommandEvent& event) {
            moveRotationCenter(Math::Direction_Left);
        }
        
        void MapViewBase::OnMoveRotationCenterRight(wxCommandEvent& event) {
            moveRotationCenter(Math::Direction_Right);
        }
        
        void MapViewBase::OnMoveRotationCenterUp(wxCommandEvent& event) {
            moveRotationCenter(Math::Direction_Up);
        }
        
        void MapViewBase::OnMoveRotationCenterDown(wxCommandEvent& event) {
            moveRotationCenter(Math::Direction_Down);
        }
        
        void MapViewBase::moveRotationCenter(const Math::Direction direction) {
            MapDocumentSPtr document = lock(m_document);
            const Grid& grid = document->grid();
            const Vec3 delta = moveDirection(direction) * static_cast<FloatType>(grid.actualSize());
            m_toolBox.moveRotationCenter(delta);
            Refresh();
        }
        
        void MapViewBase::OnToggleVertexTool(wxCommandEvent& event) {
            m_toolBox.toggleVertexTool();
        }
        
        void MapViewBase::OnCancel(wxCommandEvent& event) {
            if (MapViewBase::cancel())
                return;
            if (ToolBoxConnector::cancel())
                return;
            lock(m_document)->deselectAll();
        }
        
        bool MapViewBase::cancel() {
            return doCancel();
        }
        
        void MapViewBase::OnSetFocus(wxFocusEvent& event) {
            updateAcceleratorTable(true);
            event.Skip();
        }
        
        void MapViewBase::OnKillFocus(wxFocusEvent& event) {
            updateAcceleratorTable(false);
            event.Skip();
        }
        
        void MapViewBase::OnActivateFrame(wxActivateEvent& event) {
            if (event.GetActive())
                updateLastActivation();
            event.Skip();
        }
        
        void MapViewBase::updateAcceleratorTable() {
            updateAcceleratorTable(HasFocus());
        }

        void MapViewBase::updateAcceleratorTable(const bool hasFocus) {
            if (hasFocus) {
                const wxAcceleratorTable acceleratorTable = doCreateAccelerationTable(actionContext());
                SetAcceleratorTable(acceleratorTable);
            } else {
                SetAcceleratorTable(wxNullAcceleratorTable);
            }
        }
        
        ActionContext MapViewBase::actionContext() const {
            const ActionContext derivedContext = doGetActionContext();
            if (derivedContext != ActionContext_Default)
                return derivedContext;
            
            /*
             if (clipToolActive())
             return Action::Context_ClipTool;
             */
            if (m_toolBox.vertexToolActive())
                return ActionContext_VertexTool;
            if (m_toolBox.rotateObjectsToolActive())
                return ActionContext_RotateTool;
            
            MapDocumentSPtr document = lock(m_document);
            if (document->hasSelectedNodes())
                return ActionContext_NodeSelection;
            if (document->hasSelectedBrushFaces())
                return ActionContext_FaceSelection;
            return ActionContext_Default;
        }
        
        void MapViewBase::flashSelection() {
            FlashSelectionAnimation* animation = new FlashSelectionAnimation(m_renderer, *this, 180);
            m_animationManager->runAnimation(animation, true);
        }

        void MapViewBase::doInitializeGL(const bool firstInitialization) {
            if (firstInitialization) {
                const wxString vendor   = wxString::FromUTF8(reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
                const wxString renderer = wxString::FromUTF8(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
                const wxString version  = wxString::FromUTF8(reinterpret_cast<const char*>(glGetString(GL_VERSION)));
                
                m_logger->info(wxString::Format(L"Renderer info: %s version %s from %s", renderer, version, vendor));
                m_logger->info("Depth buffer bits: %d", depthBits());
                
                if (multisample())
                    m_logger->info("Multisampling enabled");
                else
                    m_logger->info("Multisampling disabled");
            }
        }
        
        bool MapViewBase::doShouldRenderFocusIndicator() const {
            return true;
        }
        
        void MapViewBase::doRender() {
            const IO::Path& fontPath = pref(Preferences::RendererFontPath());
            const size_t fontSize = static_cast<size_t>(pref(Preferences::RendererFontSize));
            const Renderer::FontDescriptor fontDescriptor(fontPath, fontSize);

            Renderer::RenderContext renderContext = createRenderContext();
            
            setupGL(renderContext);
            setRenderOptions(renderContext);
            
            Renderer::RenderBatch renderBatch(sharedVbo());
            
            doRenderMap(m_renderer, renderContext, renderBatch);
            doRenderTools(m_toolBox, renderContext, renderBatch);
            doRenderExtras(renderContext, renderBatch);
            
            renderBatch.render(renderContext);
        }
        
        Renderer::RenderContext MapViewBase::createRenderContext() {
            MapDocumentSPtr document = lock(m_document);
            const Grid& grid = document->grid();
            
            Renderer::RenderContext renderContext = doCreateRenderContext();
            renderContext.setShowGrid(grid.visible());
            renderContext.setGridSize(grid.actualSize());
            return renderContext;
        }
        
        void MapViewBase::setupGL(Renderer::RenderContext& context) {
            const Renderer::Camera::Viewport& viewport = context.camera().unzoomedViewport();
            glViewport(viewport.x, viewport.y, viewport.width, viewport.height);
            
            glEnable(GL_MULTISAMPLE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glShadeModel(GL_SMOOTH);
        }
        
        void MapViewBase::doShowPopupMenu() {
        }
    }
}
