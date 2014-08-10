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

#include "MapView.h"

#include "CollectionUtils.h"
#include "Exceptions.h"
#include "Logger.h"
#include "Macros.h"
#include "Notifier.h"
#include "Preferences.h"
#include "Model/Brush.h"
#include "Model/BrushVertex.h"
#include "Model/Entity.h"
#include "Model/HitAdapter.h"
#include "Model/Map.h"
#include "Model/ModelUtils.h"
#include "Model/Object.h"
#include "Renderer/PerspectiveCamera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Transformation.h"
#include "Renderer/Vertex.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexSpec.h"
#include "View/ActionManager.h"
#include "View/Animation.h"
#include "View/CameraAnimation.h"
#include "View/CameraTool.h"
#include "View/ClipTool.h"
#include "View/CommandIds.h"
#include "View/CreateBrushTool.h"
#include "View/CreateEntityTool.h"
#include "View/FlashSelectionAnimation.h"
#include "View/MapDocument.h"
#include "View/ToolBoxDropTarget.h"
#include "View/MoveObjectsTool.h"
#include "View/ResizeBrushesTool.h"
#include "View/RotateObjectsTool.h"
#include "View/SelectionTool.h"
#include "View/SetFaceAttribsTool.h"
#include "View/TextureTool.h"
#include "View/VertexTool.h"
#include "View/wxUtils.h"

#include <wx/menu.h>
#include <wx/timer.h>

namespace TrenchBroom {
    namespace View {
        const int MapView::FlyTimerId = wxID_HIGHEST + 1;
        
        MapView::MapView(wxWindow* parent, Logger* logger, wxBookCtrlBase* toolBook, View::MapDocumentWPtr document, ControllerWPtr controller, Renderer::Camera& camera) :
        RenderView(parent, attribs()),
        m_logger(logger),
        m_document(document),
        m_controller(controller),
        m_camera(camera),
        m_animationManager(new AnimationManager()),
        m_toolBox(this, this),
        m_cameraTool(NULL),
        m_clipTool(NULL),
        m_createBrushTool(NULL),
        m_createEntityTool(NULL),
        m_moveObjectsTool(NULL),
        m_vertexTool(NULL),
        m_resizeBrushesTool(NULL),
        m_rotateObjectsTool(NULL),
        m_selectionTool(NULL),
        m_setFaceAttribsTool(NULL),
        m_textureTool(NULL),
        m_flyModeHelper(this, camera),
        m_renderer(document, contextHolder()->fontManager()),
        m_compass(),
        m_selectionGuide(document, defaultFont(contextHolder()->fontManager())) {
            createTools(toolBook);
            bindEvents();
            bindObservers();
            updateAcceleratorTable(HasFocus());
        }
        
        MapView::~MapView() {
            if (cameraFlyModeActive())
                toggleCameraFlyMode();
            
            unbindObservers();
            deleteTools();
            m_animationManager->Delete();
            m_animationManager = NULL;
            m_logger = NULL;
        }
        
        void MapView::setToolboxDropTarget() {
            SetDropTarget(new ToolBoxDropTarget(m_toolBox));
        }
        
        void MapView::clearToolboxDropTarget() {
            SetDropTarget(NULL);
        }

        void MapView::centerCameraOnSelection() {
            MapDocumentSPtr document = lock(m_document);
            const Model::EntityList& entities = document->selectedEntities();
            const Model::BrushList& brushes = document->selectedBrushes();
            assert(!entities.empty() || !brushes.empty());
            
            const Vec3 newPosition = centerCameraOnObjectsPosition(entities, brushes);
            animateCamera(newPosition, m_camera.direction(), m_camera.up(), 150);
        }
        
        void MapView::animateCamera(const Vec3f& position, const Vec3f& direction, const Vec3f& up, const wxLongLong duration) {
            CameraAnimation* animation = new CameraAnimation(m_camera, position, direction, up, duration);
            m_animationManager->runAnimation(animation, true);
        }
        
        void MapView::flashSelection() {
            FlashSelectionAnimation* animation = new FlashSelectionAnimation(m_renderer, *this, 180);
            m_animationManager->runAnimation(animation, true);
        }

        bool MapView::cameraFlyModeActive() const {
            return m_flyModeHelper.enabled();
        }
        
        void MapView::toggleCameraFlyMode() {
            if (!cameraFlyModeActive()) {
                m_toolBox.disable();
                m_flyModeHelper.enable();
            } else {
                m_flyModeHelper.disable();
                m_toolBox.enable();
            }
            updateAcceleratorTable(HasFocus());
            Refresh();
        }
        
        void MapView::toggleMovementRestriction() {
            m_movementRestriction.toggleHorizontalRestriction(m_camera);
            updateAcceleratorTable(HasFocus());
            Refresh();
        }
        
        bool MapView::anyToolActive() const {
            return m_toolBox.anyToolActive();
        }
        
        void MapView::toggleClipTool() {
            m_toolBox.toggleTool(m_clipTool);
            updateAcceleratorTable(HasFocus());
        }
        
        bool MapView::clipToolActive() const {
            return m_toolBox.toolActive(m_clipTool);
        }
        
        bool MapView::canToggleClipSide() const {
            assert(clipToolActive());
            return m_clipTool->canToggleClipSide();
        }
        
        void MapView::toggleClipSide() {
            assert(clipToolActive());
            m_clipTool->toggleClipSide();
            m_toolBox.updateHits();
            Refresh();
        }
        
        bool MapView::canPerformClip() const {
            assert(clipToolActive());
            return m_clipTool->canPerformClip();
        }
        
        void MapView::performClip() {
            assert(clipToolActive());
            m_clipTool->performClip();
            m_toolBox.updateHits();
            Refresh();
        }
        
        bool MapView::canDeleteLastClipPoint() const {
            assert(clipToolActive());
            return m_clipTool->canDeleteLastClipPoint();
        }
        
        void MapView::deleteLastClipPoint() {
            assert(clipToolActive());
            m_clipTool->deleteLastClipPoint();
            m_toolBox.updateHits();
            Refresh();
        }
        
        void MapView::toggleRotateObjectsTool() {
            m_toolBox.toggleTool(m_rotateObjectsTool);
            updateAcceleratorTable(HasFocus());
        }
        
        bool MapView::rotateObjectsToolActive() const {
            return m_toolBox.toolActive(m_rotateObjectsTool);
        }
        
        void MapView::toggleVertexTool() {
            m_toolBox.toggleTool(m_vertexTool);
            updateAcceleratorTable(HasFocus());
        }
        
        bool MapView::vertexToolActive() const {
            return m_toolBox.toolActive(m_vertexTool);
        }
        
        bool MapView::hasSelectedVertices() const {
            return vertexToolActive() && m_vertexTool->hasSelectedHandles();
        }
        
        bool MapView::canSnapVertices() const {
            return vertexToolActive() && m_vertexTool->canSnapVertices();
        }
        
        void MapView::snapVertices(const size_t snapTo) {
            assert(vertexToolActive());
            m_vertexTool->snapVertices(snapTo);
            m_toolBox.updateHits();
        }
        
        void MapView::toggleTextureTool() {
            m_toolBox.toggleTool(m_textureTool);
            updateAcceleratorTable(HasFocus());
        }
        
        bool MapView::textureToolActive() const {
            return m_toolBox.toolActive(m_textureTool);
        }
        
