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
#include "Assets/EntityDefinitionManager.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/Entity.h"
#include "Model/EntityAttributes.h"
#include "Model/FindLayerVisitor.h"
#include "Model/Hit.h"
#include "Model/HitAdapter.h"
#include "Model/HitQuery.h"
#include "Model/Layer.h"
#include "Model/PointFile.h"
#include "Model/World.h"
#include "Renderer/Camera.h"
#include "Renderer/Compass.h"
#include "Renderer/FontDescriptor.h"
#include "Renderer/MapRenderer.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderService.h"
#include "View/ActionManager.h"
#include "View/Animation.h"
#include "View/CameraAnimation.h"
#include "View/CommandIds.h"
#include "View/FlashSelectionAnimation.h"
#include "View/FlyModeHelper.h"
#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/MapViewConfig.h"
#include "View/MapViewToolBox.h"
#include "View/ToolBoxDropTarget.h"
#include "View/ViewUtils.h"
#include "View/wxUtils.h"

namespace TrenchBroom {
    namespace View {
        const wxLongLong MapViewBase::DefaultCameraAnimationDuration = 250;

        MapViewBase::MapViewBase(wxWindow* parent, Logger* logger, MapDocumentWPtr document, MapViewToolBox& toolBox, Renderer::MapRenderer& renderer, GLContextManager& contextManager) :
        RenderView(parent, contextManager, buildAttribs()),
        ToolBoxConnector(this),
        m_logger(logger),
        m_document(document),
        m_toolBox(toolBox),
        m_animationManager(new AnimationManager()),
        m_renderer(renderer),
        m_compass(NULL) {
            setToolBox(toolBox);
            toolBox.addWindow(this);
            bindEvents();
            bindObservers();
            updateAcceleratorTable(HasFocus());
        }

        void MapViewBase::setCompass(Renderer::Compass* compass) {
            m_compass = compass;
        }
        
        MapViewBase::~MapViewBase() {
            m_toolBox.removeWindow(this);
            unbindObservers();
            m_animationManager->Delete();
            delete m_compass;
        }

        void MapViewBase::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->nodesWereAddedNotifier.addObserver(this, &MapViewBase::nodesDidChange);
            document->nodesWereRemovedNotifier.addObserver(this, &MapViewBase::nodesDidChange);
            document->nodesDidChangeNotifier.addObserver(this, &MapViewBase::nodesDidChange);
            document->nodeVisibilityDidChangeNotifier.addObserver(this, &MapViewBase::nodesDidChange);
            document->nodeLockingDidChangeNotifier.addObserver(this, &MapViewBase::nodesDidChange);
            document->commandDoneNotifier.addObserver(this, &MapViewBase::commandDone);
            document->commandUndoneNotifier.addObserver(this, &MapViewBase::commandUndone);
            document->selectionDidChangeNotifier.addObserver(this, &MapViewBase::selectionDidChange);
            document->textureCollectionsDidChangeNotifier.addObserver(this, &MapViewBase::textureCollectionsDidChange);
            document->entityDefinitionsDidChangeNotifier.addObserver(this, &MapViewBase::entityDefinitionsDidChange);
            document->modsDidChangeNotifier.addObserver(this, &MapViewBase::modsDidChange);
            document->editorContextDidChangeNotifier.addObserver(this, &MapViewBase::editorContextDidChange);
            document->mapViewConfigDidChangeNotifier.addObserver(this, &MapViewBase::mapViewConfigDidChange);
			document->documentWasNewedNotifier.addObserver(this, &MapViewBase::documentDidChange);
			document->documentWasClearedNotifier.addObserver(this, &MapViewBase::documentDidChange);
			document->documentWasLoadedNotifier.addObserver(this, &MapViewBase::documentDidChange);

            Grid& grid = document->grid();
            grid.gridDidChangeNotifier.addObserver(this, &MapViewBase::gridDidChange);

