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

#include "MapViewBase.h"

#include "TrenchBroom.h"
#include "Logger.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Assets/EntityDefinitionManager.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/CollectMatchingNodesVisitor.h"
#include "Model/EditorContext.h"
#include "Model/Entity.h"
#include "Model/EntityAttributes.h"
#include "Model/FindGroupVisitor.h"
#include "Model/FindLayerVisitor.h"
#include "Model/Group.h"
#include "Model/Hit.h"
#include "Model/HitAdapter.h"
#include "Model/HitQuery.h"
#include "Model/Layer.h"
#include "Model/PointFile.h"
#include "Model/PortalFile.h"
#include "Model/PushSelection.h"
#include "Model/World.h"
#include "Renderer/Camera.h"
#include "Renderer/Compass.h"
#include "Renderer/FontDescriptor.h"
#include "Renderer/FontManager.h"
#include "Renderer/MapRenderer.h"
#include "Renderer/RenderBatch.h"
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
#include "View/SelectionTool.h"
#include "View/ToolBoxDropTarget.h"
#include "View/ViewUtils.h"
#include "View/wxUtils.h"

#include <vecmath/polygon.h>
#include <vecmath/util.h>

#include <wx/frame.h>
#include <wx/menu.h>

#include <algorithm>
#include <iterator>

wxDEFINE_EVENT(SHOW_POPUP_MENU_EVENT, wxCommandEvent);

namespace TrenchBroom {
    namespace View {
        static wxString GLVendor, GLRenderer, GLVersion;

        const wxString &MapViewBase::glRendererString() {
            return GLRenderer;
        }

        const wxString &MapViewBase::glVendorString() {
            return GLVendor;
        }

        const wxString &MapViewBase::glVersionString() {
            return GLVersion;
        }

        const wxLongLong MapViewBase::DefaultCameraAnimationDuration = 250;