        Vec3 MapView::pasteObjectsDelta(const BBox3& bounds) const {
            MapDocumentSPtr document = lock(m_document);
            const Grid& grid = document->grid();
            const wxMouseState mouseState = wxGetMouseState();
            const wxPoint clientCoords = ScreenToClient(mouseState.GetPosition());
            if (HitTest(clientCoords) == wxHT_WINDOW_INSIDE) {
                const Ray3f pickRay = m_camera.pickRay(clientCoords.x, clientCoords.y);
                const Hits& hits = document->pick(Ray3(pickRay));
                const Hit& hit = Model::findFirstHit(hits, Model::Brush::BrushHit, document->filter(), true);
                if (hit.isMatch()) {
                    const Model::BrushFace* face = Model::hitAsFace(hit);
                    const Vec3 snappedHitPoint = grid.snap(hit.hitPoint());
                    return grid.moveDeltaForBounds(face, bounds, document->worldBounds(), pickRay, snappedHitPoint);
                } else {
                    const Vec3 snappedCenter = grid.snap(bounds.center());
                    const Vec3 snappedDefaultPoint = grid.snap(m_camera.defaultPoint(pickRay));
                    return snappedDefaultPoint - snappedCenter;
                }
            } else {
                const Vec3 snappedCenter = grid.snap(bounds.center());
                const Vec3 snappedDefaultPoint = grid.snap(m_camera.defaultPoint());
                return snappedDefaultPoint - snappedCenter;
            }
        }
        
        void MapView::OnToggleClipTool(wxCommandEvent& event) {
            assert(lock(m_document)->hasSelectedBrushes());
            toggleClipTool();
        }
        
        void MapView::OnToggleClipSide(wxCommandEvent& event) {
            assert(clipToolActive());
            if (canToggleClipSide())
                toggleClipSide();
        }
        
        void MapView::OnPerformClip(wxCommandEvent& event) {
            assert(clipToolActive());
            if (canPerformClip())
                performClip();
        }
        
        void MapView::OnDeleteLastClipPoint(wxCommandEvent& event) {
            assert(clipToolActive());
            if (canDeleteLastClipPoint())
                deleteLastClipPoint();
        }
        
        void MapView::OnToggleVertexTool(wxCommandEvent& event) {
            toggleVertexTool();
        }
        
        void MapView::OnMoveVerticesForward(wxCommandEvent& event) {
            assert(vertexToolActive());
            moveVertices(Math::Direction_Forward);
        }
        
        void MapView::OnMoveVerticesBackward(wxCommandEvent& event) {
            assert(vertexToolActive());
            moveVertices(Math::Direction_Backward);
        }
        
        void MapView::OnMoveVerticesLeft(wxCommandEvent& event) {
            assert(vertexToolActive());
            moveVertices(Math::Direction_Left);
        }
        
        void MapView::OnMoveVerticesRight(wxCommandEvent& event) {
            assert(vertexToolActive());
            moveVertices(Math::Direction_Right);
        }
        
        void MapView::OnMoveVerticesUp(wxCommandEvent& event) {
            assert(vertexToolActive());
            moveVertices(Math::Direction_Up);
        }
        
        void MapView::OnMoveVerticesDown(wxCommandEvent& event) {
            assert(vertexToolActive());
            moveVertices(Math::Direction_Down);
        }
        
        void MapView::moveVertices(const Math::Direction direction) {
            assert(vertexToolActive());
            if (hasSelectedVertices()) {
                MapDocumentSPtr document = lock(m_document);
                const Grid& grid = document->grid();
                const Vec3 delta = moveDirection(direction) * static_cast<FloatType>(grid.actualSize());
                m_vertexTool->moveVerticesAndRebuildBrushGeometry(delta);
            }
        }
        
        void MapView::OnToggleRotateObjectsTool(wxCommandEvent& event) {
            toggleRotateObjectsTool();
        }
        
        void MapView::OnToggleFlyMode(wxCommandEvent& event) {
            toggleCameraFlyMode();
        }
        
        void MapView::OnToggleMovementRestriction(wxCommandEvent& event) {
            toggleMovementRestriction();
        }
        
        void MapView::OnDeleteObjects(wxCommandEvent& event) {
            lock(m_controller)->deleteSelectedObjects();
        }

        void MapView::OnMoveObjectsForward(wxCommandEvent& event) {
            moveObjects(Math::Direction_Forward);
        }
        
        void MapView::OnMoveObjectsBackward(wxCommandEvent& event) {
            moveObjects(Math::Direction_Backward);
        }
        
        void MapView::OnMoveObjectsLeft(wxCommandEvent& event) {
            moveObjects(Math::Direction_Left);
        }
        
        void MapView::OnMoveObjectsRight(wxCommandEvent& event) {
            moveObjects(Math::Direction_Right);
        }
        
        void MapView::OnMoveObjectsUp(wxCommandEvent& event) {
            moveObjects(Math::Direction_Up);
        }
        
        void MapView::OnMoveObjectsDown(wxCommandEvent& event) {
            moveObjects(Math::Direction_Down);
        }
        
        void MapView::OnRollObjectsCW(wxCommandEvent& event) {
            rotateObjects(RotationAxis_Roll, true);
        }
        
        void MapView::OnRollObjectsCCW(wxCommandEvent& event) {
            rotateObjects(RotationAxis_Roll, false);
        }
        
        void MapView::OnPitchObjectsCW(wxCommandEvent& event) {
            rotateObjects(RotationAxis_Pitch, true);
        }
        
        void MapView::OnPitchObjectsCCW(wxCommandEvent& event) {
            rotateObjects(RotationAxis_Pitch, false);
        }
        
        void MapView::OnYawObjectsCW(wxCommandEvent& event) {
            rotateObjects(RotationAxis_Yaw, true);
        }
        
        void MapView::OnYawObjectsCCW(wxCommandEvent& event) {
            rotateObjects(RotationAxis_Yaw, false);
        }
        
        void MapView::OnFlipObjectsH(wxCommandEvent& event) {
            flipObjects(Math::Direction_Left);
        }
        
        void MapView::OnFlipObjectsV(wxCommandEvent& event) {
            flipObjects(Math::Direction_Up);
        }
        
        void MapView::OnDuplicateObjectsForward(wxCommandEvent& event) {
            duplicateAndMoveObjects(Math::Direction_Forward);
        }
        
        void MapView::OnDuplicateObjectsBackward(wxCommandEvent& event) {
            duplicateAndMoveObjects(Math::Direction_Backward);
        }
        
        void MapView::OnDuplicateObjectsLeft(wxCommandEvent& event) {
            duplicateAndMoveObjects(Math::Direction_Left);
        }
        
        void MapView::OnDuplicateObjectsRight(wxCommandEvent& event) {
            duplicateAndMoveObjects(Math::Direction_Right);
        }
        
        void MapView::OnDuplicateObjectsUp(wxCommandEvent& event) {
            duplicateAndMoveObjects(Math::Direction_Up);
        }
        
        void MapView::OnDuplicateObjectsDown(wxCommandEvent& event) {
            duplicateAndMoveObjects(Math::Direction_Down);
        }
        
        void MapView::OnDuplicateObjects(wxCommandEvent& event) {
            ControllerSPtr controller = lock(m_controller);
            const UndoableCommandGroup commandGroup(controller, "Duplicate Objects");
            duplicateObjects();
        }

