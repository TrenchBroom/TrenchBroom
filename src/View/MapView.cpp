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
#include "Notifier.h"
#include "Preferences.h"
#include "Model/Brush.h"
#include "Model/BrushVertex.h"
#include "Model/Entity.h"
#include "Model/HitAdapter.h"
#include "Model/HitFilters.h"
#include "Model/Map.h"
#include "Model/Object.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Transformation.h"
#include "Renderer/Vertex.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexSpec.h"
#include "View/Animation.h"
#include "View/CameraAnimation.h"
#include "View/CameraTool.h"
#include "View/ClipTool.h"
#include "View/CommandIds.h"
#include "View/CreateBrushTool.h"
#include "View/CreateEntityTool.h"
#include "View/MapDocument.h"
#include "View/MapViewDropTarget.h"
#include "View/MoveObjectsTool.h"
#include "View/ResizeBrushesTool.h"
#include "View/RotateObjectsTool.h"
#include "View/SelectionTool.h"
#include "View/VertexTool.h"

#include <wx/dcclient.h>
#include <wx/menu.h>
#include <wx/settings.h>
#include <wx/timer.h>

namespace TrenchBroom {
    namespace View {
        MapView::MapView(wxWindow* parent, Logger* logger, View::MapDocumentWPtr document, ControllerWPtr controller) :
        wxGLCanvas(parent, wxID_ANY, &attribs().front()),
        m_logger(logger),
        m_initialized(false),
        m_glContext(new wxGLContext(this)),
        m_auxVbo(0xFFF),
        m_document(document),
        m_controller(controller),
        m_renderResources(attribs(), m_glContext),
        m_renderer(m_document, m_renderResources.fontManager()),
        m_compass(),
        m_selectionGuide(defaultFont(m_renderResources)),
        m_animationManager(new AnimationManager()),
        m_inputState(m_camera),
        m_cameraTool(NULL),
        m_clipTool(NULL),
        m_createBrushTool(NULL),
        m_createEntityTool(NULL),
        m_moveObjectsTool(NULL),
        m_vertexTool(NULL),
        m_resizeBrushesTool(NULL),
        m_rotateObjectsTool(NULL),
        m_selectionTool(NULL),
        m_toolChain(NULL),
        m_dragReceiver(NULL),
        m_modalReceiver(NULL),
        m_dropReceiver(NULL),
        m_savedDropReceiver(NULL),
        m_ignoreNextDrag(false),
        m_ignoreNextClick(false),
        m_lastFrameActivation(wxDateTime::Now()) {
            m_camera.setDirection(Vec3f(-1.0f, -1.0f, -0.65f).normalized(), Vec3f::PosZ);
            m_camera.moveTo(Vec3f(160.0f, 160.0f, 48.0f));

            const wxColour color = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
            const float r = static_cast<float>(color.Red()) / 0xFF;
            const float g = static_cast<float>(color.Green()) / 0xFF;
            const float b = static_cast<float>(color.Blue()) / 0xFF;
            const float a = 1.0f;
            m_focusColor = Color(r, g, b, a);

            createTools();
            bindEvents();
            bindObservers();
            
            SetDropTarget(new MapViewDropTarget(this));
        }
        
        MapView::~MapView() {
            m_animationManager->Delete();
            m_animationManager = NULL;
            
            unbindObservers();
            deleteTools();
            delete m_glContext;
            m_glContext = NULL;
            m_logger = NULL;
        }
        
        Renderer::RenderResources& MapView::renderResources() {
            return m_renderResources;
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
            CameraAnimation* animation = new CameraAnimation(*this, m_camera, position, direction, up, duration);
            m_animationManager->runAnimation(animation, true);
        }

        bool MapView::anyToolActive() const {
            return m_modalReceiver != NULL;
        }
        
        void MapView::deactivateAllTools() {
            toggleTool(NULL);
        }

        void MapView::toggleClipTool() {
            toggleTool(m_clipTool);
        }
        
        bool MapView::clipToolActive() const {
            return m_modalReceiver == m_clipTool;
        }

        bool MapView::canToggleClipSide() const {
            assert(clipToolActive());
            return m_clipTool->canToggleClipSide();
        }

        void MapView::toggleClipSide() {
            assert(clipToolActive());
            m_clipTool->toggleClipSide();
            Refresh();
        }
        
        bool MapView::canPerformClip() const {
            assert(clipToolActive());
            return m_clipTool->canPerformClip();
        }

        void MapView::performClip() {
            assert(clipToolActive());
            m_clipTool->performClip();
            Refresh();
        }

        bool MapView::canDeleteLastClipPoint() const {
            assert(clipToolActive());
            return m_clipTool->canDeleteLastClipPoint();
        }
        
        void MapView::deleteLastClipPoint() {
            assert(clipToolActive());
            m_clipTool->deleteLastClipPoint();
            Refresh();
        }

        void MapView::toggleRotateObjectsTool() {
            toggleTool(m_rotateObjectsTool);
        }
        
        bool MapView::rotateObjectsToolActive() const {
            return m_modalReceiver == m_rotateObjectsTool;
        }