        MapViewBase::MapViewBase(wxWindow* parent, Logger* logger, MapDocumentWPtr document, MapViewToolBox& toolBox, Renderer::MapRenderer& renderer, GLContextManager& contextManager) :
        RenderView(parent, contextManager, GLAttribs::attribs()),
        m_logger(logger),
        m_document(document),
        m_toolBox(toolBox),
        m_animationManager(new AnimationManager()),
        m_renderer(renderer),
        m_compass(nullptr),
        m_portalFileRenderer(nullptr) {
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
            document->pointFileWasLoadedNotifier.addObserver(this, &MapViewBase::pointFileDidChange);
            document->pointFileWasUnloadedNotifier.addObserver(this, &MapViewBase::pointFileDidChange);
            document->portalFileWasLoadedNotifier.addObserver(this, &MapViewBase::portalFileDidChange);
            document->portalFileWasUnloadedNotifier.addObserver(this, &MapViewBase::portalFileDidChange);

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
                document->pointFileWasLoadedNotifier.removeObserver(this, &MapViewBase::pointFileDidChange);
                document->pointFileWasUnloadedNotifier.removeObserver(this, &MapViewBase::pointFileDidChange);
                document->portalFileWasLoadedNotifier.removeObserver(this, &MapViewBase::portalFileDidChange);
                document->portalFileWasUnloadedNotifier.removeObserver(this, &MapViewBase::portalFileDidChange);

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

        void MapViewBase::pointFileDidChange() {
            Refresh();
        }

        void MapViewBase::portalFileDidChange() {
            invalidatePortalFileRenderer();
            Refresh();
        }

        void MapViewBase::preferenceDidChange(const IO::Path& path) {
            if(path == Preferences::RendererFontSize.path()) {
                fontManager().clearCache();
            }

            updateAcceleratorTable();
            Refresh();
        }

        void MapViewBase::documentDidChange(MapDocument* document) {
            updatePickResult();
            Refresh();
        }

        void MapViewBase::bindEvents() {
            Bind(wxEVT_SET_FOCUS, &MapViewBase::OnSetFocus,                       this);
            Bind(wxEVT_KILL_FOCUS, &MapViewBase::OnKillFocus,                     this);

            Bind(wxEVT_MENU, &MapViewBase::OnMakeStructural,                      this, CommandIds::Actions::MakeStructural);

            Bind(wxEVT_MENU, &MapViewBase::OnToggleTagVisible,                    this, CommandIds::Actions::LowestToggleTagCommandId, CommandIds::Actions::HighestToggleTagCommandId);
            Bind(wxEVT_MENU, &MapViewBase::OnEnableTag,                           this, CommandIds::Actions::LowestEnableTagCommandId, CommandIds::Actions::HighestEnableTagCommandId);
            Bind(wxEVT_MENU, &MapViewBase::OnDisableTag,                          this, CommandIds::Actions::LowestDisableTagCommandId, CommandIds::Actions::HighestDisableTagCommandId);

            Bind(wxEVT_MENU, &MapViewBase::OnToggleEntityDefinitionVisible,       this, CommandIds::Actions::LowestToggleEntityDefinitionCommandId, CommandIds::Actions::HighestToggleEntityDefinitionCommandId);
            Bind(wxEVT_MENU, &MapViewBase::OnCreateEntity,                        this, CommandIds::Actions::LowestCreateEntityCommandId, CommandIds::Actions::HighestCreateEntityCommandId);

            Bind(wxEVT_MENU, &MapViewBase::OnToggleShowEntityClassnames,          this, CommandIds::Actions::ToggleShowEntityClassnames);
            Bind(wxEVT_MENU, &MapViewBase::OnToggleShowGroupBounds,               this, CommandIds::Actions::ToggleShowGroupBounds);
            Bind(wxEVT_MENU, &MapViewBase::OnToggleShowBrushEntityBounds,         this, CommandIds::Actions::ToggleShowBrushEntityBounds);
            Bind(wxEVT_MENU, &MapViewBase::OnToggleShowPointEntityBounds,         this, CommandIds::Actions::ToggleShowPointEntityBounds);
            Bind(wxEVT_MENU, &MapViewBase::OnToggleShowPointEntities,             this, CommandIds::Actions::ToggleShowPointEntities);
            Bind(wxEVT_MENU, &MapViewBase::OnToggleShowPointEntities,             this, CommandIds::Actions::ToggleShowPointEntities);
            Bind(wxEVT_MENU, &MapViewBase::OnToggleShowPointEntityModels,         this, CommandIds::Actions::ToggleShowPointEntityModels);
            Bind(wxEVT_MENU, &MapViewBase::OnToggleShowBrushes,                   this, CommandIds::Actions::ToggleShowBrushes);
            Bind(wxEVT_MENU, &MapViewBase::OnRenderModeShowTextures,              this, CommandIds::Actions::RenderModeShowTextures);
            Bind(wxEVT_MENU, &MapViewBase::OnRenderModeHideTextures,              this, CommandIds::Actions::RenderModeHideTextures);
            Bind(wxEVT_MENU, &MapViewBase::OnRenderModeHideFaces,                 this, CommandIds::Actions::RenderModeHideFaces);
            Bind(wxEVT_MENU, &MapViewBase::OnRenderModeShadeFaces,                this, CommandIds::Actions::RenderModeShadeFaces);
            Bind(wxEVT_MENU, &MapViewBase::OnRenderModeUseFog,                    this, CommandIds::Actions::RenderModeUseFog);
            Bind(wxEVT_MENU, &MapViewBase::OnRenderModeShowEdges,                 this, CommandIds::Actions::RenderModeShowEdges);
            Bind(wxEVT_MENU, &MapViewBase::OnRenderModeShowAllEntityLinks,        this, CommandIds::Actions::RenderModeShowAllEntityLinks);
            Bind(wxEVT_MENU, &MapViewBase::OnRenderModeShowTransitiveEntityLinks, this, CommandIds::Actions::RenderModeShowTransitiveEntityLinks);
            Bind(wxEVT_MENU, &MapViewBase::OnRenderModeShowDirectEntityLinks,     this, CommandIds::Actions::RenderModeShowDirectEntityLinks);
            Bind(wxEVT_MENU, &MapViewBase::OnRenderModeHideEntityLinks,           this, CommandIds::Actions::RenderModeHideEntityLinks);

            Bind(wxEVT_MENU, &MapViewBase::OnToggleClipSide,                      this, CommandIds::Actions::ToggleClipSide);
            Bind(wxEVT_MENU, &MapViewBase::OnPerformClip,                         this, CommandIds::Actions::PerformClip);

            Bind(wxEVT_MENU, &MapViewBase::OnMoveVerticesForward,                 this, CommandIds::Actions::MoveVerticesForward);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveVerticesBackward,                this, CommandIds::Actions::MoveVerticesBackward);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveVerticesLeft,                    this, CommandIds::Actions::MoveVerticesLeft);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveVerticesRight,                   this, CommandIds::Actions::MoveVerticesRight);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveVerticesUp,                      this, CommandIds::Actions::MoveVerticesUp);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveVerticesDown,                    this, CommandIds::Actions::MoveVerticesDown);

            Bind(wxEVT_MENU, &MapViewBase::OnMoveObjectsForward,                  this, CommandIds::Actions::MoveObjectsForward);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveObjectsBackward,                 this, CommandIds::Actions::MoveObjectsBackward);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveObjectsLeft,                     this, CommandIds::Actions::MoveObjectsLeft);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveObjectsRight,                    this, CommandIds::Actions::MoveObjectsRight);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveObjectsUp,                       this, CommandIds::Actions::MoveObjectsUp);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveObjectsDown,                     this, CommandIds::Actions::MoveObjectsDown);

            Bind(wxEVT_MENU, &MapViewBase::OnDuplicateObjectsForward,             this, CommandIds::Actions::DuplicateObjectsForward);
            Bind(wxEVT_MENU, &MapViewBase::OnDuplicateObjectsBackward,            this, CommandIds::Actions::DuplicateObjectsBackward);
            Bind(wxEVT_MENU, &MapViewBase::OnDuplicateObjectsLeft,                this, CommandIds::Actions::DuplicateObjectsLeft);
            Bind(wxEVT_MENU, &MapViewBase::OnDuplicateObjectsRight,               this, CommandIds::Actions::DuplicateObjectsRight);
            Bind(wxEVT_MENU, &MapViewBase::OnDuplicateObjectsUp,                  this, CommandIds::Actions::DuplicateObjectsUp);
            Bind(wxEVT_MENU, &MapViewBase::OnDuplicateObjectsDown,                this, CommandIds::Actions::DuplicateObjectsDown);

            Bind(wxEVT_MENU, &MapViewBase::OnRollObjectsCW,                       this, CommandIds::Actions::RollObjectsCW);
            Bind(wxEVT_MENU, &MapViewBase::OnRollObjectsCCW,                      this, CommandIds::Actions::RollObjectsCCW);
            Bind(wxEVT_MENU, &MapViewBase::OnPitchObjectsCW,                      this, CommandIds::Actions::PitchObjectsCW);
            Bind(wxEVT_MENU, &MapViewBase::OnPitchObjectsCCW,                     this, CommandIds::Actions::PitchObjectsCCW);
            Bind(wxEVT_MENU, &MapViewBase::OnYawObjectsCW,                        this, CommandIds::Actions::YawObjectsCW);
            Bind(wxEVT_MENU, &MapViewBase::OnYawObjectsCCW,                       this, CommandIds::Actions::YawObjectsCCW);

            Bind(wxEVT_MENU, &MapViewBase::OnFlipObjectsH,                        this, CommandIds::Actions::FlipObjectsHorizontally);
            Bind(wxEVT_MENU, &MapViewBase::OnFlipObjectsV,                        this, CommandIds::Actions::FlipObjectsVertically);

            Bind(wxEVT_MENU, &MapViewBase::OnMoveRotationCenterForward,           this, CommandIds::Actions::MoveRotationCenterForward);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveRotationCenterBackward,          this, CommandIds::Actions::MoveRotationCenterBackward);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveRotationCenterLeft,              this, CommandIds::Actions::MoveRotationCenterLeft);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveRotationCenterRight,             this, CommandIds::Actions::MoveRotationCenterRight);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveRotationCenterUp,                this, CommandIds::Actions::MoveRotationCenterUp);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveRotationCenterDown,              this, CommandIds::Actions::MoveRotationCenterDown);

            Bind(wxEVT_MENU, &MapViewBase::OnCancel,                              this, CommandIds::Actions::Cancel);
            Bind(wxEVT_MENU, &MapViewBase::OnDeactivateTool,                      this, CommandIds::Actions::DeactivateTool);

            Bind(SHOW_POPUP_MENU_EVENT, &MapViewBase::OnShowPopupMenu,            this, CommandIds::MapViewPopupMenu::ShowPopupMenu);
            Bind(wxEVT_MENU, &MapViewBase::OnGroupSelectedObjects,                this, CommandIds::MapViewPopupMenu::GroupObjects);
            Bind(wxEVT_MENU, &MapViewBase::OnUngroupSelectedObjects,              this, CommandIds::MapViewPopupMenu::UngroupObjects);
            Bind(wxEVT_MENU, &MapViewBase::OnRenameGroups,                        this, CommandIds::MapViewPopupMenu::RenameGroups);
            Bind(wxEVT_MENU, &MapViewBase::OnAddObjectsToGroup,                   this, CommandIds::MapViewPopupMenu::AddObjectsToGroup);
            Bind(wxEVT_MENU, &MapViewBase::OnRemoveObjectsFromGroup,              this, CommandIds::MapViewPopupMenu::RemoveObjectsFromGroup);
            Bind(wxEVT_MENU, &MapViewBase::OnMergeGroups,                         this, CommandIds::MapViewPopupMenu::MergeGroups);
            Bind(wxEVT_MENU, &MapViewBase::OnMoveBrushesTo,                       this, CommandIds::MapViewPopupMenu::MoveBrushesToEntity);
            Bind(wxEVT_MENU, &MapViewBase::OnMakeStructural,                      this, CommandIds::MapViewPopupMenu::MakeStructural);
            Bind(wxEVT_MENU, &MapViewBase::OnCreatePointEntity,                   this, CommandIds::MapViewPopupMenu::LowestPointEntityItem, CommandIds::MapViewPopupMenu::HighestPointEntityItem);
            Bind(wxEVT_MENU, &MapViewBase::OnCreateBrushEntity,                   this, CommandIds::MapViewPopupMenu::LowestBrushEntityItem, CommandIds::MapViewPopupMenu::HighestBrushEntityItem);

            Bind(wxEVT_UPDATE_UI, &MapViewBase::OnUpdatePopupMenuItem,            this, CommandIds::MapViewPopupMenu::GroupObjects);
            Bind(wxEVT_UPDATE_UI, &MapViewBase::OnUpdatePopupMenuItem,            this, CommandIds::MapViewPopupMenu::UngroupObjects);
            Bind(wxEVT_UPDATE_UI, &MapViewBase::OnUpdatePopupMenuItem,            this, CommandIds::MapViewPopupMenu::MergeGroups);
            Bind(wxEVT_UPDATE_UI, &MapViewBase::OnUpdatePopupMenuItem,            this, CommandIds::MapViewPopupMenu::RenameGroups);
            Bind(wxEVT_UPDATE_UI, &MapViewBase::OnUpdatePopupMenuItem,            this, CommandIds::MapViewPopupMenu::MakeStructural);
            Bind(wxEVT_UPDATE_UI, &MapViewBase::OnUpdatePopupMenuItem,            this, CommandIds::MapViewPopupMenu::LowestPointEntityItem, CommandIds::MapViewPopupMenu::HighestPointEntityItem);
            Bind(wxEVT_UPDATE_UI, &MapViewBase::OnUpdatePopupMenuItem,            this, CommandIds::MapViewPopupMenu::LowestBrushEntityItem, CommandIds::MapViewPopupMenu::HighestBrushEntityItem);

            wxFrame* frame = findFrame(this);
            frame->Bind(wxEVT_ACTIVATE, &MapViewBase::OnActivateFrame,            this);
        }

        void MapViewBase::OnMoveObjectsForward(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveObjects(vm::direction::forward);
        }

        void MapViewBase::OnMoveObjectsBackward(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveObjects(vm::direction::backward);
        }

        void MapViewBase::OnMoveObjectsLeft(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveObjects(vm::direction::left);
        }

        void MapViewBase::OnMoveObjectsRight(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveObjects(vm::direction::right);
        }

        void MapViewBase::OnMoveObjectsUp(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveObjects(vm::direction::up);
        }

        void MapViewBase::OnMoveObjectsDown(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveObjects(vm::direction::down);
        }

        void MapViewBase::OnDuplicateObjectsForward(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            duplicateAndMoveObjects(vm::direction::forward);
        }

        void MapViewBase::OnDuplicateObjectsBackward(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            duplicateAndMoveObjects(vm::direction::backward);
        }

        void MapViewBase::OnDuplicateObjectsLeft(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            duplicateAndMoveObjects(vm::direction::left);
        }

        void MapViewBase::OnDuplicateObjectsRight(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            duplicateAndMoveObjects(vm::direction::right);
        }

        void MapViewBase::OnDuplicateObjectsUp(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            duplicateAndMoveObjects(vm::direction::up);
        }

        void MapViewBase::OnDuplicateObjectsDown(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            duplicateAndMoveObjects(vm::direction::down);
        }

        void MapViewBase::OnRollObjectsCW(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            rotateObjects(vm::rotation_axis::roll, true);
        }

        void MapViewBase::OnRollObjectsCCW(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            rotateObjects(vm::rotation_axis::roll, false);
        }

        void MapViewBase::OnPitchObjectsCW(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            rotateObjects(vm::rotation_axis::pitch, true);
        }

        void MapViewBase::OnPitchObjectsCCW(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            rotateObjects(vm::rotation_axis::pitch, false);
        }

        void MapViewBase::OnYawObjectsCW(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            rotateObjects(vm::rotation_axis::yaw, true);
        }

        void MapViewBase::OnYawObjectsCCW(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            rotateObjects(vm::rotation_axis::yaw, false);
        }

        void MapViewBase::OnFlipObjectsH(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            flipObjects(vm::direction::left);
        }

        void MapViewBase::OnFlipObjectsV(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            flipObjects(vm::direction::up);
        }

        void MapViewBase::duplicateAndMoveObjects(const vm::direction direction) {
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

        void MapViewBase::moveObjects(const vm::direction direction) {
            MapDocumentSPtr document = lock(m_document);
            if (!document->hasSelectedNodes())
                return;

            const Grid& grid = document->grid();
            const vm::vec3 delta = moveDirection(direction) * static_cast<FloatType>(grid.actualSize());
            document->translateObjects(delta);
        }

        vm::vec3 MapViewBase::moveDirection(const vm::direction direction) const {
            return doGetMoveDirection(direction);
        }

        void MapViewBase::rotateObjects(const vm::rotation_axis axisSpec, const bool clockwise) {
            MapDocumentSPtr document = lock(m_document);
            if (!document->hasSelectedNodes())
                return;

            const vm::vec3 axis = rotationAxis(axisSpec, clockwise);
            const double angle = m_toolBox.rotateObjectsToolActive() ? std::abs(m_toolBox.rotateToolAngle()) : vm::C::piOverTwo();

            const Grid& grid = document->grid();
            const vm::vec3 center = m_toolBox.rotateObjectsToolActive() ? m_toolBox.rotateToolCenter() : grid.referencePoint(document->selectionBounds());

            document->rotateObjects(center, axis, angle);
        }

        vm::vec3 MapViewBase::rotationAxis(const vm::rotation_axis axisSpec, const bool clockwise) const {
            vm::vec3 axis;
            switch (axisSpec) {
                case vm::rotation_axis::roll:
                    axis = -moveDirection(vm::direction::forward);
                    break;
                case vm::rotation_axis::pitch:
                    axis = moveDirection(vm::direction::right);
                    break;
                case vm::rotation_axis::yaw:
                    axis = moveDirection(vm::direction::up);
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

        void MapViewBase::OnToggleScaleObjectsTool(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            m_toolBox.toggleScaleObjectsTool();
        }

        void MapViewBase::OnToggleShearObjectsTool(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            m_toolBox.toggleShearObjectsTool();
        }

        void MapViewBase::OnMoveRotationCenterForward(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveRotationCenter(vm::direction::forward);
        }

        void MapViewBase::OnMoveRotationCenterBackward(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveRotationCenter(vm::direction::backward);
        }

        void MapViewBase::OnMoveRotationCenterLeft(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveRotationCenter(vm::direction::left);
        }

        void MapViewBase::OnMoveRotationCenterRight(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveRotationCenter(vm::direction::right);
        }

        void MapViewBase::OnMoveRotationCenterUp(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveRotationCenter(vm::direction::up);
        }

        void MapViewBase::OnMoveRotationCenterDown(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveRotationCenter(vm::direction::down);
        }

        void MapViewBase::moveRotationCenter(const vm::direction direction) {
            MapDocumentSPtr document = lock(m_document);
            const Grid& grid = document->grid();
            const vm::vec3 delta = moveDirection(direction) * static_cast<FloatType>(grid.actualSize());
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

        void MapViewBase::OnMoveVerticesForward(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveVertices(vm::direction::forward);
        }

        void MapViewBase::OnMoveVerticesBackward(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveVertices(vm::direction::backward);
        }

        void MapViewBase::OnMoveVerticesLeft(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveVertices(vm::direction::left);
        }

        void MapViewBase::OnMoveVerticesRight(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveVertices(vm::direction::right);
        }

        void MapViewBase::OnMoveVerticesUp(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveVertices(vm::direction::up);
        }

        void MapViewBase::OnMoveVerticesDown(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveVertices(vm::direction::down);
        }

        void MapViewBase::moveVertices(const vm::direction direction) {
            MapDocumentSPtr document = lock(m_document);
            const Grid& grid = document->grid();
            const vm::vec3 delta = moveDirection(direction) * static_cast<FloatType>(grid.actualSize());
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
            } else if (document->currentGroup() != nullptr) {
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

        void MapViewBase::OnCreatePointEntity(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            const size_t index = static_cast<size_t>(event.GetId() - CommandIds::MapViewPopupMenu::LowestPointEntityItem);
            const Assets::EntityDefinition* definition = findEntityDefinition(Assets::EntityDefinition::Type_PointEntity, index);
            ensure(definition != nullptr, "definition is null");
            assert(definition->type() == Assets::EntityDefinition::Type_PointEntity);
            createPointEntity(static_cast<const Assets::PointEntityDefinition*>(definition));
        }

        void MapViewBase::OnCreateBrushEntity(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            const size_t index = static_cast<size_t>(event.GetId() - CommandIds::MapViewPopupMenu::LowestBrushEntityItem);
            const Assets::EntityDefinition* definition = findEntityDefinition(Assets::EntityDefinition::Type_BrushEntity, index);
            ensure(definition != nullptr, "definition is null");
            assert(definition->type() == Assets::EntityDefinition::Type_BrushEntity);
            createBrushEntity(static_cast<const Assets::BrushEntityDefinition*>(definition));
        }

        Assets::EntityDefinition* MapViewBase::findEntityDefinition(const Assets::EntityDefinition::Type type, const size_t index) const {
            size_t count = 0;
            for (const Assets::EntityDefinitionGroup& group : lock(m_document)->entityDefinitionManager().groups()) {
                const Assets::EntityDefinitionList definitions = group.definitions(type, Assets::EntityDefinition::Name);
                if (index < count + definitions.size())
                    return definitions[index - count];
                count += definitions.size();
            }
            return nullptr;
        }

        void MapViewBase::createPointEntity(const Assets::PointEntityDefinition* definition) {
            ensure(definition != nullptr, "definition is null");

            auto document = lock(m_document);
            const auto delta = doComputePointEntityPosition(definition->bounds());
            document->createPointEntity(definition, delta);
        }

        void MapViewBase::createBrushEntity(const Assets::BrushEntityDefinition* definition) {
            ensure(definition != nullptr, "definition is null");

            auto document = lock(m_document);
            document->createBrushEntity(definition);
        }

        bool MapViewBase::canCreateBrushEntity() {
            auto document = lock(m_document);
            return document->selectedNodes().hasOnlyBrushes();
        }

        void MapViewBase::OnToggleTagVisible(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            const auto tagIndex = static_cast<size_t>((event.GetId() - CommandIds::Actions::LowestToggleTagCommandId));

            auto document = lock(m_document);
            auto& editorContext = document->editorContext();
            auto hiddenTags = editorContext.hiddenTags();
            hiddenTags ^= 1UL << tagIndex;
            editorContext.setHiddenTags(hiddenTags);
        }

        class MapViewBase::EnableDisableTagCallback : public Model::TagMatcherCallback, public wxEvtHandler {
        private:
            wxWindow* m_window;
            size_t m_selectedOption;
        public:
            explicit EnableDisableTagCallback(wxWindow* window) :
            m_window(window),
            m_selectedOption(0) {
                assert(m_window != nullptr);
            }

            size_t selectOption(const StringList& options) {
                wxMenu menu;
                for (size_t i = 0; i < options.size(); ++i) {
                    const auto& option = options[i];
                    const auto commandId = CommandIds::ToggleTagPopupMenu::Lowest + static_cast<int>(i);
                    menu.Append(commandId, option);
                    menu.Bind(wxEVT_MENU, &EnableDisableTagCallback::OnMenuItem, this, commandId);
                }


                m_selectedOption = options.size();
                m_window->PopupMenu(&menu);
                m_selectedOption = std::min(m_selectedOption, options.size());
                return m_selectedOption;
            }

            void OnMenuItem(wxCommandEvent& event) {
                m_selectedOption = static_cast<size_t>(event.GetId() - CommandIds::ToggleTagPopupMenu::Lowest);
            }
        };

        void MapViewBase::OnEnableTag(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            const auto tagIndex = static_cast<size_t>((event.GetId() - CommandIds::Actions::LowestEnableTagCommandId));

            auto document = lock(m_document);
            if (document->isRegisteredSmartTag(tagIndex)) {
                const auto& tag = document->smartTag(tagIndex);
                assert(tag.canEnable());

                Transaction transaction(document, "Turn Selection into " + tag.name());
                EnableDisableTagCallback callback(this);
                tag.enable(callback, *document);
            }
        }

        void MapViewBase::OnDisableTag(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            const auto tagIndex = static_cast<size_t>((event.GetId() - CommandIds::Actions::LowestDisableTagCommandId));

            auto document = lock(m_document);
            if (document->isRegisteredSmartTag(tagIndex)) {
                const auto& tag = document->smartTag(tagIndex);
                assert(tag.canDisable());

                Transaction transaction(document, "Turn Selection into non-" + tag.name());
                EnableDisableTagCallback callback(this);
                tag.disable(callback, *document);
            }
        }

        void MapViewBase::OnMakeStructural(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            auto document = lock(m_document);
            if (!document->selectedNodes().hasBrushes()) {
                return;
            }

            Transaction transaction(document, "Make Structural");
            Model::NodeList toReparent;
            for (auto* brush : document->selectedNodes().brushes()) {
                if (brush->entity() != document->world()) {
                    toReparent.push_back(brush);
                }
            }

            if (!toReparent.empty()) {
                reparentNodes(toReparent, document->currentParent(), false);
            }

            bool anyTagDisabled = false;
            EnableDisableTagCallback callback(this);
            for (auto* brush : document->selectedNodes().brushes()) {
                for (const auto& tag : document->smartTags()) {
                    if (brush->hasTag(tag) || brush->anyFacesHaveAnyTagInMask(tag.type())) {
                        anyTagDisabled = true;
                        tag.disable(callback, *document);
                    }
                }
            }

            if (!anyTagDisabled && toReparent.empty()) {
                transaction.cancel();
            }
        }

        void MapViewBase::OnToggleEntityDefinitionVisible(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            auto document = lock(m_document);
            const auto& definitions = document->entityDefinitionManager().definitions();

            const auto definitionIndex = static_cast<size_t>((event.GetId() - CommandIds::Actions::LowestToggleEntityDefinitionCommandId));
            if (definitionIndex >= definitions.size()) {
                return;
            }

            const auto* definition = definitions[definitionIndex];

            Model::EditorContext& editorContext = document->editorContext();
            editorContext.setEntityDefinitionHidden(definition, !editorContext.entityDefinitionHidden(definition));
        }

        void MapViewBase::OnCreateEntity(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            auto document = lock(m_document);
            const auto& definitions = document->entityDefinitionManager().definitions();

            const auto definitionIndex = static_cast<size_t>((event.GetId() - CommandIds::Actions::LowestCreateEntityCommandId));
            if (definitionIndex >= definitions.size()) {
                return;
            }

            const auto* definition = definitions[definitionIndex];
            if (definition->type() == Assets::EntityDefinition::Type_PointEntity) {
                createPointEntity(static_cast<const Assets::PointEntityDefinition*>(definition));
            } else if (canCreateBrushEntity()) {
                createBrushEntity(static_cast<const Assets::BrushEntityDefinition*>(definition));
            }
        }

        void MapViewBase::OnToggleShowEntityClassnames(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setShowEntityClassnames(!config.showEntityClassnames());
        }

        void MapViewBase::OnToggleShowGroupBounds(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setShowGroupBounds(!config.showGroupBounds());
        }

        void MapViewBase::OnToggleShowBrushEntityBounds(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setShowBrushEntityBounds(!config.showBrushEntityBounds());
        }

        void MapViewBase::OnToggleShowPointEntityBounds(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setShowPointEntityBounds(!config.showPointEntityBounds());
        }

        void MapViewBase::OnToggleShowPointEntities(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            Model::EditorContext& editorContext = document->editorContext();
            editorContext.setShowPointEntities(!editorContext.showPointEntities());
        }

        void MapViewBase::OnToggleShowPointEntityModels(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setShowPointEntityModels(!config.showPointEntityModels());
        }

        void MapViewBase::OnToggleShowBrushes(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            Model::EditorContext& editorContext = document->editorContext();
            editorContext.setShowBrushes(!editorContext.showBrushes());
        }

        void MapViewBase::OnRenderModeShowTextures(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setFaceRenderMode(MapViewConfig::FaceRenderMode_Textured);
        }

        void MapViewBase::OnRenderModeHideTextures(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setFaceRenderMode(MapViewConfig::FaceRenderMode_Flat);
        }

        void MapViewBase::OnRenderModeHideFaces(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setFaceRenderMode(MapViewConfig::FaceRenderMode_Skip);
        }

        void MapViewBase::OnRenderModeShadeFaces(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setShadeFaces(!config.shadeFaces());
        }

        void MapViewBase::OnRenderModeUseFog(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setShowFog(!config.showFog());
        }

        void MapViewBase::OnRenderModeShowEdges(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setShowEdges(!config.showEdges());
        }

        void MapViewBase::OnRenderModeShowAllEntityLinks(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            Model::EditorContext& editorContext = document->editorContext();
            editorContext.setEntityLinkMode(Model::EditorContext::EntityLinkMode_All);
        }

        void MapViewBase::OnRenderModeShowTransitiveEntityLinks(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            Model::EditorContext& editorContext = document->editorContext();
            editorContext.setEntityLinkMode(Model::EditorContext::EntityLinkMode_Transitive);
        }

        void MapViewBase::OnRenderModeShowDirectEntityLinks(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            Model::EditorContext& editorContext = document->editorContext();
            editorContext.setEntityLinkMode(Model::EditorContext::EntityLinkMode_Direct);
        }

        void MapViewBase::OnRenderModeHideEntityLinks(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            Model::EditorContext& editorContext = document->editorContext();
            editorContext.setEntityLinkMode(Model::EditorContext::EntityLinkMode_None);
        }

        void MapViewBase::OnSetFocus(wxFocusEvent& event) {
            if (IsBeingDeleted()) return;

            updateAcceleratorTable(true);
            updateModifierKeys();
            event.Skip();
        }

        void MapViewBase::OnKillFocus(wxFocusEvent& event) {
            if (IsBeingDeleted()) return;

            updateAcceleratorTable(false);
            clearModifierKeys();
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
            if (m_toolBox.anyVertexToolActive())
                return ActionContext_AnyVertexTool;
            if (m_toolBox.rotateObjectsToolActive())
                return ActionContext_RotateTool;
            if (m_toolBox.scaleObjectsToolActive())
                return ActionContext_ScaleTool;
            if (m_toolBox.shearObjectsToolActive())
                return ActionContext_ShearTool;

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
            SetDropTarget(new ToolBoxDropTarget(this, this));
        }

        void MapViewBase::doClearDropTarget() {
            SetDropTarget(nullptr);
        }

        bool MapViewBase::doCanFlipObjects() const {
            MapDocumentSPtr document = lock(m_document);
            return !m_toolBox.anyToolActive() && document->hasSelectedNodes();
        }

        void MapViewBase::doFlipObjects(const vm::direction direction) {
            auto document = lock(m_document);
            if (!document->hasSelectedNodes())
                return;

            // If we snap the selection bounds' center to the grid size, then
            // selections that are an odd number of grid units wide get translated.
            // Instead, snap to 1/2 the grid size.
            // (see: https://github.com/kduske/TrenchBroom/issues/1495 )
            Grid halfGrid(document->grid().size());
            halfGrid.decSize();

            const auto center = halfGrid.referencePoint(document->selectionBounds());
            const auto axis = firstComponent(moveDirection(direction));

            document->flipObjects(center, axis);
        }

        bool MapViewBase::doCancelMouseDrag() {
            return ToolBoxConnector::cancelDrag();
        }

        void MapViewBase::doInitializeGL(const bool firstInitialization) {
            if (firstInitialization) {
                GLVendor   = wxString::FromUTF8(reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
                GLRenderer = wxString::FromUTF8(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
                GLVersion  = wxString::FromUTF8(reinterpret_cast<const char*>(glGetString(GL_VERSION)));

                m_logger->info(wxString::Format(L"Renderer info: %s version %s from %s", GLRenderer, GLVersion, GLVendor));
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
            doPreRender();

            const IO::Path& fontPath = pref(Preferences::RendererFontPath());
            const size_t fontSize = static_cast<size_t>(pref(Preferences::RendererFontSize));
            const Renderer::FontDescriptor fontDescriptor(fontPath, fontSize);

            MapDocumentSPtr document = lock(m_document);
            const MapViewConfig& mapViewConfig = document->mapViewConfig();
            const Grid& grid = document->grid();

            Renderer::RenderContext renderContext(doGetRenderMode(), doGetCamera(), fontManager(), shaderManager());
            renderContext.setShowTextures(mapViewConfig.showTextures());
            renderContext.setShowFaces(mapViewConfig.showFaces());
            renderContext.setShowEdges(mapViewConfig.showEdges());
            renderContext.setShadeFaces(mapViewConfig.shadeFaces());
            renderContext.setShowPointEntities(mapViewConfig.showPointEntities());
            renderContext.setShowPointEntityModels(mapViewConfig.showPointEntityModels());
            renderContext.setShowEntityClassnames(mapViewConfig.showEntityClassnames());
            renderContext.setShowGroupBounds(mapViewConfig.showGroupBounds());
            renderContext.setShowBrushEntityBounds(mapViewConfig.showBrushEntityBounds());
            renderContext.setShowPointEntityBounds(mapViewConfig.showPointEntityBounds());
            renderContext.setShowFog(mapViewConfig.showFog());
            renderContext.setShowGrid(grid.visible());
            renderContext.setGridSize(grid.actualSize());

            setupGL(renderContext);
            setRenderOptions(renderContext);

            Renderer::RenderBatch renderBatch(vertexVbo(), indexVbo());

            doRenderGrid(renderContext, renderBatch);
            doRenderMap(m_renderer, renderContext, renderBatch);
            doRenderTools(m_toolBox, renderContext, renderBatch);
            doRenderExtras(renderContext, renderBatch);
            renderCoordinateSystem(renderContext, renderBatch);
            renderPointFile(renderContext, renderBatch);
            renderPortalFile(renderContext, renderBatch);
            renderCompass(renderBatch);

            renderBatch.render(renderContext);
        }

        void MapViewBase::setupGL(Renderer::RenderContext& context) {
            const Renderer::Camera::Viewport& viewport = context.camera().viewport();
            glAssert(glViewport(viewport.x, viewport.y, viewport.width, viewport.height));

            glAssert(glEnable(GL_MULTISAMPLE));
            glAssert(glEnable(GL_BLEND));
            glAssert(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
            glAssert(glShadeModel(GL_SMOOTH));
        }

        void MapViewBase::renderCoordinateSystem(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            if (pref(Preferences::ShowAxes)) {
                MapDocumentSPtr document = lock(m_document);
                const vm::bbox3& worldBounds = document->worldBounds();

                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.renderCoordinateSystem(vm::bbox3f(worldBounds));
            }
        }

        void MapViewBase::renderPointFile(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            MapDocumentSPtr document = lock(m_document);
            Model::PointFile* pointFile = document->pointFile();
            if (pointFile != nullptr) {
                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.setForegroundColor(pref(Preferences::PointFileColor));
                renderService.renderLineStrip(pointFile->points());
            }
        }

        void MapViewBase::renderPortalFile(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            if (m_portalFileRenderer == nullptr) {
                validatePortalFileRenderer(renderContext);
                assert(m_portalFileRenderer != nullptr);
            }
            renderBatch.add(m_portalFileRenderer.get());
        }

        void MapViewBase::invalidatePortalFileRenderer() {
            m_portalFileRenderer = nullptr;
        }

        void MapViewBase::validatePortalFileRenderer(Renderer::RenderContext& renderContext) {
            assert(m_portalFileRenderer == nullptr);
            m_portalFileRenderer = std::make_unique<Renderer::PrimitiveRenderer>();

            MapDocumentSPtr document = lock(m_document);
            Model::PortalFile* portalFile = document->portalFile();
            if (portalFile != nullptr) {
                for (const auto& poly : portalFile->portals()) {
                    m_portalFileRenderer->renderFilledPolygon(pref(Preferences::PortalFileFillColor),
                                                              Renderer::PrimitiveRenderer::OP_Hide,
                                                              Renderer::PrimitiveRenderer::CP_ShowBackfaces,
                                                              poly.vertices());

                    const auto lineWidth = 4.0f;
                    m_portalFileRenderer->renderPolygon(pref(Preferences::PortalFileBorderColor),
                                                        lineWidth,
                                                        Renderer::PrimitiveRenderer::OP_Hide,
                                                        poly.vertices());
                }
            }
        }

        void MapViewBase::renderCompass(Renderer::RenderBatch& renderBatch) {
            if (m_compass != nullptr)
                m_compass->render(renderBatch);
        }

        void MapViewBase::processEvent(const KeyEvent& event) {
            ToolBoxConnector::processEvent(event);
        }

        void MapViewBase::processEvent(const MouseEvent& event) {
            ToolBoxConnector::processEvent(event);
        }

        void MapViewBase::processEvent(const CancelEvent& event) {
            ToolBoxConnector::processEvent(event);
        }

        static bool isEntity(const Model::Node* node) {
            class IsEntity : public Model::ConstNodeVisitor, public Model::NodeQuery<bool> {
            private:
                void doVisit(const Model::World* world) override   { setResult(false); }
                void doVisit(const Model::Layer* layer) override   { setResult(false); }
                void doVisit(const Model::Group* group) override   { setResult(false); }
                void doVisit(const Model::Entity* entity) override { setResult(true); }
                void doVisit(const Model::Brush* brush) override   { setResult(false); }
            };

            IsEntity visitor;
            node->accept(visitor);
            return visitor.result();
        }

        void MapViewBase::doShowPopupMenu() {
            // We process input events during paint event processing, but we cannot show a popup menu
            // during paint processing, so we enqueue an event for later.
            QueueEvent(new wxCommandEvent(SHOW_POPUP_MENU_EVENT, CommandIds::MapViewPopupMenu::ShowPopupMenu));
        }

        void MapViewBase::OnShowPopupMenu(wxCommandEvent& event) {
            if (!doBeforePopupMenu())
                return;

            MapDocumentSPtr document = lock(m_document);
            const Model::NodeList& nodes = document->selectedNodes().nodes();
            Model::Node* newBrushParent = findNewParentEntityForBrushes(nodes);
            Model::Node* currentGroup = document->editorContext().currentGroup();
            Model::Node* newGroup = findNewGroupForObjects(nodes);
            Model::Node* mergeGroup = findGroupToMergeGroupsInto(document->selectedNodes());

            wxMenu menu;
            menu.SetEventHandler(this);
            menu.Append(CommandIds::MapViewPopupMenu::GroupObjects, "Group");
            menu.Append(CommandIds::MapViewPopupMenu::UngroupObjects, "Ungroup");
            if (mergeGroup != nullptr) {
                menu.Append(CommandIds::MapViewPopupMenu::MergeGroups, "Merge Groups into " + mergeGroup->name());
            } else {
                menu.Append(CommandIds::MapViewPopupMenu::MergeGroups, "Merge Groups");
            }
            menu.Append(CommandIds::MapViewPopupMenu::RenameGroups, "Rename");

            if (newGroup != nullptr && newGroup != currentGroup) {
                menu.Append(CommandIds::MapViewPopupMenu::AddObjectsToGroup, "Add Objects to Group " + newGroup->name());
            }
            if (currentGroup != nullptr && !document->selectedNodes().empty()) {
                menu.Append(CommandIds::MapViewPopupMenu::RemoveObjectsFromGroup, "Remove Objects from Group " + currentGroup->name());
            }
            menu.AppendSeparator();

            if (document->selectedNodes().hasOnlyBrushes()) {
                menu.Append(CommandIds::MapViewPopupMenu::MakeStructural, "Make Structural");
                if (isEntity(newBrushParent)) {
                    menu.Append(CommandIds::MapViewPopupMenu::MoveBrushesToEntity, "Move Brushes to Entity " + newBrushParent->name());
                }
            }

            menu.AppendSeparator();

            menu.AppendSubMenu(makeEntityGroupsMenu(Assets::EntityDefinition::Type_PointEntity, CommandIds::MapViewPopupMenu::LowestPointEntityItem), "Create Point Entity");
            menu.AppendSubMenu(makeEntityGroupsMenu(Assets::EntityDefinition::Type_BrushEntity, CommandIds::MapViewPopupMenu::LowestBrushEntityItem), "Create Brush Entity");

            menu.UpdateUI(this);
            PopupMenu(&menu);

            // Generate a synthetic mouse move event to update the mouse position after the popup menu closes.
            wxMouseEvent mouseEvent(wxEVT_MOTION);
            mouseEvent.SetPosition(ScreenToClient(wxGetMousePosition()));
            OnMouse(mouseEvent);

            doAfterPopupMenu();
        }

        wxMenu* MapViewBase::makeEntityGroupsMenu(const Assets::EntityDefinition::Type type, int id) {
            wxMenu* menu = new wxMenu();

            MapDocumentSPtr document = lock(m_document);
            for (const Assets::EntityDefinitionGroup& group : document->entityDefinitionManager().groups()) {
                const Assets::EntityDefinitionList definitions = group.definitions(type, Assets::EntityDefinition::Name);

                Assets::EntityDefinitionList filteredDefinitions;
                std::copy_if(std::begin(definitions), std::end(definitions), std::back_inserter(filteredDefinitions),
                             [](const Assets::EntityDefinition* definition) { return !StringUtils::caseSensitiveEqual(definition->name(), Model::AttributeValues::WorldspawnClassname); }
                );

                if (!filteredDefinitions.empty()) {
                    const String groupName = group.displayName();
                    wxMenu* groupMenu = new wxMenu();
                    groupMenu->SetEventHandler(this);

                    for (Assets::EntityDefinition* definition : filteredDefinitions)
                        groupMenu->Append(id++, definition->shortName());

                    menu->AppendSubMenu(groupMenu, groupName);
                }
            }

            return menu;
        }

        void MapViewBase::OnAddObjectsToGroup(wxCommandEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            const Model::NodeList nodes = document->selectedNodes().nodes();
            Model::Node* newGroup = findNewGroupForObjects(nodes);
            ensure(newGroup != nullptr, "newGroup is null");

            Transaction transaction(document, "Add Objects to Group");
            reparentNodes(nodes, newGroup, true);
            document->deselectAll();
            document->select(newGroup);
        }

        void MapViewBase::OnRemoveObjectsFromGroup(wxCommandEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            const Model::NodeList nodes = document->selectedNodes().nodes();
            Model::Node* currentGroup = document->editorContext().currentGroup();
            ensure(currentGroup != nullptr, "currentGroup is null");

            Transaction transaction(document, "Remove Objects from Group");
            reparentNodes(nodes, document->currentLayer(), true);

            while (document->currentGroup() != nullptr) {
                document->closeGroup();
            }
            document->select(nodes);
        }

        Model::Node* MapViewBase::findNewGroupForObjects(const Model::NodeList& nodes) const {
            Model::Node* newGroup = nullptr;

            MapDocumentSPtr document = lock(m_document);
            const Model::Hit& hit = pickResult().query().pickable().first();
            if (hit.isMatch())
                newGroup = outermostClosedGroup(Model::hitToNode(hit));

            if (newGroup != nullptr && canReparentNodes(nodes, newGroup))
                return newGroup;
            return nullptr;
        }

        void MapViewBase::OnMergeGroups(wxCommandEvent& event) {
            auto document = lock(m_document);
            auto* newGroup = findGroupToMergeGroupsInto(document->selectedNodes());
            ensure(newGroup != nullptr, "newGroup is null");

            Transaction transaction(document, "Merge Groups");
            document->mergeSelectedGroupsWithGroup(newGroup);
        }

        Model::Group* MapViewBase::findGroupToMergeGroupsInto(const Model::NodeCollection& selectedNodes) const {
            if (!(selectedNodes.hasOnlyGroups() && selectedNodes.groupCount() >= 2)) {
                return nullptr;
            }

            Model::Group* mergeTarget = nullptr;

            auto document = lock(m_document);
            const Model::Hit& hit = pickResult().query().pickable().first();
            if (hit.isMatch()) {
                mergeTarget = outermostClosedGroup(Model::hitToNode(hit));
            }
            if (mergeTarget == nullptr) {
                return nullptr;
            }

            const auto& nodes = selectedNodes.nodes();
            const bool canReparentAll = std::all_of(nodes.begin(), nodes.end(), [&](const auto* node){
                return node == mergeTarget || this->canReparentNode(node, mergeTarget);
            });

            if (canReparentAll) {
                return mergeTarget;
            } else {
                return nullptr;
            }
        }

        bool MapViewBase::canReparentNode(const Model::Node* node, const Model::Node* newParent) const {
            return newParent != node && newParent != node->parent() && !newParent->isDescendantOf(node);
        }

        void MapViewBase::OnMoveBrushesTo(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            const Model::NodeList nodes = document->selectedNodes().nodes();
            Model::Node* newParent = findNewParentEntityForBrushes(nodes);
            ensure(newParent != nullptr, "newParent is null");

            const Transaction transaction(document, "Move " + StringUtils::safePlural(nodes.size(), "Brush", "Brushes"));
            reparentNodes(nodes, newParent, false);

            document->deselectAll();
            document->select(nodes);
        }

        Model::Node* MapViewBase::findNewParentEntityForBrushes(const Model::NodeList& nodes) const {
            Model::Node* newParent = nullptr;

            MapDocumentSPtr document = lock(m_document);
            const Model::Hit& hit = pickResult().query().pickable().type(Model::Brush::BrushHit).occluded().first();
            if (hit.isMatch()) {
                const Model::Brush* brush = Model::hitToBrush(hit);
                newParent = brush->entity();
            }

            if (newParent != nullptr && newParent != document->world() && canReparentNodes(nodes, newParent)) {
                return newParent;
            }

            if (!nodes.empty()) {
                Model::Node* lastNode = nodes.back();

                Model::Group* group = Model::findGroup(lastNode);
                if (group != nullptr) {
                    return group;
                }

                Model::Layer* layer = Model::findLayer(lastNode);
                if (layer != nullptr) {
                    return layer;
                }
            }

            return document->currentLayer();
        }

        bool MapViewBase::canReparentNodes(const Model::NodeList& nodes, const Model::Node* newParent) const {
            for (const Model::Node* node : nodes) {
                if (canReparentNode(node, newParent)) {
                    return true;
                }
            }
            return false;
        }

        class BrushesToEntities {
        private:
            const Model::World* m_world;
        public:
            BrushesToEntities(const Model::World* world) : m_world(world) {}
        public:
            bool operator()(const Model::World* world) const   { return false; }
            bool operator()(const Model::Layer* layer) const   { return false; }
            bool operator()(const Model::Group* group) const   { return true;  }
            bool operator()(const Model::Entity* entity) const { return true; }
            bool operator()(const Model::Brush* brush) const   { return brush->entity() == m_world; }
        };

        static Model::NodeList collectEntitiesForBrushes(const Model::NodeList& selectedNodes, const Model::World *world) {
            using BrushesToEntitiesVisitor = Model::CollectMatchingNodesVisitor<BrushesToEntities, Model::UniqueNodeCollectionStrategy, Model::StopRecursionIfMatched>;

            BrushesToEntitiesVisitor collect(world);
            Model::Node::acceptAndEscalate(std::begin(selectedNodes), std::end(selectedNodes), collect);
            return collect.nodes();
        }

        void MapViewBase::reparentNodes(const Model::NodeList& nodes, Model::Node* newParent, const bool preserveEntities) {
            ensure(newParent != nullptr, "newParent is null");

            MapDocumentSPtr document = lock(m_document);
            Model::NodeList inputNodes;
            if (preserveEntities) {
                inputNodes = collectEntitiesForBrushes(nodes, document->world());
            } else {
                inputNodes = nodes;
            }

            const Model::NodeList reparentableNodes = collectReparentableNodes(inputNodes, newParent);
            assert(!reparentableNodes.empty());

            StringStream name;
            name << "Move " << (reparentableNodes.size() == 1 ? "Object" : "Objects") << " to " << newParent->name();

            const Transaction transaction(document, name.str());
            document->deselectAll();
            document->reparentNodes(newParent, reparentableNodes);
            document->select(reparentableNodes);
        }

        Model::NodeList MapViewBase::collectReparentableNodes(const Model::NodeList& nodes, const Model::Node* newParent) const {
            Model::NodeList result;
            std::copy_if(std::begin(nodes), std::end(nodes), std::back_inserter(result), [=](const Model::Node* node) { return newParent != node && newParent != node->parent() && !newParent->isDescendantOf(node); });
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
                case CommandIds::MapViewPopupMenu::MergeGroups:
                    updateMergeGroupsMenuItem(event);
                    break;
                case CommandIds::MapViewPopupMenu::RenameGroups:
                    updateRenameGroupsMenuItem(event);
                    break;
                case CommandIds::MapViewPopupMenu::MakeStructural:
                    updateMakeStructuralMenuItem(event);
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

        void MapViewBase::updateMergeGroupsMenuItem(wxUpdateUIEvent& event) const {
            MapDocumentSPtr document = lock(m_document);
            Model::Node* mergeGroup = findGroupToMergeGroupsInto(document->selectedNodes());
            event.Enable(mergeGroup != nullptr);
        }

        void MapViewBase::updateRenameGroupsMenuItem(wxUpdateUIEvent& event) const {
            MapDocumentSPtr document = lock(m_document);
            event.Enable(document->selectedNodes().hasOnlyGroups());
        }

        void MapViewBase::updateMakeStructuralMenuItem(wxUpdateUIEvent& event) const {
            MapDocumentSPtr document = lock(m_document);
            if (document->selectedNodes().hasOnlyBrushes()) {
                const Model::BrushList& brushes = document->selectedNodes().brushes();
                for (const auto* brush : brushes) {
                    if (brush->hasAnyTag() || brush->entity() != document->world() || brush->anyFaceHasAnyTag()) {
                        event.Enable(true);
                        return;
                    }
                }
            }
            event.Enable(false);
        }

        void MapViewBase::doPreRender() {}

        void MapViewBase::doRenderExtras(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {}

        bool MapViewBase::doBeforePopupMenu() { return true; }
        void MapViewBase::doAfterPopupMenu() {}
    }
}