        void MapView::rotateObjects(const RotationAxis axisSpec, const bool clockwise) {
            MapDocumentSPtr document = lock(m_document);
            const Model::ObjectList& objects = document->selectedObjects();
            if (objects.empty())
                return;
            
            Vec3 axis;
            switch (axisSpec) {
                case RotationAxis_Roll:
                    axis = moveDirection(Math::Direction_Forward);
                    break;
                case RotationAxis_Pitch:
                    axis = moveDirection(Math::Direction_Right);
                    break;
                case RotationAxis_Yaw:
                    axis = Vec3::PosZ;
                    break;
                    DEFAULT_SWITCH()
            }
            
            if (!clockwise)
                axis *= -1.0;
            
            ControllerSPtr controller = lock(m_controller);
            const Grid& grid = document->grid();
            const Vec3 center = grid.referencePoint(document->selectionBounds());
            controller->rotateObjects(objects, center, axis, Math::C::piOverTwo(), document->textureLock());
        }
        
        void MapView::flipObjects(const Math::Direction direction) {
            MapDocumentSPtr document = lock(m_document);
            const Model::ObjectList& objects = document->selectedObjects();
            if (objects.empty())
                return;
            
            const Grid& grid = document->grid();
            const Vec3 center = grid.referencePoint(document->selectionBounds());
            const Math::Axis::Type axis = moveDirection(direction).firstComponent();
            
            ControllerSPtr controller = lock(m_controller);
            controller->flipObjects(objects, center, axis, document->textureLock());
        }
        
        void MapView::duplicateAndMoveObjects(const Math::Direction direction) {
            ControllerSPtr controller = lock(m_controller);
            const UndoableCommandGroup commandGroup(controller, "Duplicate Objects");
            duplicateObjects();
            moveObjects(direction);
        }
        
        void MapView::duplicateObjects() {
            MapDocumentSPtr document = lock(m_document);
            const Model::ObjectList& objects = document->selectedObjects();
            if (objects.empty())
                return;
            
            ControllerSPtr controller = lock(m_controller);
            const Model::ObjectList duplicates = controller->duplicateObjects(objects, document->worldBounds());
            controller->deselectAllAndSelectObjects(duplicates);
            flashSelection();
        }
        
        void MapView::moveObjects(const Math::Direction direction) {
            MapDocumentSPtr document = lock(m_document);
            const Model::ObjectList& objects = document->selectedObjects();
            if (objects.empty())
                return;
            
            ControllerSPtr controller = lock(m_controller);
            const Grid& grid = document->grid();
            const Vec3 delta = moveDirection(direction) * static_cast<FloatType>(grid.actualSize());
            
            controller->moveObjects(objects, delta, document->textureLock());
        }
        
        void MapView::OnMoveTexturesUp(wxCommandEvent& event) {
            moveTextures(Vec2f(0.0f, moveTextureDistance()));
        }
        
        void MapView::OnMoveTexturesDown(wxCommandEvent& event) {
            moveTextures(Vec2f(0.0f, -moveTextureDistance()));
        }
        
        void MapView::OnMoveTexturesLeft(wxCommandEvent& event) {
            moveTextures(Vec2f(-moveTextureDistance(), 0.0f));
        }
        
        void MapView::OnMoveTexturesRight(wxCommandEvent& event) {
            moveTextures(Vec2f(moveTextureDistance(), 0.0f));
        }
        
        void MapView::OnRotateTexturesCW(wxCommandEvent& event) {
            rotateTextures(rotateTextureAngle(true));
        }
        
        void MapView::OnRotateTexturesCCW(wxCommandEvent& event) {
            rotateTextures(rotateTextureAngle(false));
        }
        
        float MapView::moveTextureDistance() const {
            const Grid& grid = lock(m_document)->grid();
            const float gridSize = static_cast<float>(grid.actualSize());
            
            const wxMouseState mouseState = wxGetMouseState();
            switch (mouseState.GetModifiers()) {
                case wxMOD_CMD:
                    return 1.0f;
                case wxMOD_SHIFT:
                    return 2.0f * gridSize;
                default:
                    return gridSize;
            }
        }
        
        void MapView::moveTextures(const Vec2f& offset) {
            MapDocumentSPtr document = lock(m_document);
            const Model::BrushFaceList& faces = document->allSelectedFaces();
            assert(!faces.empty());
            
            ControllerSPtr controller = lock(m_controller);
            controller->moveTextures(faces, m_camera.up(), m_camera.right(), offset);
        }
        
        float MapView::rotateTextureAngle(const bool clockwise) const {
            const Grid& grid = lock(m_document)->grid();
            const float gridAngle = static_cast<float>(Math::degrees(grid.angle()));
            float angle = 0.0f;
            
            const wxMouseState mouseState = wxGetMouseState();
            switch (mouseState.GetModifiers()) {
                case wxMOD_CMD:
                    angle = 1.0f;
                    break;
                case wxMOD_SHIFT:
                    angle = 90.0f;
                    break;
                default:
                    angle = gridAngle;
                    break;
            }
            
            return clockwise ? angle : -angle;
        }
        
        void MapView::rotateTextures(const float angle) {
            MapDocumentSPtr document = lock(m_document);
            const Model::BrushFaceList& faces = document->selectedFaces();
            assert(!faces.empty());
            
            ControllerSPtr controller = lock(m_controller);
            controller->rotateTextures(faces, angle);
        }
        
        void MapView::OnKey(wxKeyEvent& event) {
            m_movementRestriction.setVerticalRestriction(event.AltDown());
            Refresh();
            event.Skip();
        }
        
        void MapView::OnActivateFrame(wxActivateEvent& event) {
            if (event.GetActive())
                m_toolBox.updateLastActivation();
            if (cameraFlyModeActive())
                toggleCameraFlyMode();
            event.Skip();
        }
        
        void MapView::OnSetFocus(wxFocusEvent& event) {
            updateAcceleratorTable(true);
            event.Skip();
        }
        
        void MapView::OnKillFocus(wxFocusEvent& event) {
            if (cameraFlyModeActive())
                toggleCameraFlyMode();
            updateAcceleratorTable(false);
            event.Skip();
        }
        
        void MapView::OnPopupReparentBrushes(wxCommandEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            const Model::BrushList& brushes = document->selectedBrushes();
            Model::Entity* newParent = findNewBrushParent(brushes);
            assert(newParent != NULL);
            
            reparentBrushes(brushes, newParent);
        }
        
        void MapView::OnPopupMoveBrushesToWorld(wxCommandEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            const Model::BrushList& brushes = document->selectedBrushes();
            reparentBrushes(brushes, document->worldspawn());
        }
        
        void MapView::OnPopupCreatePointEntity(wxCommandEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            Assets::EntityDefinitionManager& manager = document->entityDefinitionManager();
            const Assets::EntityDefinitionGroups groups = manager.groups(Assets::EntityDefinition::Type_PointEntity);
            const size_t index = static_cast<size_t>(event.GetId() - CommandIds::CreateEntityPopupMenu::LowestPointEntityItem);
            const Assets::EntityDefinition* definition = findEntityDefinition(groups, index);
            assert(definition != NULL);
            assert(definition->type() == Assets::EntityDefinition::Type_PointEntity);
            createPointEntity(*static_cast<const Assets::PointEntityDefinition*>(definition));
        }
        