        void MapView::toggleVertexTool() {
            toggleTool(m_vertexTool);
        }
        
        bool MapView::vertexToolActive() const {
            return m_modalReceiver == m_vertexTool;
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
        }

        void MapView::toggleMovementRestriction() {
            m_movementRestriction.toggleHorizontalRestriction(m_camera);
            Refresh();
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
        
        void MapView::rotateObjects(const RotationAxis axisSpec, const bool clockwise) {
            MapDocumentSPtr document = lock(m_document);
            const Model::ObjectList& objects = document->selectedObjects();
            if (objects.empty())
                return;
            
            Vec3 axis;
            switch (axisSpec) {
                case RARoll:
                    axis = moveDirection(Math::DForward);
                    break;
                case RAPitch:
                    axis = moveDirection(Math::DRight);
                    break;
                case RAYaw:
                    axis = Vec3::PosZ;
                    break;
            }
            
            if (!clockwise)
                axis *= -1.0;
            
            ControllerSPtr controller = lock(m_controller);
            const Grid& grid = document->grid();
            const Vec3 center = grid.referencePoint(document->selectionBounds());
            controller->rotateObjects(objects, center, axis, Math::Constants<FloatType>::PiOverTwo, document->textureLock());
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
        
        void MapView::moveTextures(const Math::Direction direction, const bool snapToGrid) {
            MapDocumentSPtr document = lock(m_document);
            const Model::BrushFaceList& faces = document->allSelectedFaces();
            if (faces.empty())
                return;
            
            const Grid& grid = document->grid();
            const float distance = snapToGrid ? static_cast<float>(grid.actualSize()) : 1.0f;
            
            ControllerSPtr controller = lock(m_controller);
            controller->moveTextures(faces, m_camera.up(), m_camera.right(), direction, distance);
        }
        
        void MapView::moveVertices(const Math::Direction direction) {
            assert(vertexToolActive());
            const Grid& grid = lock(m_document)->grid();
            const Vec3 delta = moveDirection(direction) * static_cast<FloatType>(grid.actualSize());
            m_vertexTool->moveVerticesAndRebuildBrushGeometry(delta);
        }
        
        Vec3 MapView::pasteObjectsDelta(const BBox3& bounds) const {
            MapDocumentSPtr document = lock(m_document);
            
            const Grid& grid = document->grid();
            const wxMouseState mouseState = wxGetMouseState();
            const wxPoint clientCoords = ScreenToClient(mouseState.GetPosition());
            if (HitTest(clientCoords) == wxHT_WINDOW_INSIDE) {
                const Ray3f pickRay = m_camera.pickRay(clientCoords.x, clientCoords.y);
                Model::PickResult pickResult = document->pick(Ray3(pickRay));
                pickResult.sortHits();
                
                const Model::PickResult::FirstHit first = Model::firstHit(pickResult, Model::Brush::BrushHit, document->filter(), true);
                if (first.matches) {
                    const Model::BrushFace* face = Model::hitAsFace(first.hit);
                    const Vec3 snappedHitPoint = grid.snap(first.hit.hitPoint());
                    return grid.moveDeltaForBounds(face, bounds, document->worldBounds(), pickRay, snappedHitPoint);
                } else {
                    const Vec3 snappedCenter = grid.snap(bounds.center());
                    const Vec3 snappedDefaultPoint = grid.snap(m_camera.defaultPoint(pickRay.direction));
                    return snappedDefaultPoint - snappedCenter;
                }
            } else {
                const Vec3 snappedCenter = grid.snap(bounds.center());
                const Vec3 snappedDefaultPoint = grid.snap(m_camera.defaultPoint());
                return snappedDefaultPoint - snappedCenter;
            }
        }

        bool MapView::dragEnter(const wxCoord x, const wxCoord y, const String& text) {
            assert(m_dropReceiver == NULL);
            
            deactivateAllTools();
            m_inputState.mouseMove(x, y);
            updatePickResults(x, y);
            m_createEntityTool->activate(m_inputState);
            m_dropReceiver = m_toolChain->dragEnter(m_inputState, text);
            Refresh();
            
            return m_dropReceiver != NULL;
        }
        
        bool MapView::dragMove(const wxCoord x, const wxCoord y, const String& text) {
            if (m_dropReceiver == NULL)
                return false;
            
            m_inputState.mouseMove(x, y);
            updatePickResults(x, y);
            m_dropReceiver->dragMove(m_inputState);
            Refresh();
            
            return true;
        }
        
        void MapView::dragLeave() {
            if (m_dropReceiver == NULL)
                return;

            // This is a workaround for a bug in wxWidgets 3.0.0 on GTK2, where a drag leave event
            // is sent right before the drop event. So we save the drag receiver in an instance variable
            // and if dragDrop() is called, it can use that variable to find out who the drop receiver is.
            m_savedDropReceiver = m_dropReceiver;

            m_dropReceiver->dragLeave(m_inputState);
            m_createEntityTool->deactivate(m_inputState);
            m_dropReceiver = NULL;
            Refresh();
        }
        
        bool MapView::dragDrop(const wxCoord x, const wxCoord y, const String& text) {
            if (m_dropReceiver == NULL && m_savedDropReceiver == NULL)
                return false;
            
            if (m_dropReceiver == NULL) {
                m_dropReceiver = m_savedDropReceiver;
                m_dropReceiver->activate(m_inputState); // GTK2 fix: has been deactivated by dragLeave()
                m_dropReceiver->dragEnter(m_inputState, text);
            }

            updatePickResults(x, y);
            const bool success = m_dropReceiver->dragDrop(m_inputState);
            m_dropReceiver->deactivate(m_inputState);
            m_dropReceiver = NULL;
            m_savedDropReceiver = NULL;
            Refresh();
            
            return success;
        }

        void MapView::OnKey(wxKeyEvent& event) {
            if (updateModifierKeys()) {
                m_movementRestriction.setVerticalRestriction(m_inputState.modifierKeysDown(ModifierKeys::MKAlt));
                updatePickResults(event.GetX(), event.GetY());
                m_toolChain->modifierKeyChange(m_inputState);
            }
            Refresh();
            event.Skip();
        }
        
        void MapView::OnMouseButton(wxMouseEvent& event) {
            const MouseButtonState button = mouseButton(event);

            if (m_ignoreNextClick && button == MouseButtons::MBLeft) {
                if (event.ButtonUp())
                    m_ignoreNextClick = false;
                event.Skip();
                return;
            }
            
            updateModifierKeys();
            if (event.ButtonDown()) {
                if (!HasCapture())
                    CaptureMouse();
                m_clickPos = event.GetPosition();
                m_inputState.mouseDown(button);
                m_toolChain->mouseDown(m_inputState);
            } else {
                if (m_dragReceiver != NULL) {
                    m_dragReceiver->endMouseDrag(m_inputState);
                    m_dragReceiver = NULL;

                    m_inputState.mouseUp(button);
                    if (HasCapture())
                        ReleaseMouse();
                } else if (!m_ignoreNextDrag) {
                    const bool handled = m_toolChain->mouseUp(m_inputState);

                    m_inputState.mouseUp(button);
                    if (HasCapture())
                        ReleaseMouse();

                    if (button == MouseButtons::MBRight && !handled)
                        showPopupMenu();
                } else {
                    m_inputState.mouseUp(button);
                    if (HasCapture())
                        ReleaseMouse();
                }
            }

            updatePickResults(event.GetX(), event.GetY());
            m_ignoreNextDrag = false;
            
            Refresh();
            event.Skip();
        }
        
        void MapView::OnMouseDoubleClick(wxMouseEvent& event) {
            const MouseButtonState button = mouseButton(event);
            updateModifierKeys();

            m_clickPos = event.GetPosition();
            m_inputState.mouseDown(button);
            m_toolChain->mouseDoubleClick(m_inputState);
            m_inputState.mouseUp(button);
            
            updatePickResults(event.GetX(), event.GetY());

            Refresh();
            event.Skip();
        }

        void MapView::OnMouseMotion(wxMouseEvent& event) {
            updateModifierKeys();
            updatePickResults(event.GetX(), event.GetY());
            if (m_dragReceiver != NULL) {
                m_inputState.mouseMove(event.GetX(), event.GetY());
                if (!m_dragReceiver->mouseDrag(m_inputState)) {
                    m_dragReceiver->endMouseDrag(m_inputState);
                    m_dragReceiver = NULL;
                    m_ignoreNextDrag = true;
                }
            } else if (!m_ignoreNextDrag) {
                if (m_inputState.mouseButtons() != MouseButtons::MBNone &&
                    (std::abs(event.GetX() - m_clickPos.x) > 1 ||
                     std::abs(event.GetY() - m_clickPos.y) > 1)) {
                        m_dragReceiver = m_toolChain->startMouseDrag(m_inputState);
                        if (m_dragReceiver == NULL)
                            m_ignoreNextDrag = true;
                    }
                if (m_dragReceiver != NULL) {
                    m_inputState.mouseMove(event.GetX(), event.GetY());
                    m_dragReceiver->mouseDrag(m_inputState);
                } else {
                    m_inputState.mouseMove(event.GetX(), event.GetY());
                    m_toolChain->mouseMove(m_inputState);
                }
            }
            Refresh();
            event.Skip();
        }
        
        void MapView::OnMouseWheel(wxMouseEvent& event) {
            updateModifierKeys();
            const float delta = static_cast<float>(event.GetWheelRotation()) / event.GetWheelDelta() * event.GetLinesPerAction();
            if (event.GetWheelAxis() == wxMOUSE_WHEEL_HORIZONTAL)
                m_inputState.scroll(delta, 0.0f);
            else if (event.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL)
                m_inputState.scroll(0.0f, delta);
            m_toolChain->scroll(m_inputState);

            updatePickResults(event.GetX(), event.GetY());
            Refresh();
            event.Skip();
        }

        void MapView::OnMouseCaptureLost(wxMouseCaptureLostEvent& event) {
            cancelCurrentDrag();
            Refresh();
            event.Skip();
        }

        void MapView::OnSetFocus(wxFocusEvent& event) {
            if (updateModifierKeys())
                m_toolChain->modifierKeyChange(m_inputState);
            Refresh();
            SetCursor(wxCursor(wxCURSOR_ARROW));
            
            // if this focus event happens as a result of a window activation, the don't ignore the next click
            if ((wxDateTime::Now() - m_lastFrameActivation).IsShorterThan(wxTimeSpan(0, 0, 0, 100)))
                m_ignoreNextClick = false;
            
            event.Skip();
        }
        
        void MapView::OnKillFocus(wxFocusEvent& event) {
            cancelCurrentDrag();
            if (GetCapture() == this)
                ReleaseMouse();
            if (clearModifierKeys())
                m_toolChain->modifierKeyChange(m_inputState);
            m_ignoreNextClick = true;
            Refresh();
            SetCursor(wxCursor(wxCURSOR_HAND));
            event.Skip();
        }

        void MapView::OnActivateFrame(wxActivateEvent& event) {
            m_lastFrameActivation = wxDateTime::Now();
        }

        void MapView::OnPaint(wxPaintEvent& event) {
#ifndef TESTING
            if (!IsShownOnScreen())
                return;
            
            if (!m_initialized)
                initializeGL();
            
            if (SetCurrent(*m_glContext)) {
                wxPaintDC paintDC(this);

                MapDocumentSPtr document = lock(m_document);
                document->commitPendingRenderStateChanges();
                { // new block to make sure that the render context is destroyed before SwapBuffers is called
                    const View::Grid& grid = document->grid();
                    Renderer::RenderContext context(m_camera, m_renderResources.shaderManager(), grid.visible(), grid.actualSize());
                    setupGL(context);
                    setRenderOptions(context);
                    clearBackground(context);
                    renderMap(context);
                    renderSelectionGuide(context);
                    renderTools(context);
                    renderCoordinateSystem(context);
                    renderCompass(context);
                    renderFocusRect(context);
                }
                SwapBuffers();
            }
#endif
        }

        void MapView::OnSize(wxSizeEvent& event) {
            const wxSize clientSize = GetClientSize();
            const Renderer::Camera::Viewport viewport(0, 0, clientSize.x, clientSize.y);
            m_camera.setViewport(viewport);
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
            const Assets::EntityDefinitionGroups groups = manager.groups(Assets::EntityDefinition::PointEntity);
            const size_t index = static_cast<size_t>(event.GetId() - CommandIds::CreateEntityPopupMenu::LowestPointEntityItem);
            const Assets::EntityDefinition* definition = findEntityDefinition(groups, index);
            assert(definition != NULL);
            assert(definition->type() == Assets::EntityDefinition::PointEntity);
            createPointEntity(*static_cast<const Assets::PointEntityDefinition*>(definition));
        }
        
        void MapView::OnPopupCreateBrushEntity(wxCommandEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            Assets::EntityDefinitionManager& manager = document->entityDefinitionManager();
            const Assets::EntityDefinitionGroups groups = manager.groups(Assets::EntityDefinition::BrushEntity);
            const size_t index = static_cast<size_t>(event.GetId() - CommandIds::CreateEntityPopupMenu::LowestBrushEntityItem);
            const Assets::EntityDefinition* definition = findEntityDefinition(groups, index);
            assert(definition != NULL);
            assert(definition->type() == Assets::EntityDefinition::BrushEntity);
            createBrushEntity(*static_cast<const Assets::BrushEntityDefinition*>(definition));
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
            const Model::PickResult::FirstHit first = Model::firstHit(m_inputState.pickResult(), Model::Entity::EntityHit | Model::Brush::BrushHit, document->filter(), true);
            if (first.matches) {
                if (first.hit.type() == Model::Entity::EntityHit) {
                    newParent = Model::hitAsEntity(first.hit);
                } else if (first.hit.type() == Model::Brush::BrushHit) {
                    const Model::Brush* brush = Model::hitAsBrush(first.hit);
                    newParent = brush->parent();
                }
            }
            
            if (newParent == NULL)
                return NULL;
            if (canReparentBrushes(brushes, newParent))
                return newParent;
            return NULL;
        }

        bool MapView::canReparentBrushes(const Model::BrushList& brushes, const Model::Entity* newParent) const {
            Model::BrushList::const_iterator it, end;
            for (it = brushes.begin(), end = brushes.end(); it != end; ++it) {
                const Model::Brush* brush = *it;
                if (brush->parent() != newParent)
                    return true;
            }
            return false;
        }

        // note that we make a copy of the brush list on purpose here
        void MapView::reparentBrushes(const Model::BrushList brushes, Model::Entity* newParent) {
            assert(newParent != NULL);
            assert(canReparentBrushes(brushes, newParent));
            
            ControllerSPtr controller = lock(m_controller);

            StringStream name;
            name << "Move " << (brushes.size() == 1 ? "Brush" : "Brushes") << " to " << newParent->classname("<missing classname>");
            
            controller->beginUndoableGroup(name.str());
            controller->deselectAll();
            controller->reparentBrushes(brushes, newParent);
            controller->selectObjects(VectorUtils::cast<Model::Object*>(brushes));
            controller->closeGroup();
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
            
            const Model::PickResult::FirstHit first = Model::firstHit(m_inputState.pickResult(), Model::Brush::BrushHit, document->filter(), true);
            if (first.matches) {
                delta = grid.moveDeltaForBounds(Model::hitAsFace(first.hit), definition.bounds(), document->worldBounds(), m_inputState.pickRay(), first.hit.hitPoint());
            } else {
                const Vec3 newPosition(m_camera.defaultPoint(m_inputState.pickRay().direction));
                delta = grid.moveDeltaForPoint(definition.bounds().center(), document->worldBounds(), newPosition - definition.bounds().center());
            }
            
            StringStream name;
            name << "Create " << definition.name();
            
            controller->beginUndoableGroup(name.str());
            controller->deselectAll();
            controller->addEntity(entity);
            controller->selectObject(entity);
            controller->moveObjects(Model::ObjectList(1, entity), delta, false);
            controller->closeGroup();
        }
        
        Vec3 MapView::moveDirection(const Math::Direction direction) const {
            switch (direction) {
                case Math::DForward: {
                    Vec3 dir = m_camera.direction().firstAxis();
                    if (dir.z() < 0.0)
                        dir = m_camera.up().firstAxis();
                    else if (dir.z() > 0.0)
                        dir = -m_camera.up().firstAxis();
                    return dir;
                }
                case Math::DBackward:
                    return -moveDirection(Math::DForward);
                case Math::DLeft:
                    return -moveDirection(Math::DRight);
                case Math::DRight: {
                    Vec3 dir = m_camera.right().firstAxis();
                    if (dir == moveDirection(Math::DForward))
                        dir = crossed(dir, Vec3::PosZ);
                    return dir;
                }
                case Math::DUp:
                    return Vec3::PosZ;
                case Math::DDown:
                    return Vec3::NegZ;
                default:
                    assert(false);
                    return Vec3::Null;
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
            
            controller->beginUndoableGroup(name.str());
            controller->deselectAll();
            controller->addEntity(entity);
            controller->reparentBrushes(brushes, entity);
            controller->selectObjects(VectorUtils::cast<Model::Object*>(brushes));
            controller->closeGroup();
        }

        void MapView::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->documentWasNewedNotifier.addObserver(this, &MapView::documentWasNewed);
            document->documentWasLoadedNotifier.addObserver(this, &MapView::documentWasLoaded);
            document->objectWasAddedNotifier.addObserver(this, &MapView::objectWasAdded);
            document->objectDidChangeNotifier.addObserver(this, &MapView::objectDidChange);
            document->faceDidChangeNotifier.addObserver(this, &MapView::faceDidChange);
            document->selectionDidChangeNotifier.addObserver(this, &MapView::selectionDidChange);
            document->modsDidChangeNotifier.addObserver(this, &MapView::modsDidChange);

            ControllerSPtr controller = lock(m_controller);
            controller->commandDoneNotifier.addObserver(this, &MapView::commandDoneOrUndone);
            controller->commandUndoneNotifier.addObserver(this, &MapView::commandDoneOrUndone);
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &MapView::preferenceDidChange);
        }
        
        void MapView::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->documentWasNewedNotifier.removeObserver(this, &MapView::documentWasNewed);
                document->documentWasLoadedNotifier.removeObserver(this, &MapView::documentWasLoaded);
                document->objectWasAddedNotifier.removeObserver(this, &MapView::objectWasAdded);
                document->objectDidChangeNotifier.removeObserver(this, &MapView::objectDidChange);
                document->faceDidChangeNotifier.removeObserver(this, &MapView::faceDidChange);
                document->selectionDidChangeNotifier.removeObserver(this, &MapView::selectionDidChange);
                document->modsDidChangeNotifier.removeObserver(this, &MapView::modsDidChange);
            }
            