            m_toolBox.toolActivatedNotifier.addObserver(this, &MapViewBase::toolChanged);
            m_toolBox.toolDeactivatedNotifier.addObserver(this, &MapViewBase::toolChanged);

            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &MapViewBase::preferenceDidChange);
        }

        void MapViewBase::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->nodesWereAddedNotifier.removeObserver(this, &MapViewBase::nodesDidChange);
                document->nodesWereRemovedNotifier.removeObserver(this, &MapViewBase::nodesDidChange);
                document->nodesDidChangeNotifier.removeObserver(this, &MapViewBase::nodesDidChange);
                document->nodeVisibilityDidChangeNotifier.removeObserver(this, &MapViewBase::nodesDidChange);
                document->nodeLockingDidChangeNotifier.removeObserver(this, &MapViewBase::nodesDidChange);
                document->commandDoneNotifier.removeObserver(this, &MapViewBase::commandDone);
                document->commandUndoneNotifier.removeObserver(this, &MapViewBase::commandUndone);
                document->selectionDidChangeNotifier.removeObserver(this, &MapViewBase::selectionDidChange);
                document->textureCollectionsDidChangeNotifier.removeObserver(this, &MapViewBase::textureCollectionsDidChange);
                document->entityDefinitionsDidChangeNotifier.removeObserver(this, &MapViewBase::entityDefinitionsDidChange);
                document->modsDidChangeNotifier.removeObserver(this, &MapViewBase::modsDidChange);
                document->editorContextDidChangeNotifier.removeObserver(this, &MapViewBase::editorContextDidChange);
                document->mapViewConfigDidChangeNotifier.removeObserver(this, &MapViewBase::mapViewConfigDidChange);
				document->documentWasNewedNotifier.removeObserver(this, &MapViewBase::documentDidChange);
				document->documentWasClearedNotifier.removeObserver(this, &MapViewBase::documentDidChange);
				document->documentWasLoadedNotifier.removeObserver(this, &MapViewBase::documentDidChange);

                Grid& grid = document->grid();
                grid.gridDidChangeNotifier.removeObserver(this, &MapViewBase::gridDidChange);
            }

            m_toolBox.toolActivatedNotifier.removeObserver(this, &MapViewBase::toolChanged);
            m_toolBox.toolDeactivatedNotifier.removeObserver(this, &MapViewBase::toolChanged);

            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &MapViewBase::preferenceDidChange);
        }

        void MapViewBase::nodesDidChange(const Model::NodeList& nodes) {
            updatePickResult();
            Refresh();
        }

        void MapViewBase::toolChanged(Tool* tool) {
            updatePickResult();
            updateAcceleratorTable(HasFocus());
            Refresh();
        }

        void MapViewBase::commandDone(Command::Ptr command) {
            updatePickResult();
            Refresh();
        }

        void MapViewBase::commandUndone(UndoableCommand::Ptr command) {
            updatePickResult();
            Refresh();
        }
        
        void MapViewBase::selectionDidChange(const Selection& selection) {
            updateAcceleratorTable(HasFocus());
        }

        void MapViewBase::textureCollectionsDidChange() {
            Refresh();
        }

        void MapViewBase::entityDefinitionsDidChange() {
            Refresh();
        }

        void MapViewBase::modsDidChange() {
            Refresh();
        }

        void MapViewBase::editorContextDidChange() {
            Refresh();
        }

        void MapViewBase::mapViewConfigDidChange() {
            Refresh();
        }

        void MapViewBase::gridDidChange() {
            Refresh();
        }

        void MapViewBase::preferenceDidChange(const IO::Path& path) {
            Refresh();
        }

		void MapViewBase::documentDidChange(MapDocument* document) {
			updatePickResult();
			Refresh();
		}

		void MapViewBase::bindEvents() {
            Bind(wxEVT_SET_FOCUS, &MapViewBase::OnSetFocus, this);
            Bind(wxEVT_KILL_FOCUS, &MapViewBase::OnKillFocus, this);

            Bind(wxEVT_MENU, &MapViewBase::OnToggleClipSide,               this, CommandIds::Actions::ToggleClipSide);
            Bind(wxEVT_MENU, &MapViewBase::OnPerformClip,                  this, CommandIds::Actions::PerformClip);
            Bind(wxEVT_MENU, &MapViewBase::OnRemoveLastClipPoint,          this, CommandIds::Actions::RemoveLastClipPoint);

            Bind(wxEVT_MENU, &MapViewBase::OnMoveVerticesForward,          this, CommandIds::Actions::MoveVerticesForward);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveVerticesBackward,         this, CommandIds::Actions::MoveVerticesBackward);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveVerticesLeft,             this, CommandIds::Actions::MoveVerticesLeft);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveVerticesRight,            this, CommandIds::Actions::MoveVerticesRight);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveVerticesUp,               this, CommandIds::Actions::MoveVerticesUp);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveVerticesDown,             this, CommandIds::Actions::MoveVerticesDown);
            
            Bind(wxEVT_MENU, &MapViewBase::OnMoveObjectsForward,           this, CommandIds::Actions::MoveObjectsForward);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveObjectsBackward,          this, CommandIds::Actions::MoveObjectsBackward);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveObjectsLeft,              this, CommandIds::Actions::MoveObjectsLeft);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveObjectsRight,             this, CommandIds::Actions::MoveObjectsRight);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveObjectsUp,                this, CommandIds::Actions::MoveObjectsUp);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveObjectsDown,              this, CommandIds::Actions::MoveObjectsDown);

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

            Bind(wxEVT_MENU, &MapViewBase::OnMoveRotationCenterForward,    this, CommandIds::Actions::MoveRotationCenterForward);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveRotationCenterBackward,   this, CommandIds::Actions::MoveRotationCenterBackward);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveRotationCenterLeft,       this, CommandIds::Actions::MoveRotationCenterLeft);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveRotationCenterRight,      this, CommandIds::Actions::MoveRotationCenterRight);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveRotationCenterUp,         this, CommandIds::Actions::MoveRotationCenterUp);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveRotationCenterDown,       this, CommandIds::Actions::MoveRotationCenterDown);

            Bind(wxEVT_MENU, &MapViewBase::OnCancel,                       this, CommandIds::Actions::Cancel);
            Bind(wxEVT_MENU, &MapViewBase::OnDeactivateTool,               this, CommandIds::Actions::DeactivateTool);
            
            Bind(wxEVT_MENU, &MapViewBase::OnGroupSelectedObjects,         this, CommandIds::MapViewPopupMenu::GroupObjects);
            Bind(wxEVT_MENU, &MapViewBase::OnUngroupSelectedObjects,       this, CommandIds::MapViewPopupMenu::UngroupObjects);
            Bind(wxEVT_MENU, &MapViewBase::OnRenameGroups,                 this, CommandIds::MapViewPopupMenu::RenameGroups);
            Bind(wxEVT_MENU, &MapViewBase::OnReparentBrushes,              this, CommandIds::MapViewPopupMenu::ReparentBrushes);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveBrushesToWorld,           this, CommandIds::MapViewPopupMenu::MoveBrushesToWorld);
            Bind(wxEVT_MENU, &MapViewBase::OnCreatePointEntity,            this, CommandIds::MapViewPopupMenu::LowestPointEntityItem, CommandIds::MapViewPopupMenu::HighestPointEntityItem);
            Bind(wxEVT_MENU, &MapViewBase::OnCreateBrushEntity,            this, CommandIds::MapViewPopupMenu::LowestBrushEntityItem, CommandIds::MapViewPopupMenu::HighestBrushEntityItem);
            
            Bind(wxEVT_UPDATE_UI, &MapViewBase::OnUpdatePopupMenuItem,     this, CommandIds::MapViewPopupMenu::GroupObjects);
            Bind(wxEVT_UPDATE_UI, &MapViewBase::OnUpdatePopupMenuItem,     this, CommandIds::MapViewPopupMenu::UngroupObjects);
            Bind(wxEVT_UPDATE_UI, &MapViewBase::OnUpdatePopupMenuItem,     this, CommandIds::MapViewPopupMenu::RenameGroups);
            Bind(wxEVT_UPDATE_UI, &MapViewBase::OnUpdatePopupMenuItem,     this, CommandIds::MapViewPopupMenu::ReparentBrushes);
            Bind(wxEVT_UPDATE_UI, &MapViewBase::OnUpdatePopupMenuItem,     this, CommandIds::MapViewPopupMenu::MoveBrushesToWorld);
            Bind(wxEVT_UPDATE_UI, &MapViewBase::OnUpdatePopupMenuItem,     this, CommandIds::MapViewPopupMenu::LowestPointEntityItem, CommandIds::MapViewPopupMenu::HighestPointEntityItem);
            Bind(wxEVT_UPDATE_UI, &MapViewBase::OnUpdatePopupMenuItem,     this, CommandIds::MapViewPopupMenu::LowestBrushEntityItem, CommandIds::MapViewPopupMenu::HighestBrushEntityItem);

            wxFrame* frame = findFrame(this);
            frame->Bind(wxEVT_ACTIVATE, &MapViewBase::OnActivateFrame, this);
        }

        void MapViewBase::OnMoveObjectsForward(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveObjects(Math::Direction_Forward);
        }

        void MapViewBase::OnMoveObjectsBackward(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveObjects(Math::Direction_Backward);
        }

        void MapViewBase::OnMoveObjectsLeft(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveObjects(Math::Direction_Left);
        }

        void MapViewBase::OnMoveObjectsRight(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveObjects(Math::Direction_Right);
        }

        void MapViewBase::OnMoveObjectsUp(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveObjects(Math::Direction_Up);
        }

        void MapViewBase::OnMoveObjectsDown(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveObjects(Math::Direction_Down);
        }

        void MapViewBase::OnDuplicateObjectsForward(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            duplicateAndMoveObjects(Math::Direction_Forward);
        }

        void MapViewBase::OnDuplicateObjectsBackward(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            duplicateAndMoveObjects(Math::Direction_Backward);
        }

        void MapViewBase::OnDuplicateObjectsLeft(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            duplicateAndMoveObjects(Math::Direction_Left);
        }

        void MapViewBase::OnDuplicateObjectsRight(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            duplicateAndMoveObjects(Math::Direction_Right);
        }

        void MapViewBase::OnDuplicateObjectsUp(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            duplicateAndMoveObjects(Math::Direction_Up);
        }

        void MapViewBase::OnDuplicateObjectsDown(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            duplicateAndMoveObjects(Math::Direction_Down);
        }

        void MapViewBase::OnRollObjectsCW(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            rotateObjects(Math::RotationAxis_Roll, true);
        }

        void MapViewBase::OnRollObjectsCCW(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            rotateObjects(Math::RotationAxis_Roll, false);
        }

        void MapViewBase::OnPitchObjectsCW(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            rotateObjects(Math::RotationAxis_Pitch, true);
        }

        void MapViewBase::OnPitchObjectsCCW(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            rotateObjects(Math::RotationAxis_Pitch, false);
        }

        void MapViewBase::OnYawObjectsCW(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            rotateObjects(Math::RotationAxis_Yaw, true);
        }

        void MapViewBase::OnYawObjectsCCW(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            rotateObjects(Math::RotationAxis_Yaw, false);
        }

        void MapViewBase::OnFlipObjectsH(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            flipObjects(Math::Direction_Left);
        }

        void MapViewBase::OnFlipObjectsV(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

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
                    axis = moveDirection(Math::Direction_Up);
                    break;
                    switchDefault()
            }

            if (clockwise)
                axis = -axis;
            return axis;
        }

        void MapViewBase::OnToggleRotateObjectsTool(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            m_toolBox.toggleRotateObjectsTool();
        }

        void MapViewBase::OnMoveRotationCenterForward(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveRotationCenter(Math::Direction_Forward);
        }

        void MapViewBase::OnMoveRotationCenterBackward(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveRotationCenter(Math::Direction_Backward);
        }

        void MapViewBase::OnMoveRotationCenterLeft(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveRotationCenter(Math::Direction_Left);
        }

        void MapViewBase::OnMoveRotationCenterRight(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveRotationCenter(Math::Direction_Right);
        }

        void MapViewBase::OnMoveRotationCenterUp(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveRotationCenter(Math::Direction_Up);
        }

        void MapViewBase::OnMoveRotationCenterDown(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveRotationCenter(Math::Direction_Down);
        }

        void MapViewBase::moveRotationCenter(const Math::Direction direction) {
            MapDocumentSPtr document = lock(m_document);
            const Grid& grid = document->grid();
            const Vec3 delta = moveDirection(direction) * static_cast<FloatType>(grid.actualSize());
            m_toolBox.moveRotationCenter(delta);
            Refresh();
        }

        void MapViewBase::OnToggleClipSide(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            m_toolBox.toggleClipSide();
        }

        void MapViewBase::OnPerformClip(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            m_toolBox.performClip();
        }

        void MapViewBase::OnRemoveLastClipPoint(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            m_toolBox.removeLastClipPoint();
        }

        void MapViewBase::OnMoveVerticesForward(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveVertices(Math::Direction_Forward);
        }
        
        void MapViewBase::OnMoveVerticesBackward(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveVertices(Math::Direction_Backward);
        }
        
        void MapViewBase::OnMoveVerticesLeft(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveVertices(Math::Direction_Left);
        }
        
        void MapViewBase::OnMoveVerticesRight(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveVertices(Math::Direction_Right);
        }
        
        void MapViewBase::OnMoveVerticesUp(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveVertices(Math::Direction_Up);
        }
        
        void MapViewBase::OnMoveVerticesDown(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveVertices(Math::Direction_Down);
        }

        void MapViewBase::moveVertices(const Math::Direction direction) {
            MapDocumentSPtr document = lock(m_document);
            const Grid& grid = document->grid();
            const Vec3 delta = moveDirection(direction) * static_cast<FloatType>(grid.actualSize());
            m_toolBox.moveVertices(delta);
        }

        void MapViewBase::OnCancel(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (MapViewBase::cancel())
                return;
            if (ToolBoxConnector::cancel())
                return;
            
            MapDocumentSPtr document = lock(m_document);
            if (document->hasSelection()) {
                document->deselectAll();
            } else if (document->currentGroup() != NULL) {
                document->closeGroup();
            }
        }

        bool MapViewBase::cancel() {
            return doCancel();
        }

        void MapViewBase::OnDeactivateTool(wxCommandEvent& event) {
            m_toolBox.deactivateAllTools();
        }
        
        void MapViewBase::OnGroupSelectedObjects(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            if (document->hasSelectedNodes()) {
                const String name = queryGroupName(this);
                if (!name.empty())
                    document->groupSelection(name);
            }
        }
        
        void MapViewBase::OnUngroupSelectedObjects(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            if (document->hasSelectedNodes() && document->selectedNodes().hasOnlyGroups())
                document->ungroupSelection();
        }

        void MapViewBase::OnRenameGroups(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            assert(document->selectedNodes().hasOnlyGroups());
            const String name = queryGroupName(this);
            if (!name.empty())
                document->renameGroups(name);
        }

        void MapViewBase::OnMoveBrushesToWorld(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            const Model::NodeList& nodes = document->selectedNodes().nodes();
            reparentNodes(nodes, document->currentParent());
        }
        
        void MapViewBase::OnCreatePointEntity(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            const size_t index = static_cast<size_t>(event.GetId() - CommandIds::MapViewPopupMenu::LowestPointEntityItem);
            const Assets::EntityDefinition* definition = findEntityDefinition(Assets::EntityDefinition::Type_PointEntity, index);
            assert(definition != NULL);
            assert(definition->type() == Assets::EntityDefinition::Type_PointEntity);
            createPointEntity(static_cast<const Assets::PointEntityDefinition*>(definition));
        }
        
        void MapViewBase::OnCreateBrushEntity(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            const size_t index = static_cast<size_t>(event.GetId() - CommandIds::MapViewPopupMenu::LowestBrushEntityItem);
            const Assets::EntityDefinition* definition = findEntityDefinition(Assets::EntityDefinition::Type_BrushEntity, index);
            assert(definition != NULL);
            assert(definition->type() == Assets::EntityDefinition::Type_BrushEntity);
            createBrushEntity(static_cast<const Assets::BrushEntityDefinition*>(definition));
        }
        
        Assets::EntityDefinition* MapViewBase::findEntityDefinition(const Assets::EntityDefinition::Type type, const size_t index) const {
            size_t count = 0;
            const Assets::EntityDefinitionGroup::List& groups = lock(m_document)->entityDefinitionManager().groups();
            Assets::EntityDefinitionGroup::List::const_iterator groupIt, groupEnd;
            for (groupIt = groups.begin(), groupEnd = groups.end(); groupIt != groupEnd; ++groupIt) {
                const Assets::EntityDefinitionGroup& group = *groupIt;
                const Assets::EntityDefinitionList definitions = group.definitions(type, Assets::EntityDefinition::Name);
                if (index < count + definitions.size())
                    return definitions[index - count];
                count += definitions.size();
            }
            return NULL;
        }
        
        void MapViewBase::createPointEntity(const Assets::PointEntityDefinition* definition) {
            assert(definition != NULL);
            
            MapDocumentSPtr document = lock(m_document);
            Model::Entity* entity = document->world()->createEntity();
            entity->addOrUpdateAttribute(Model::AttributeNames::Classname, definition->name());
            
            StringStream name;
            name << "Create " << definition->name();
            
            const Vec3 delta = doComputePointEntityPosition(definition->bounds());
            
            const Transaction transaction(document, name.str());
            document->deselectAll();
            document->addNode(entity, document->currentParent());
            document->select(entity);
            document->translateObjects(delta);
        }
        
        void MapViewBase::createBrushEntity(const Assets::BrushEntityDefinition* definition) {
            assert(definition != NULL);
            
            MapDocumentSPtr document = lock(m_document);
            
            const Model::BrushList brushes = document->selectedNodes().brushes();
            assert(!brushes.empty());
            
            // if all brushes belong to the same entity, and that entity is not worldspawn, copy its properties
            Model::BrushList::const_iterator it = brushes.begin();
            Model::BrushList::const_iterator end = brushes.end();
            Model::AttributableNode* entityTemplate = (*it++)->entity();
            while (it != end && entityTemplate != NULL)
                if ((*it++)->parent() != entityTemplate)
                    entityTemplate = NULL;
            
            Model::Entity* entity = document->world()->createEntity();
            if (entityTemplate != NULL && entityTemplate != document->world())
                entity->setAttributes(entityTemplate->attributes());
            entity->addOrUpdateAttribute(Model::AttributeNames::Classname, definition->name());
            
            StringStream name;
            name << "Create " << definition->name();
            
            const Model::NodeList nodes(brushes.begin(), brushes.end());
            
            const Transaction transaction(document, name.str());
            document->deselectAll();
            document->addNode(entity, document->currentParent());
            document->reparentNodes(entity, nodes);
            document->select(nodes);
        }
        
        bool MapViewBase::canCreateBrushEntity() {
            MapDocumentSPtr document = lock(m_document);
            return document->selectedNodes().hasOnlyBrushes();
        }

        void MapViewBase::OnSetFocus(wxFocusEvent& event) {
            if (IsBeingDeleted()) return;

            updateAcceleratorTable(true);
            event.Skip();
        }

        void MapViewBase::OnKillFocus(wxFocusEvent& event) {
            if (IsBeingDeleted()) return;

            updateAcceleratorTable(false);
            event.Skip();
        }

        void MapViewBase::OnActivateFrame(wxActivateEvent& event) {
            if (IsBeingDeleted()) return;

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

            if (m_toolBox.createComplexBrushToolActive())
                return ActionContext_CreateComplexBrushTool;
            if (m_toolBox.clipToolActive())
                return ActionContext_ClipTool;
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

        void MapViewBase::doFlashSelection() {
            FlashSelectionAnimation* animation = new FlashSelectionAnimation(m_renderer, *this, 180);
            m_animationManager->runAnimation(animation, true);
        }

        bool MapViewBase::doGetIsCurrent() const {
            return HasFocus();
        }
        
        void MapViewBase::doSetToolBoxDropTarget() {
            SetDropTarget(new ToolBoxDropTarget(this));
        }
        
        void MapViewBase::doClearDropTarget() {
            SetDropTarget(NULL);
        }

        bool MapViewBase::doCanFlipObjects() const {
            MapDocumentSPtr document = lock(m_document);
            return !m_toolBox.anyToolActive() && document->hasSelectedNodes();
        }

        void MapViewBase::doFlipObjects(const Math::Direction direction) {
            MapDocumentSPtr document = lock(m_document);
            if (!document->hasSelectedNodes())
                return;
            
            const Grid& grid = document->grid();
            const Vec3 center = grid.referencePoint(document->selectionBounds());
            const Math::Axis::Type axis = moveDirection(direction).firstComponent();
            
            document->flipObjects(center, axis);
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

            Renderer::RenderBatch renderBatch(vertexVbo(), indexVbo());

            doRenderGrid(renderContext, renderBatch);
            doRenderMap(m_renderer, renderContext, renderBatch);
            doRenderTools(m_toolBox, renderContext, renderBatch);
            doRenderExtras(renderContext, renderBatch);
            renderCoordinateSystem(renderContext, renderBatch);
            renderCompass(renderBatch);
            
            renderBatch.render(renderContext);
        }

        Renderer::RenderContext MapViewBase::createRenderContext() {
            MapDocumentSPtr document = lock(m_document);
            const MapViewConfig& mapViewConfig = document->mapViewConfig();
            const Grid& grid = document->grid();

            Renderer::RenderContext renderContext = doCreateRenderContext();
            renderContext.setShowTextures(mapViewConfig.showTextures());
            renderContext.setShowFaces(mapViewConfig.showFaces());
            renderContext.setShowEdges(mapViewConfig.showEdges());
            renderContext.setShadeFaces(mapViewConfig.shadeFaces());
            renderContext.setShowPointEntities(mapViewConfig.showPointEntities());
            renderContext.setShowPointEntityModels(mapViewConfig.showPointEntityModels());
            renderContext.setShowEntityClassnames(mapViewConfig.showEntityClassnames());
            renderContext.setShowEntityBounds(mapViewConfig.showEntityBounds());
            renderContext.setShowFog(mapViewConfig.showFog());
            renderContext.setShowGrid(grid.visible());
            renderContext.setGridSize(grid.actualSize());
            return renderContext;
        }

        void MapViewBase::setupGL(Renderer::RenderContext& context) {
            const Renderer::Camera::Viewport& viewport = context.camera().unzoomedViewport();
            glAssert(glViewport(viewport.x, viewport.y, viewport.width, viewport.height));

            glAssert(glEnable(GL_MULTISAMPLE));
            glAssert(glEnable(GL_BLEND));
            glAssert(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
            glAssert(glShadeModel(GL_SMOOTH));
        }

        void MapViewBase::renderCoordinateSystem(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            if (pref(Preferences::ShowAxes)) {
                MapDocumentSPtr document = lock(m_document);
                const BBox3& worldBounds = document->worldBounds();
                
                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.renderCoordinateSystem(worldBounds);
            }
        }

        void MapViewBase::renderCompass(Renderer::RenderBatch& renderBatch) {
            if (m_compass != NULL)
                m_compass->render(renderBatch);
        }

        void MapViewBase::doShowPopupMenu() {
            wxMenu menu;
            menu.SetEventHandler(this);
            menu.Append(CommandIds::MapViewPopupMenu::GroupObjects, "Group");
            menu.Append(CommandIds::MapViewPopupMenu::UngroupObjects, "Ungroup");
            menu.Append(CommandIds::MapViewPopupMenu::RenameGroups, "Rename");
            menu.AppendSeparator();
            menu.Append(CommandIds::MapViewPopupMenu::ReparentBrushes, "Move Brushes to...");
            menu.Append(CommandIds::MapViewPopupMenu::MoveBrushesToWorld, "Move Brushes to World");
            menu.AppendSeparator();
            menu.AppendSubMenu(makeEntityGroupsMenu(Assets::EntityDefinition::Type_PointEntity, CommandIds::MapViewPopupMenu::LowestPointEntityItem), "Create Point Entity");
            menu.AppendSubMenu(makeEntityGroupsMenu(Assets::EntityDefinition::Type_BrushEntity, CommandIds::MapViewPopupMenu::LowestBrushEntityItem), "Create Brush Entity");
            
            menu.UpdateUI(this);
            PopupMenu(&menu);
            
            doAfterPopupMenu();
        }

        wxMenu* MapViewBase::makeEntityGroupsMenu(const Assets::EntityDefinition::Type type, int id) {
            wxMenu* menu = new wxMenu();
            
            MapDocumentSPtr document = lock(m_document);
            const Assets::EntityDefinitionGroup::List& groups = document->entityDefinitionManager().groups();
            Assets::EntityDefinitionGroup::List::const_iterator groupIt, groupEnd;
            for (groupIt = groups.begin(), groupEnd = groups.end(); groupIt != groupEnd; ++groupIt) {
                const Assets::EntityDefinitionGroup& group = *groupIt;
                const Assets::EntityDefinitionList definitions = group.definitions(type, Assets::EntityDefinition::Name);

                Assets::EntityDefinitionList filteredDefinitions;
                filteredDefinitions.reserve(definitions.size());
                
                Assets::EntityDefinitionList::const_iterator dIt, dEnd;
                for (dIt = definitions.begin(), dEnd = definitions.end(); dIt != dEnd; ++dIt) {
                    Assets::EntityDefinition* definition = *dIt;
                    if (definition->name() != Model::AttributeValues::WorldspawnClassname)
                        filteredDefinitions.push_back(definition);
                }

                if (!filteredDefinitions.empty()) {
                    const String groupName = group.displayName();
                    wxMenu* groupMenu = new wxMenu();
                    groupMenu->SetEventHandler(this);
                    
                    for (dIt = filteredDefinitions.begin(), dEnd = filteredDefinitions.end(); dIt != dEnd; ++dIt) {
                        const Assets::EntityDefinition* definition = *dIt;
                        groupMenu->Append(id++, definition->shortName());
                    }
                    
                    menu->AppendSubMenu(groupMenu, groupName);
                }
            }
            
            return menu;
        }

        void MapViewBase::OnReparentBrushes(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            const Model::NodeList& nodes = document->selectedNodes().nodes();
            Model::Node* newParent = findNewNodeParent(nodes);
            assert(newParent != NULL);
            
            reparentNodes(nodes, newParent);
        }
        
        Model::Node* MapViewBase::findNewNodeParent(const Model::NodeList& nodes) const {
            Model::Node* newParent = NULL;
            
            MapDocumentSPtr document = lock(m_document);
            const Model::Hit& hit = pickResult().query().pickable().type(Model::Entity::EntityHit | Model::Brush::BrushHit).occluded().first();
            if (hit.isMatch()) {
                if (hit.type() == Model::Entity::EntityHit) {
                    newParent = Model::hitToEntity(hit);
                } else if (hit.type() == Model::Brush::BrushHit) {
                    const Model::Brush* brush = Model::hitToBrush(hit);
                    newParent = brush->parent();
                }
            }
            
            if (newParent != NULL && canReparentNodes(nodes, newParent))
                return newParent;
            return NULL;
        }

        
        
        bool MapViewBase::canReparentNodes(const Model::NodeList& nodes, const Model::Node* newParent) const {
            Model::NodeList::const_iterator it, end;
            for (it = nodes.begin(), end = nodes.end(); it != end; ++it) {
                const Model::Node* node = *it;
                if (node->parent() != newParent)
                    return true;
            }
            return false;
        }

        void MapViewBase::reparentNodes(const Model::NodeList& nodes, Model::Node* newParent) {
            assert(newParent != NULL);
            
            const Model::NodeList reparentableNodes = collectReparentableNodes(nodes, newParent);
            assert(!reparentableNodes.empty());

            MapDocumentSPtr document = lock(m_document);
            
            StringStream name;
            name << "Move " << (reparentableNodes.size() == 1 ? "Brush" : "Brushes") << " to " << newParent->name();
            
            const Transaction transaction(document, name.str());
            document->deselectAll();
            document->reparentNodes(newParent, reparentableNodes);
            document->select(reparentableNodes);
        }

        Model::NodeList MapViewBase::collectReparentableNodes(const Model::NodeList& nodes, const Model::Node* newParent) const {
            Model::NodeList result;
            Model::NodeList::const_iterator it, end;
            for (it = nodes.begin(), end = nodes.end(); it != end; ++it) {
                Model::Node* node = *it;
                if (node->parent() != newParent)
                    result.push_back(node);
            }
            return result;
        }

        void MapViewBase::OnUpdatePopupMenuItem(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

            switch (event.GetId()) {
                case CommandIds::MapViewPopupMenu::GroupObjects:
                    updateGroupObjectsMenuItem(event);
                    break;
                case CommandIds::MapViewPopupMenu::UngroupObjects:
                    updateUngroupObjectsMenuItem(event);
                    break;
                case CommandIds::MapViewPopupMenu::RenameGroups:
                    updateRenameGroupsMenuItem(event);
                    break;
                case CommandIds::MapViewPopupMenu::ReparentBrushes:
                    updateReparentBrushesMenuItem(event);
                    break;
                case CommandIds::MapViewPopupMenu::MoveBrushesToWorld:
                    updateMoveBrushesToWorldMenuItem(event);
                    break;
                default:
                    if (event.GetId() >= CommandIds::MapViewPopupMenu::LowestBrushEntityItem &&
                        event.GetId() <= CommandIds::MapViewPopupMenu::HighestBrushEntityItem) {
                        event.Enable(canCreateBrushEntity());
                    } else {
                        event.Enable(true);
                    }
                    break;
            }
        }

        void MapViewBase::updateGroupObjectsMenuItem(wxUpdateUIEvent& event) const {
            MapDocumentSPtr document = lock(m_document);
            event.Enable(!document->selectedNodes().empty());
        }
    
        void MapViewBase::updateUngroupObjectsMenuItem(wxUpdateUIEvent& event) const {
            MapDocumentSPtr document = lock(m_document);
            event.Enable(document->selectedNodes().hasOnlyGroups());
        }
        
        void MapViewBase::updateRenameGroupsMenuItem(wxUpdateUIEvent& event) const {
            MapDocumentSPtr document = lock(m_document);
            event.Enable(document->selectedNodes().hasOnlyGroups());
        }

        void MapViewBase::updateReparentBrushesMenuItem(wxUpdateUIEvent& event) const {
            MapDocumentSPtr document = lock(m_document);
            const Model::NodeList& nodes = document->selectedNodes().nodes();
            StringStream name;
            name << "Move " << StringUtils::safePlural(nodes.size(), "Brush", "Brushes") << " to ";
            
            if (!document->selectedNodes().hasOnlyBrushes()) {
                event.Enable(false);
                name << "Entity";
            } else {
                Model::Node* newParent = findNewNodeParent(nodes);
                if (newParent != NULL) {
                    event.Enable(true);
                    name << newParent->name();
                } else {
                    event.Enable(false);
                    name << "Entity";
                }
            }
            event.SetText(name.str());
        }
        
        void MapViewBase::updateMoveBrushesToWorldMenuItem(wxUpdateUIEvent& event) const {
            MapDocumentSPtr document = lock(m_document);
            Model::World* world = document->world();
            Model::Layer* layer = document->currentLayer();
            
            const Model::NodeList& nodes = document->selectedNodes().nodes();
            StringStream name;
            name << "Move " << StringUtils::safePlural(nodes.size(), "Brush", "Brushes") << " to " << world->name();
            event.Enable(canReparentNodes(nodes, layer));
            event.SetText(name.str());
        }

        void MapViewBase::doRenderExtras(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {}

        void MapViewBase::doAfterPopupMenu() {}
    }
}