        void MapView::OnPopupCreateBrushEntity(wxCommandEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            Assets::EntityDefinitionManager& manager = document->entityDefinitionManager();
            const Assets::EntityDefinitionGroups groups = manager.groups(Assets::EntityDefinition::Type_BrushEntity);
            const size_t index = static_cast<size_t>(event.GetId() - CommandIds::CreateEntityPopupMenu::LowestBrushEntityItem);
            const Assets::EntityDefinition* definition = findEntityDefinition(groups, index);
            assert(definition != NULL);
            assert(definition->type() == Assets::EntityDefinition::Type_BrushEntity);
            createBrushEntity(*static_cast<const Assets::BrushEntityDefinition*>(definition));
        }
        
        void MapView::updateAcceleratorTable(const bool hasFocus) {
            if (hasFocus) {
                const ActionManager& actionManager = ActionManager::instance();
                const Action::Context context = actionContext();
                const wxAcceleratorTable acceleratorTable = actionManager.createMapViewAcceleratorTable(context);
                SetAcceleratorTable(acceleratorTable);
            } else {
                SetAcceleratorTable(wxNullAcceleratorTable);
            }
        }
        
        Action::Context MapView::actionContext() const {
            if (clipToolActive())
                return Action::Context_ClipTool;
            if (vertexToolActive())
                return Action::Context_VertexTool;
            if (rotateObjectsToolActive())
                return Action::Context_RotateTool;
            if (cameraFlyModeActive())
                return Action::Context_FlyMode;
            
            MapDocumentSPtr document = lock(m_document);
            if (document->hasSelectedObjects())
                return Action::Context_ObjectSelection;
            if (document->hasSelectedFaces())
                return Action::Context_FaceSelection;
            return Action::Context_Default;
        }
        
        void MapView::OnUpdatePopupMenuItem(wxUpdateUIEvent& event) {
            switch (event.GetId()) {
                case CommandIds::CreateEntityPopupMenu::ReparentBrushes:
                    updateReparentBrushesMenuItem(event);
                    break;
                case CommandIds::CreateEntityPopupMenu::MoveBrushesToWorld:
                    updateMoveBrushesToWorldMenuItem(event);
                    break;
                default:
                    event.Enable(true);
                    break;
            }
        }
        
        void MapView::updateReparentBrushesMenuItem(wxUpdateUIEvent& event) const {
            MapDocumentSPtr document = lock(m_document);
            const Model::BrushList& brushes = document->selectedBrushes();
            StringStream name;
            name << "Move " << (brushes.size() == 1 ? "Brush" : "Brushes") << " to ";
            
            if (!document->hasSelectedBrushes() || document->hasSelectedEntities() || document->hasSelectedFaces()) {
                event.Enable(false);
                name << "Entity";
            } else {
                const Model::Entity* newParent = findNewBrushParent(brushes);
                if (newParent != NULL) {
                    event.Enable(true);
                    name << newParent->classname("<missing classname>");
                } else {
                    event.Enable(false);
                    name << "Entity";
                }
            }
            event.SetText(name.str());
        }
        
        void MapView::updateMoveBrushesToWorldMenuItem(wxUpdateUIEvent& event) const {
            MapDocumentSPtr document = lock(m_document);
            const Model::BrushList& brushes = document->selectedBrushes();
            StringStream name;
            name << "Move " << (brushes.size() == 1 ? "Brush" : "Brushes") << " to World";
            event.Enable(canReparentBrushes(brushes, document->worldspawn()));
            event.SetText(name.str());
        }
        
        Model::Entity* MapView::findNewBrushParent(const Model::BrushList& brushes) const {
            Model::Entity* newParent = NULL;
            
            MapDocumentSPtr document = lock(m_document);
            const Hit& hit = Model::findFirstHit(m_toolBox.hits(), Model::Entity::EntityHit | Model::Brush::BrushHit, document->filter(), true);
            if (hit.isMatch()) {
                if (hit.type() == Model::Entity::EntityHit) {
                    newParent = Model::hitAsEntity(hit);
                } else if (hit.type() == Model::Brush::BrushHit) {
                    const Model::Brush* brush = Model::hitAsBrush(hit);
                    newParent = brush->parent();
                }
            }
            
            if (newParent == NULL)
                return NULL;
            if (canReparentBrushes(brushes, newParent))
                return newParent;
            return NULL;
        }
        
        // note that we make a copy of the brush list on purpose here
        void MapView::reparentBrushes(const Model::BrushList& brushes, Model::Entity* newParent) {
            assert(newParent != NULL);
            
            const Model::BrushList reparentableBrushes = filterReparentableBrushes(brushes, newParent);
            assert(!reparentableBrushes.empty());
            
            ControllerSPtr controller = lock(m_controller);
            
            StringStream name;
            name << "Move " << (reparentableBrushes.size() == 1 ? "Brush" : "Brushes") << " to " << newParent->classname("<missing classname>");
            
            const UndoableCommandGroup commandGroup(controller, name.str());
            controller->deselectAll();
            controller->reparentBrushes(reparentableBrushes, newParent);
            controller->selectObjects(VectorUtils::cast<Model::Object*>(brushes));
        }
        
        struct ReparentFilter {
        private:
            const Model::Entity* m_newParent;
        public:
            ReparentFilter(const Model::Entity* newParent) :
            m_newParent(newParent) {}
            
            bool operator()(Model::Brush* brush) const {
                return brush->parent() != m_newParent;
            }
        };

        bool MapView::canReparentBrushes(const Model::BrushList& brushes, const Model::Entity* newParent) const {
            return Model::any(brushes.begin(), brushes.end(), ReparentFilter(newParent));
        }
        
        Model::BrushList MapView::filterReparentableBrushes(const Model::BrushList& brushes, Model::Entity* newParent) {
            Model::BrushList result;
            Model::filter(brushes.begin(), brushes.end(), ReparentFilter(newParent), std::back_inserter(result));
            return result;
        }

        Assets::EntityDefinition* MapView::findEntityDefinition(const Assets::EntityDefinitionGroups& groups, const size_t index) const {
            Assets::EntityDefinitionGroups::const_iterator groupIt, groupEnd;
            Assets::EntityDefinitionList::const_iterator defIt, defEnd;
            
            size_t count = 0;
            for (groupIt = groups.begin(), groupEnd = groups.end(); groupIt != groupEnd; ++groupIt) {
                const Assets::EntityDefinitionList& definitions = groupIt->second;
                if (index < count + definitions.size())
                    return definitions[index - count];
                count += definitions.size();
            }
            return NULL;
        }
        
        void MapView::createPointEntity(const Assets::PointEntityDefinition& definition) {
            MapDocumentSPtr document = lock(m_document);
            ControllerSPtr controller = lock(m_controller);
            
            Model::Entity* entity = document->map()->createEntity();
            entity->addOrUpdateProperty(Model::PropertyKeys::Classname, definition.name());
            
            Vec3 delta;
            View::Grid& grid = document->grid();
            
            const Hit& hit = Model::findFirstHit(m_toolBox.hits(), Model::Brush::BrushHit, document->filter(), true);
            if (hit.isMatch()) {
                delta = grid.moveDeltaForBounds(Model::hitAsFace(hit), definition.bounds(), document->worldBounds(), m_toolBox.pickRay(), hit.hitPoint());
            } else {
                const Vec3 newPosition(m_camera.defaultPoint(m_toolBox.pickRay()));
                delta = grid.moveDeltaForPoint(definition.bounds().center(), document->worldBounds(), newPosition - definition.bounds().center());
            }
            
            StringStream name;
            name << "Create " << definition.name();
            
            const UndoableCommandGroup commandGroup(controller, name.str());
            controller->deselectAll();
            controller->addEntity(entity);
            controller->selectObject(entity);
            controller->moveObjects(Model::ObjectList(1, entity), delta, false);
        }
        