            if (!expired(m_controller)) {
                ControllerSPtr controller = lock(m_controller);
                controller->commandDoneNotifier.removeObserver(this, &MapView::commandDoneOrUndone);
                controller->commandUndoneNotifier.removeObserver(this, &MapView::commandDoneOrUndone);
            }
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &MapView::preferenceDidChange);
        }

        void MapView::documentWasNewed() {
            Refresh();
        }
        
        void MapView::documentWasLoaded() {
            Refresh();
        }
        
        void MapView::objectWasAdded(Model::Object* object) {
            Refresh();
        }
        
        void MapView::objectDidChange(Model::Object* object) {
            View::MapDocumentSPtr document = lock(m_document);
            if (document->hasSelectedObjects())
                m_selectionGuide.setBounds(lock(m_document)->selectionBounds());
            Refresh();
        }
        
        void MapView::faceDidChange(Model::BrushFace* face) {
            Refresh();
        }
        
        void MapView::selectionDidChange(const Model::SelectionResult& result) {
            View::MapDocumentSPtr document = lock(m_document);
            if (document->hasSelectedObjects())
                m_selectionGuide.setBounds(lock(m_document)->selectionBounds());
            Refresh();
        }

        void MapView::commandDoneOrUndone(Controller::Command::Ptr command) {
            const wxMouseState mouseState = wxGetMouseState();
            const wxPoint clientPos = ScreenToClient(mouseState.GetPosition());
            updatePickResults(clientPos.x, clientPos.y);
            Refresh();
        }

        void MapView::modsDidChange() {
            Refresh();
        }

        void MapView::preferenceDidChange(const IO::Path& path) {
            Refresh();
        }

        void MapView::updatePickResults(const int x, const int y) {
            MapDocumentSPtr document = lock(m_document);

            m_inputState.setPickRay(m_camera.pickRay(x, y));
            Model::PickResult pickResult = document->pick(m_inputState.pickRay());
            m_toolChain->pick(m_inputState, pickResult);
            pickResult.sortHits();
            m_inputState.setPickResult(pickResult);
        }

        void MapView::createTools() {
            Renderer::TextureFont& font = defaultFont(m_renderResources);

            m_selectionTool = new SelectionTool(NULL, m_document, m_controller);
            m_resizeBrushesTool = new ResizeBrushesTool(m_selectionTool, m_document, m_controller);
            m_moveObjectsTool = new MoveObjectsTool(m_resizeBrushesTool, m_document, m_controller, m_movementRestriction);
            m_createBrushTool = new CreateBrushTool(m_moveObjectsTool, m_document, m_controller, font);
            m_createEntityTool = new CreateEntityTool(m_createBrushTool, m_document, m_controller, m_renderResources.fontManager());
            m_vertexTool = new VertexTool(m_createEntityTool, m_document, m_controller, m_movementRestriction, font);
            m_rotateObjectsTool = new RotateObjectsTool(m_vertexTool, m_document, m_controller, m_movementRestriction, font);
            m_clipTool = new ClipTool(m_rotateObjectsTool, m_document, m_controller, m_camera);
            m_cameraTool = new CameraTool(m_clipTool, m_document, m_controller, m_camera);
            m_toolChain = m_cameraTool;
        }
        
        void MapView::deleteTools() {
            m_toolChain = NULL;
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
            delete m_vertexTool;
            m_vertexTool = NULL;
        }

        void MapView::toggleTool(BaseTool* tool) {
            if (tool == NULL) {
                if (m_modalReceiver != NULL) {
                    m_modalReceiver->deactivate(m_inputState);
                    m_modalReceiver = NULL;
                }
            } else {
                if (m_modalReceiver == tool) {
                    assert(m_modalReceiver->active());
                    m_modalReceiver->deactivate(m_inputState);
                    m_modalReceiver = NULL;
                } else {
                    if (m_modalReceiver != NULL) {
                        assert(m_modalReceiver->active());
                        m_modalReceiver->deactivate(m_inputState);
                        m_modalReceiver = NULL;
                    }
                    if (tool->activate(m_inputState))
                        m_modalReceiver = tool;
                }
            }
            Refresh();
        }
        
        void MapView::cancelCurrentDrag() {
            if (m_dragReceiver != NULL) {
                m_toolChain->cancelMouseDrag(m_inputState);
                m_inputState.clearMouseButtons();
                m_dragReceiver = NULL;
            }
        }

        ModifierKeyState MapView::modifierKeys() {
            const wxMouseState mouseState = wxGetMouseState();
            
            ModifierKeyState state = ModifierKeys::MKNone;
            if (mouseState.CmdDown())
                state |= ModifierKeys::MKCtrlCmd;
            if (mouseState.ShiftDown())
                state |= ModifierKeys::MKShift;
            if (mouseState.AltDown())
                state |= ModifierKeys::MKAlt;
            return state;
        }
        
        bool MapView::updateModifierKeys() {
            const ModifierKeyState keys = modifierKeys();
            if (keys != m_inputState.modifierKeys()) {
                m_inputState.setModifierKeys(keys);
                return true;
            }
            return false;
        }
        
        bool MapView::clearModifierKeys() {
            if (m_inputState.modifierKeys() != ModifierKeys::MKNone) {
                m_inputState.setModifierKeys(ModifierKeys::MKNone);
                return true;
            }
            return false;
        }

        MouseButtonState MapView::mouseButton(wxMouseEvent& event) {
            switch (event.GetButton()) {
                case wxMOUSE_BTN_LEFT:
                    return MouseButtons::MBLeft;
                case wxMOUSE_BTN_MIDDLE:
                    return MouseButtons::MBMiddle;
                case wxMOUSE_BTN_RIGHT:
                    return MouseButtons::MBRight;
                default:
                    return MouseButtons::MBNone;
            }
        }

        void MapView::showPopupMenu() {
            MapDocumentSPtr document = lock(m_document);
            Assets::EntityDefinitionManager& manager = document->entityDefinitionManager();
            const Assets::EntityDefinitionGroups pointGroups = manager.groups(Assets::EntityDefinition::PointEntity);
            const Assets::EntityDefinitionGroups brushGroups = manager.groups(Assets::EntityDefinition::BrushEntity);
            
            wxMenu menu;
            menu.SetEventHandler(this);
            menu.Append(CommandIds::CreateEntityPopupMenu::ReparentBrushes, _("Move Brushes to..."));
            menu.Append(CommandIds::CreateEntityPopupMenu::MoveBrushesToWorld, _("Move Brushes to World"));
            menu.AppendSeparator();
            menu.AppendSubMenu(makeEntityGroupsMenu(pointGroups, CommandIds::CreateEntityPopupMenu::LowestPointEntityItem), _("Create Point Entity"));
            menu.AppendSubMenu(makeEntityGroupsMenu(brushGroups, CommandIds::CreateEntityPopupMenu::LowestBrushEntityItem), _("Create Brush Entity"));
            
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
            Bind(wxEVT_LEFT_DOWN, &MapView::OnMouseButton, this);
            Bind(wxEVT_LEFT_UP, &MapView::OnMouseButton, this);
            Bind(wxEVT_LEFT_DCLICK, &MapView::OnMouseDoubleClick, this);
            Bind(wxEVT_RIGHT_DOWN, &MapView::OnMouseButton, this);
            Bind(wxEVT_RIGHT_UP, &MapView::OnMouseButton, this);
            Bind(wxEVT_RIGHT_DCLICK, &MapView::OnMouseDoubleClick, this);
            Bind(wxEVT_MIDDLE_DOWN, &MapView::OnMouseButton, this);
            Bind(wxEVT_MIDDLE_UP, &MapView::OnMouseButton, this);
            Bind(wxEVT_MIDDLE_DCLICK, &MapView::OnMouseDoubleClick, this);
            Bind(wxEVT_AUX1_DOWN, &MapView::OnMouseButton, this);
            Bind(wxEVT_AUX1_UP, &MapView::OnMouseButton, this);
            Bind(wxEVT_AUX1_DCLICK, &MapView::OnMouseDoubleClick, this);
            Bind(wxEVT_AUX2_DOWN, &MapView::OnMouseButton, this);
            Bind(wxEVT_AUX2_UP, &MapView::OnMouseButton, this);
            Bind(wxEVT_AUX2_DCLICK, &MapView::OnMouseDoubleClick, this);
            Bind(wxEVT_MOTION, &MapView::OnMouseMotion, this);
            Bind(wxEVT_MOUSEWHEEL, &MapView::OnMouseWheel, this);
            Bind(wxEVT_SET_FOCUS, &MapView::OnSetFocus, this);
            Bind(wxEVT_KILL_FOCUS, &MapView::OnKillFocus, this);
            
            Bind(wxEVT_PAINT, &MapView::OnPaint, this);
            Bind(wxEVT_SIZE, &MapView::OnSize, this);
            
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnPopupReparentBrushes, this, CommandIds::CreateEntityPopupMenu::ReparentBrushes);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnPopupMoveBrushesToWorld, this, CommandIds::CreateEntityPopupMenu::MoveBrushesToWorld);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnPopupCreatePointEntity, this, CommandIds::CreateEntityPopupMenu::LowestPointEntityItem, CommandIds::CreateEntityPopupMenu::HighestPointEntityItem);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnPopupCreateBrushEntity, this, CommandIds::CreateEntityPopupMenu::LowestBrushEntityItem, CommandIds::CreateEntityPopupMenu::HighestBrushEntityItem);
            
            Bind(wxEVT_UPDATE_UI, &MapView::OnUpdatePopupMenuItem, this, CommandIds::CreateEntityPopupMenu::ReparentBrushes);
            Bind(wxEVT_UPDATE_UI, &MapView::OnUpdatePopupMenuItem, this, CommandIds::CreateEntityPopupMenu::MoveBrushesToWorld);
            Bind(wxEVT_UPDATE_UI, &MapView::OnUpdatePopupMenuItem, this, CommandIds::CreateEntityPopupMenu::LowestPointEntityItem, CommandIds::CreateEntityPopupMenu::HighestPointEntityItem);
            Bind(wxEVT_UPDATE_UI, &MapView::OnUpdatePopupMenuItem, this, CommandIds::CreateEntityPopupMenu::LowestBrushEntityItem, CommandIds::CreateEntityPopupMenu::HighestBrushEntityItem);
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
            m_toolChain->setRenderOptions(m_inputState, context);
        }

        void MapView::clearBackground(Renderer::RenderContext& context) {
            PreferenceManager& prefs = PreferenceManager::instance();
            const Color& backgroundColor = prefs.get(Preferences::BackgroundColor);
            glClearColor(backgroundColor.r(), backgroundColor.g(), backgroundColor.b(), backgroundColor.a());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        void MapView::renderCoordinateSystem(Renderer::RenderContext& context) {
            PreferenceManager& prefs = PreferenceManager::instance();
            const Color& xColor = prefs.get(Preferences::XAxisColor);
            const Color& yColor = prefs.get(Preferences::YAxisColor);
            const Color& zColor = prefs.get(Preferences::ZAxisColor);

            Renderer::ActiveShader shader(context.shaderManager(), Renderer::Shaders::VaryingPCShader);
            Renderer::SetVboState setVboState(m_auxVbo);
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
            
            Renderer::SetVboState setVboState(m_auxVbo);
            setVboState.mapped();
            array.prepare(m_auxVbo);
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
                    m_selectionGuide.render(context, document);
                }
            }
        }

        void MapView::renderTools(Renderer::RenderContext& context) {
            if (m_modalReceiver != NULL)
                m_modalReceiver->renderOnly(m_inputState, context);
            else
                m_toolChain->renderChain(m_inputState, context);
        }

        void MapView::renderCompass(Renderer::RenderContext& context) {
            Renderer::SetVboState setVboState(m_auxVbo);
            setVboState.active();
            m_compass.render(context, m_movementRestriction);
        }

        void MapView::renderFocusRect(Renderer::RenderContext& context) {
            if (!HasFocus())
                return;
            
            const Color& outer = m_focusColor;
            const Color inner(m_focusColor, 0.7f);
            const float w = static_cast<float>(context.camera().viewport().width);
            const float h = static_cast<float>(context.camera().viewport().height);
            const float t = 3.0f;
            
            typedef Renderer::VertexSpecs::P3C4::Vertex Vertex;
            Vertex::List vertices(16);
            
            // top
            vertices[ 0] = Vertex(Vec3f(0.0f, 0.0f, 0.0f), outer);
            vertices[ 1] = Vertex(Vec3f(w, 0.0f, 0.0f), outer);
            vertices[ 2] = Vertex(Vec3f(w-t, t, 0.0f), inner);
            vertices[ 3] = Vertex(Vec3f(t, t, 0.0f), inner);
            
            // right
            vertices[ 4] = Vertex(Vec3f(w, 0.0f, 0.0f), outer);
            vertices[ 5] = Vertex(Vec3f(w, h, 0.0f), outer);
            vertices[ 6] = Vertex(Vec3f(w-t, h-t, 0.0f), inner);
            vertices[ 7] = Vertex(Vec3f(w-t, t, 0.0f), inner);
            
            // bottom
            vertices[ 8] = Vertex(Vec3f(w, h, 0.0f), outer);
            vertices[ 9] = Vertex(Vec3f(0.0f, h, 0.0f), outer);
            vertices[10] = Vertex(Vec3f(t, h-t, 0.0f), inner);
            vertices[11] = Vertex(Vec3f(w-t, h-t, 0.0f), inner);
            
            // left
            vertices[12] = Vertex(Vec3f(0.0f, h, 0.0f), outer);
            vertices[13] = Vertex(Vec3f(0.0f, 0.0f, 0.0f), outer);
            vertices[14] = Vertex(Vec3f(t, t, 0.0f), inner);
            vertices[15] = Vertex(Vec3f(t, h-t, 0.0f), inner);
            
            const Mat4x4f projection = orthoMatrix(-1.0f, 1.0f, 0.0f, 0.0f, w, h);
            Renderer::ReplaceTransformation ortho(context.transformation(), projection, Mat4x4f::Identity);
            
            glDisable(GL_DEPTH_TEST);
            Renderer::VertexArray array = Renderer::VertexArray::swap(GL_QUADS, vertices);
            
            Renderer::SetVboState setVboState(m_auxVbo);
            setVboState.mapped();
            array.prepare(m_auxVbo);
            setVboState.active();
            array.render();
            glEnable(GL_DEPTH_TEST);
        }

        void MapView::initializeGL() {
            if (SetCurrent(*m_glContext)) {
                const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
                const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
                const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
                m_logger->info("Renderer info: %s version %s from %s", renderer, version, vendor);
                m_logger->info("Depth buffer bits: %d", depthBits());
                
                if (multisample())
                    m_logger->info("Multisampling enabled");
                else
                    m_logger->info("Multisampling disabled");
                
#ifndef TESTING
                glewExperimental = GL_TRUE;
                const GLenum glewState = glewInit();
                if (glewState != GLEW_OK)
                    m_logger->error("Error initializing glew: %s", glewGetErrorString(glewState));
#endif

                Renderer::SetVboState setVboState(m_auxVbo);
                setVboState.mapped();
                m_compass.prepare(m_auxVbo);
            } else {
                m_logger->info("Cannot set current GL context");
            }

            m_initialized = true;
        }
        
        const Renderer::RenderResources::GLAttribs& MapView::attribs() {
            static bool initialized = false;
            static Renderer::RenderResources::GLAttribs attribs;
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

        Renderer::TextureFont& MapView::defaultFont(Renderer::RenderResources& renderResources) {
            PreferenceManager& prefs = PreferenceManager::instance();
            const String& fontName = prefs.get(Preferences::RendererFontName);
            const size_t fontSize = static_cast<size_t>(prefs.get(Preferences::RendererFontSize));
            return renderResources.fontManager().font(Renderer::FontDescriptor(fontName, fontSize));
        }
    }
}