        Vec3 MapView::moveDirection(const Math::Direction direction) const {
            switch (direction) {
                case Math::Direction_Forward: {
                    Vec3 dir = m_camera.direction().firstAxis();
                    if (dir.z() < 0.0)
                        dir = m_camera.up().firstAxis();
                    else if (dir.z() > 0.0)
                        dir = -m_camera.up().firstAxis();
                    return dir;
                }
                case Math::Direction_Backward:
                    return -moveDirection(Math::Direction_Forward);
                case Math::Direction_Left:
                    return -moveDirection(Math::Direction_Right);
                case Math::Direction_Right: {
                    Vec3 dir = m_camera.right().firstAxis();
                    if (dir == moveDirection(Math::Direction_Forward))
                        dir = crossed(dir, Vec3::PosZ);
                    return dir;
                }
                case Math::Direction_Up:
                    return Vec3::PosZ;
                case Math::Direction_Down:
                    return Vec3::NegZ;
                    DEFAULT_SWITCH()
            }
        }
        
        Vec3f MapView::centerCameraOnObjectsPosition(const Model::EntityList& entities, const Model::BrushList& brushes) {
            Model::EntityList::const_iterator entityIt, entityEnd;
            Model::BrushList::const_iterator brushIt, brushEnd;
            
            float minDist = std::numeric_limits<float>::max();
            Vec3 center;
            size_t count = 0;
            
            for (entityIt = entities.begin(), entityEnd = entities.end(); entityIt != entityEnd; ++entityIt) {
                const Model::Entity* entity = *entityIt;
                if (entity->brushes().empty()) {
                    const Vec3::List vertices = bBoxVertices(entity->bounds());
                    for (size_t i = 0; i < vertices.size(); ++i) {
                        const Vec3f vertex(vertices[i]);
                        const Vec3f toPosition = vertex - m_camera.position();
                        minDist = std::min(minDist, toPosition.dot(m_camera.direction()));
                        center += vertices[i];
                        ++count;
                    }
                }
            }
            
            for (brushIt = brushes.begin(), brushEnd = brushes.end(); brushIt != brushEnd; ++brushIt) {
                const Model::Brush* brush = *brushIt;
                const Model::BrushVertexList& vertices = brush->vertices();
                for (size_t i = 0; i < vertices.size(); ++i) {
                    const Model::BrushVertex* vertex = vertices[i];
                    const Vec3f toPosition = Vec3f(vertex->position) - m_camera.position();
                    minDist = std::min(minDist, toPosition.dot(m_camera.direction()));
                    center += vertex->position;
                    ++count;
                }
            }
            
            center /= static_cast<FloatType>(count);
            
            // act as if the camera were there already:
            const Vec3f oldPosition = m_camera.position();
            m_camera.moveTo(Vec3f(center));
            
            float offset = std::numeric_limits<float>::max();
            
            Plane3f frustumPlanes[4];
            m_camera.frustumPlanes(frustumPlanes[0], frustumPlanes[1], frustumPlanes[2], frustumPlanes[3]);
            
            for (entityIt = entities.begin(), entityEnd = entities.end(); entityIt != entityEnd; ++entityIt) {
                const Model::Entity* entity = *entityIt;
                if (entity->brushes().empty()) {
                    const Vec3::List vertices = bBoxVertices(entity->bounds());
                    for (size_t i = 0; i < vertices.size(); ++i) {
                        const Vec3f vertex(vertices[i]);
                        
                        for (size_t j = 0; j < 4; ++j) {
                            const Plane3f& plane = frustumPlanes[j];
                            const float dist = (vertex - m_camera.position()).dot(plane.normal) - 8.0f; // adds a bit of a border
                            offset = std::min(offset, -dist / m_camera.direction().dot(plane.normal));
                        }
                    }
                }
            }
            
            for (brushIt = brushes.begin(), brushEnd = brushes.end(); brushIt != brushEnd; ++brushIt) {
                const Model::Brush* brush = *brushIt;
                const Model::BrushVertexList& vertices = brush->vertices();
                for (size_t i = 0; i < vertices.size(); ++i) {
                    const Model::BrushVertex* vertex = vertices[i];
                    
                    for (size_t j = 0; j < 4; ++j) {
                        const Plane3f& plane = frustumPlanes[j];
                        const float dist = (Vec3f(vertex->position) - m_camera.position()).dot(plane.normal) - 8.0f; // adds a bit of a border
                        offset = std::min(offset, -dist / m_camera.direction().dot(plane.normal));
                    }
                }
            }
            
            // jump back
            m_camera.moveTo(oldPosition);
            
            return center + m_camera.direction() * offset;
        }
        
        void MapView::createBrushEntity(const Assets::BrushEntityDefinition& definition) {
            MapDocumentSPtr document = lock(m_document);
            ControllerSPtr controller = lock(m_controller);
            
            const Model::BrushList brushes = document->selectedBrushes();
            assert(!brushes.empty());
            
            // if all brushes belong to the same entity, and that entity is not worldspawn, copy its properties
            Model::BrushList::const_iterator it = brushes.begin();
            Model::BrushList::const_iterator end = brushes.end();
            Model::Entity* entityTemplate = (*it++)->parent();
            while (it != end && entityTemplate != NULL)
                if ((*it++)->parent() != entityTemplate)
                    entityTemplate = NULL;
            
            Model::Entity* entity = document->map()->createEntity();
            if (entityTemplate != NULL && !entityTemplate->worldspawn())
                entity->setProperties(entityTemplate->properties());
            entity->addOrUpdateProperty(Model::PropertyKeys::Classname, definition.name());
            
            StringStream name;
            name << "Create " << definition.name();
            
            const UndoableCommandGroup commandGroup(controller, name.str());
            controller->deselectAll();
            controller->addEntity(entity);
            controller->reparentBrushes(brushes, entity);
            controller->selectObjects(VectorUtils::cast<Model::Object*>(brushes));
        }
        
        void MapView::resetCamera() {
            m_camera.setDirection(Vec3f(-1.0f, -1.0f, -0.65f).normalized(), Vec3f::PosZ);
            m_camera.moveTo(Vec3f(160.0f, 160.0f, 48.0f));
        }
        
        void MapView::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->documentWasNewedNotifier.addObserver(this, &MapView::documentWasNewedOrLoaded);
            document->documentWasLoadedNotifier.addObserver(this, &MapView::documentWasNewedOrLoaded);
            document->objectsWereAddedNotifier.addObserver(this, &MapView::objectsWereAdded);
            document->objectsWereRemovedNotifier.addObserver(this, &MapView::objectsWereRemoved);
            document->objectsDidChangeNotifier.addObserver(this, &MapView::objectsDidChange);
            document->faceDidChangeNotifier.addObserver(this, &MapView::faceDidChange);
            document->selectionDidChangeNotifier.addObserver(this, &MapView::selectionDidChange);
            document->modsDidChangeNotifier.addObserver(this, &MapView::modsDidChange);
            document->selectionDidChangeNotifier.addObserver(this, &MapView::selectionDidChange);
            document->grid().gridDidChangeNotifier.addObserver(this, &MapView::gridDidChange);
            
            ControllerSPtr controller = lock(m_controller);
            controller->commandDoneNotifier.addObserver(this, &MapView::commandDoneOrUndone);
            controller->commandUndoneNotifier.addObserver(this, &MapView::commandDoneOrUndone);
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &MapView::preferenceDidChange);
            
            m_camera.cameraDidChangeNotifier.addObserver(this, &MapView::cameraDidChange);
            
            m_toolBox.toolActivatedNotifier.addObserver(this, &MapView::toolActivated);
            m_toolBox.toolDeactivatedNotifier.addObserver(this, &MapView::toolDeactivated);
        }
        
        void MapView::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->documentWasNewedNotifier.removeObserver(this, &MapView::documentWasNewedOrLoaded);
                document->documentWasLoadedNotifier.removeObserver(this, &MapView::documentWasNewedOrLoaded);
                document->objectsWereAddedNotifier.removeObserver(this, &MapView::objectsWereAdded);
                document->objectsWereRemovedNotifier.removeObserver(this, &MapView::objectsWereRemoved);
                document->objectsDidChangeNotifier.removeObserver(this, &MapView::objectsDidChange);
                document->faceDidChangeNotifier.removeObserver(this, &MapView::faceDidChange);
                document->selectionDidChangeNotifier.removeObserver(this, &MapView::selectionDidChange);
                document->modsDidChangeNotifier.removeObserver(this, &MapView::modsDidChange);
                document->selectionDidChangeNotifier.removeObserver(this, &MapView::selectionDidChange);
                document->grid().gridDidChangeNotifier.removeObserver(this, &MapView::gridDidChange);
            }
            
            if (!expired(m_controller)) {
                ControllerSPtr controller = lock(m_controller);
                controller->commandDoneNotifier.removeObserver(this, &MapView::commandDoneOrUndone);
                controller->commandUndoneNotifier.removeObserver(this, &MapView::commandDoneOrUndone);
            }
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &MapView::preferenceDidChange);
            
            // Unfortunately due to the order in which objects and their fields are destroyed by the runtime system,
            // the camera has already been destroyed at this point.
            // m_camera.cameraDidChangeNotifier.removeObserver(this, &MapView::cameraDidChange);

            m_toolBox.toolActivatedNotifier.removeObserver(this, &MapView::toolActivated);
            m_toolBox.toolDeactivatedNotifier.removeObserver(this, &MapView::toolDeactivated);
        }
        
        void MapView::documentWasNewedOrLoaded() {
            resetCamera();
        }
        
        void MapView::objectsWereAdded(const Model::ObjectList& objects) {
            Refresh();
        }
        
        void MapView::objectsWereRemoved(const Model::ObjectParentList& objects) {
            Refresh();
        }

        void MapView::objectsDidChange(const Model::ObjectList& objects) {
            View::MapDocumentSPtr document = lock(m_document);
            if (document->hasSelectedObjects())
                m_selectionGuide.setBounds(lock(m_document)->selectionBounds());
        }
        
        void MapView::faceDidChange(Model::BrushFace* face) {
            Refresh();
        }
        
        void MapView::selectionDidChange(const Model::SelectionResult& result) {
            View::MapDocumentSPtr document = lock(m_document);
            if (document->hasSelectedObjects())
                m_selectionGuide.setBounds(lock(m_document)->selectionBounds());
            updateAcceleratorTable(HasFocus());
            Refresh();
        }
        
        void MapView::gridDidChange() {
            Refresh();
        }
        
        void MapView::modsDidChange() {
            Refresh();
        }
        
        void MapView::commandDoneOrUndone(Controller::Command::Ptr command) {
            m_toolBox.updateHits();
            Refresh();
        }
        
        void MapView::preferenceDidChange(const IO::Path& path) {
            Refresh();
        }
        
        void MapView::cameraDidChange(const Renderer::Camera* camera) {
            Refresh();
        }
        
        void MapView::createTools(wxBookCtrlBase* toolBook) {
            Renderer::TextureFont& font = defaultFont(contextHolder()->fontManager());
            
            m_cameraTool = new CameraTool(m_document, m_controller, m_camera);
            m_clipTool = new ClipTool(m_document, m_controller, m_camera);
            m_createBrushTool = new CreateBrushTool(m_document, m_controller, m_camera, font);
            m_createEntityTool = new CreateEntityTool(m_document, m_controller, m_camera, contextHolder()->fontManager());
            m_moveObjectsTool = new MoveObjectsTool(m_document, m_controller, m_movementRestriction);
            m_resizeBrushesTool = new ResizeBrushesTool(m_document, m_controller, m_camera);
            m_rotateObjectsTool = new RotateObjectsTool(m_document, m_controller, m_camera, m_movementRestriction, font);
            m_selectionTool = new SelectionTool(m_document, m_controller);
            m_setFaceAttribsTool = new SetFaceAttribsTool(m_document, m_controller);
            m_textureTool = new TextureTool(m_document, m_controller);
            m_vertexTool = new VertexTool(m_document, m_controller, m_movementRestriction, font);
            
            m_toolBox.addTool(m_cameraTool);
            m_toolBox.addTool(m_textureTool);
            m_toolBox.addTool(m_clipTool);
            m_toolBox.addTool(m_rotateObjectsTool);
            m_toolBox.addTool(m_vertexTool);
            m_toolBox.addTool(m_createEntityTool);
            m_toolBox.addTool(m_createBrushTool);
            m_toolBox.addTool(m_moveObjectsTool);
            m_toolBox.addTool(m_resizeBrushesTool);
            m_toolBox.addTool(m_setFaceAttribsTool);
            m_toolBox.addTool(m_selectionTool);
            
            m_toolBox.deactivateWhen(m_clipTool, m_resizeBrushesTool);
            
            m_moveObjectsTool->createPage(toolBook);
            m_rotateObjectsTool->createPage(toolBook);
        }
        
        void MapView::deleteTools() {
            delete m_cameraTool;
            m_cameraTool = NULL;
            delete m_clipTool;
            m_clipTool = NULL;
            delete m_createBrushTool;
            m_createBrushTool = NULL;
            delete m_createEntityTool;
            m_createEntityTool = NULL;
            delete m_moveObjectsTool;
            m_moveObjectsTool = NULL;
            delete m_resizeBrushesTool;
            m_resizeBrushesTool = NULL;
            delete m_rotateObjectsTool;
            m_rotateObjectsTool = NULL;
            delete m_selectionTool;
            m_selectionTool = NULL;
            delete m_setFaceAttribsTool;
            m_setFaceAttribsTool = NULL;
            delete m_textureTool;
            m_textureTool = NULL;
            delete m_vertexTool;
            m_vertexTool = NULL;
        }
        
        void MapView::toolActivated(Tool* tool) {
            if (tool == m_rotateObjectsTool)
                m_rotateObjectsTool->showPage();
        }
        
        void MapView::toolDeactivated(Tool* tool) {
            m_moveObjectsTool->showPage();
        }

        void MapView::doUpdateViewport(int x, int y, int width, int height) {
            const Renderer::Camera::Viewport viewport(x, y, width, height);
            m_camera.setViewport(viewport);
        }
        
        void MapView::doInitializeGL() {
            const wxString vendor   = wxString::FromUTF8(reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
            const wxString renderer = wxString::FromUTF8(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
            const wxString version  = wxString::FromUTF8(reinterpret_cast<const char*>(glGetString(GL_VERSION)));
            
            m_logger->info(wxString::Format(L"Renderer info: %s version %s from %s", renderer, version, vendor));
            m_logger->info("Depth buffer bits: %d", depthBits());
            
            if (multisample())
                m_logger->info("Multisampling enabled");
            else
                m_logger->info("Multisampling disabled");
            
            Renderer::SetVboState setVboState(m_vbo);
            setVboState.mapped();
            m_compass.prepare(m_vbo);
        }
        
        void MapView::doRender() {
            MapDocumentSPtr document = lock(m_document);
            document->commitPendingRenderStateChanges();
            
            const View::Grid& grid = document->grid();
            Renderer::RenderContext context(m_camera, contextHolder()->shaderManager(), grid.visible(), grid.actualSize());
            
            setupGL(context);
            setRenderOptions(context);
            renderMap(context);
            renderSelectionGuide(context);
            renderToolBox(context);
            renderCoordinateSystem(context);
            renderCompass(context);
        }
        
        void MapView::setupGL(Renderer::RenderContext& context) {
            const Renderer::Camera::Viewport& viewport = context.camera().viewport();
            glViewport(viewport.x, viewport.y, viewport.width, viewport.height);
            
            glEnable(GL_MULTISAMPLE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glShadeModel(GL_SMOOTH);
        }
        
        void MapView::setRenderOptions(Renderer::RenderContext& context) {
            m_toolBox.setRenderOptions(context);
            if (cameraFlyModeActive())
                context.setHideMouseIndicators();
        }
        
        void MapView::renderCoordinateSystem(Renderer::RenderContext& context) {
            PreferenceManager& prefs = PreferenceManager::instance();
            const Color& xColor = prefs.get(Preferences::XAxisColor);
            const Color& yColor = prefs.get(Preferences::YAxisColor);
            const Color& zColor = prefs.get(Preferences::ZAxisColor);
            
            Renderer::ActiveShader shader(context.shaderManager(), Renderer::Shaders::VaryingPCShader);
            Renderer::SetVboState setVboState(m_vbo);
            setVboState.active();
            glDisable(GL_DEPTH_TEST);
            renderCoordinateSystem(Color(xColor, 0.3f), Color(yColor, 0.3f), Color(zColor, 0.3f));
            glEnable(GL_DEPTH_TEST);
            renderCoordinateSystem(xColor, yColor, zColor);
        }
        
        void MapView::renderCoordinateSystem(const Color& xColor, const Color& yColor, const Color& zColor) {
            MapDocumentSPtr document = lock(m_document);
            Renderer::VertexSpecs::P3C4::Vertex::List vertices = Renderer::coordinateSystem(document->worldBounds(), xColor, yColor, zColor);
            Renderer::VertexArray array = Renderer::VertexArray::swap(GL_LINES, vertices);
            
            Renderer::SetVboState setVboState(m_vbo);
            setVboState.mapped();
            array.prepare(m_vbo);
            setVboState.active();
            array.render();
        }
        
        void MapView::renderMap(Renderer::RenderContext& context) {
            m_renderer.render(context);
        }
        
        void MapView::renderSelectionGuide(Renderer::RenderContext& context) {
            if (context.showSelectionGuide() && !expired(m_document)) {
                View::MapDocumentSPtr document = lock(m_document);
                if (document->hasSelectedObjects()) {
                    PreferenceManager& prefs = PreferenceManager::instance();
                    const Color& color = prefs.get(Preferences::HandleColor);
                    m_selectionGuide.setColor(color);
                    m_selectionGuide.render(context);
                }
            }
        }
        
        void MapView::renderToolBox(Renderer::RenderContext& context) {
            m_toolBox.renderTools(context);
        }
        
        void MapView::renderCompass(Renderer::RenderContext& context) {
            Renderer::SetVboState setVboState(m_vbo);
            setVboState.active();
            m_compass.render(context, m_movementRestriction);
        }
        
        Ray3 MapView::doGetPickRay(const int x, const int y) const {
            return m_camera.pickRay(x, y);
        }
        
        Hits MapView::doPick(const Ray3& pickRay) const {
            MapDocumentSPtr document = lock(m_document);
            return document->pick(pickRay);
        }
        
        void MapView::doShowPopupMenu() {
            MapDocumentSPtr document = lock(m_document);
            Assets::EntityDefinitionManager& manager = document->entityDefinitionManager();
            const Assets::EntityDefinitionGroups pointGroups = manager.groups(Assets::EntityDefinition::Type_PointEntity);
            const Assets::EntityDefinitionGroups brushGroups = manager.groups(Assets::EntityDefinition::Type_BrushEntity);
            
            wxMenu menu;
            menu.SetEventHandler(this);
            menu.Append(CommandIds::CreateEntityPopupMenu::ReparentBrushes, "Move Brushes to...");
            menu.Append(CommandIds::CreateEntityPopupMenu::MoveBrushesToWorld, "Move Brushes to World");
            menu.AppendSeparator();
            menu.AppendSubMenu(makeEntityGroupsMenu(pointGroups, CommandIds::CreateEntityPopupMenu::LowestPointEntityItem), "Create Point Entity");
            menu.AppendSubMenu(makeEntityGroupsMenu(brushGroups, CommandIds::CreateEntityPopupMenu::LowestBrushEntityItem), "Create Brush Entity");
            
            menu.UpdateUI(this);
            PopupMenu(&menu);
        }
        
        wxMenu* MapView::makeEntityGroupsMenu(const Assets::EntityDefinitionGroups& groups, int id) {
            wxMenu* menu = new wxMenu();
            Assets::EntityDefinitionGroups::const_iterator gIt, gEnd;
            for (gIt = groups.begin(), gEnd = groups.end(); gIt != gEnd; ++gIt) {
                const String& groupName = gIt->first;
                const Assets::EntityDefinitionList& definitions = gIt->second;
                
                wxMenu* groupMenu = new wxMenu();
                groupMenu->SetEventHandler(this);
                
                Assets::EntityDefinitionList::const_iterator dIt, dEnd;
                for (dIt = definitions.begin(), dEnd = definitions.end(); dIt != dEnd; ++dIt) {
                    const Assets::EntityDefinition* definition = *dIt;
                    if (definition->name() != Model::PropertyValues::WorldspawnClassname)
                        groupMenu->Append(id++, definition->shortName());
                }
                
                menu->AppendSubMenu(groupMenu, groupName);
            }
            return menu;
        }
        
        void MapView::bindEvents() {
            Bind(wxEVT_KEY_DOWN, &MapView::OnKey, this);
            Bind(wxEVT_KEY_UP, &MapView::OnKey, this);
            
            Bind(wxEVT_SET_FOCUS, &MapView::OnSetFocus, this);
            Bind(wxEVT_KILL_FOCUS, &MapView::OnKillFocus, this);
            
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnToggleClipTool,           this, CommandIds::Actions::ToggleClipTool);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnToggleClipSide,           this, CommandIds::Actions::ToggleClipSide);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnPerformClip,              this, CommandIds::Actions::PerformClip);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnDeleteLastClipPoint,      this, CommandIds::Actions::DeleteLastClipPoint);
            
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnToggleVertexTool,         this, CommandIds::Actions::ToggleVertexTool);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnMoveVerticesForward,      this, CommandIds::Actions::MoveVerticesForward);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnMoveVerticesBackward,     this, CommandIds::Actions::MoveVerticesBackward);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnMoveVerticesLeft,         this, CommandIds::Actions::MoveVerticesLeft);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnMoveVerticesRight,        this, CommandIds::Actions::MoveVerticesRight);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnMoveVerticesUp,           this, CommandIds::Actions::MoveVerticesUp);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnMoveVerticesDown,         this, CommandIds::Actions::MoveVerticesDown);
            
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnToggleRotateObjectsTool,  this, CommandIds::Actions::ToggleRotateObjectsTool);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnToggleFlyMode,            this, CommandIds::Actions::ToggleFlyMode);
            
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnToggleMovementRestriction,this, CommandIds::Actions::ToggleMovementRestriction);
            
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnDeleteObjects,            this, CommandIds::Actions::DeleteObjects);
            
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnMoveObjectsForward,       this, CommandIds::Actions::MoveObjectsForward);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnMoveObjectsBackward,      this, CommandIds::Actions::MoveObjectsBackward);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnMoveObjectsLeft,          this, CommandIds::Actions::MoveObjectsLeft);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnMoveObjectsRight,         this, CommandIds::Actions::MoveObjectsRight);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnMoveObjectsUp,            this, CommandIds::Actions::MoveObjectsUp);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnMoveObjectsDown,          this, CommandIds::Actions::MoveObjectsDown);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnRollObjectsCW,            this, CommandIds::Actions::RollObjectsCW);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnRollObjectsCCW,           this, CommandIds::Actions::RollObjectsCCW);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnPitchObjectsCW,           this, CommandIds::Actions::PitchObjectsCW);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnPitchObjectsCCW,          this, CommandIds::Actions::PitchObjectsCCW);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnYawObjectsCW,             this, CommandIds::Actions::YawObjectsCW);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnYawObjectsCCW,            this, CommandIds::Actions::YawObjectsCCW);
            
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnFlipObjectsH,             this, CommandIds::Actions::FlipObjectsHorizontally);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnFlipObjectsV,             this, CommandIds::Actions::FlipObjectsVertically);
            
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnDuplicateObjectsForward,  this, CommandIds::Actions::DuplicateObjectsForward);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnDuplicateObjectsBackward, this, CommandIds::Actions::DuplicateObjectsBackward);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnDuplicateObjectsLeft,     this, CommandIds::Actions::DuplicateObjectsLeft);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnDuplicateObjectsRight,    this, CommandIds::Actions::DuplicateObjectsRight);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnDuplicateObjectsUp,       this, CommandIds::Actions::DuplicateObjectsUp);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnDuplicateObjectsDown,     this, CommandIds::Actions::DuplicateObjectsDown);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnDuplicateObjects,         this, CommandIds::Actions::DuplicateObjects);
            
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnMoveTexturesUp,           this, CommandIds::Actions::MoveTexturesUp);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnMoveTexturesDown,         this, CommandIds::Actions::MoveTexturesDown);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnMoveTexturesLeft,         this, CommandIds::Actions::MoveTexturesLeft);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnMoveTexturesRight,        this, CommandIds::Actions::MoveTexturesRight);
            
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnRotateTexturesCW,         this, CommandIds::Actions::RotateTexturesCW);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnRotateTexturesCCW,        this, CommandIds::Actions::RotateTexturesCCW);
            
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnPopupReparentBrushes,     this, CommandIds::CreateEntityPopupMenu::ReparentBrushes);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnPopupMoveBrushesToWorld,  this, CommandIds::CreateEntityPopupMenu::MoveBrushesToWorld);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnPopupCreatePointEntity,   this, CommandIds::CreateEntityPopupMenu::LowestPointEntityItem, CommandIds::CreateEntityPopupMenu::HighestPointEntityItem);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnPopupCreateBrushEntity,   this, CommandIds::CreateEntityPopupMenu::LowestBrushEntityItem, CommandIds::CreateEntityPopupMenu::HighestBrushEntityItem);
            
            Bind(wxEVT_UPDATE_UI, &MapView::OnUpdatePopupMenuItem, this, CommandIds::CreateEntityPopupMenu::ReparentBrushes);
            Bind(wxEVT_UPDATE_UI, &MapView::OnUpdatePopupMenuItem, this, CommandIds::CreateEntityPopupMenu::MoveBrushesToWorld);
            Bind(wxEVT_UPDATE_UI, &MapView::OnUpdatePopupMenuItem, this, CommandIds::CreateEntityPopupMenu::LowestPointEntityItem, CommandIds::CreateEntityPopupMenu::HighestPointEntityItem);
            Bind(wxEVT_UPDATE_UI, &MapView::OnUpdatePopupMenuItem, this, CommandIds::CreateEntityPopupMenu::LowestBrushEntityItem, CommandIds::CreateEntityPopupMenu::HighestBrushEntityItem);
            
            wxFrame* frame = findFrame(this);
            frame->Bind(wxEVT_ACTIVATE, &MapView::OnActivateFrame, this);
        }
        
        const GLContextHolder::GLAttribs& MapView::attribs() {
            static bool initialized = false;
            static GLContextHolder::GLAttribs attribs;
            if (initialized)
                return attribs;
            
            int testAttribs[] =
            {
                // 32 bit depth buffer, 4 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       32,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          4,
                0,
                // 24 bit depth buffer, 4 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       24,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          4,
                0,
                // 32 bit depth buffer, 2 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       32,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          2,
                0,
                // 24 bit depth buffer, 2 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       24,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          2,
                0,
                // 16 bit depth buffer, 4 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       16,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          4,
                0,
                // 16 bit depth buffer, 2 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       16,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          2,
                0,
                // 32 bit depth buffer, no multisampling
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       32,
                0,
                // 24 bit depth buffer, no multisampling
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       24,
                0,
                // 16 bit depth buffer, no multisampling
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       16,
                0,
                0,
            };
            
            size_t index = 0;
            while (!initialized && testAttribs[index] != 0) {
                size_t count = 0;
                for (; testAttribs[index + count] != 0; ++count);
                if (wxGLCanvas::IsDisplaySupported(&testAttribs[index])) {
                    for (size_t i = 0; i < count; ++i)
                        attribs.push_back(testAttribs[index + i]);
                    attribs.push_back(0);
                    initialized = true;
                }
                index += count + 1;
            }
            
            assert(initialized);
            assert(!attribs.empty());
            return attribs;
        }
        
        int MapView::depthBits() {
            return attribs()[3];
        }
        
        bool MapView::multisample() {
            return attribs()[4] != 0;
        }
        
        Renderer::TextureFont& MapView::defaultFont(Renderer::FontManager& fontManager) {
            PreferenceManager& prefs = PreferenceManager::instance();
            const IO::Path& fontPath = prefs.get(Preferences::RendererFontPath());
            const size_t fontSize = static_cast<size_t>(prefs.get(Preferences::RendererFontSize));
            return fontManager.font(Renderer::FontDescriptor(fontPath, fontSize));
        }
    }
}
