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

#include "MapView3D.h"
#include "Logger.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/Entity.h"
#include "Model/HitAdapter.h"
#include "Model/HitQuery.h"
#include "Model/PickResult.h"
#include "Model/PointFile.h"
#include "Renderer/BoundsGuideRenderer.h"
#include "Renderer/Compass3D.h"
#include "Renderer/MapRenderer.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "Renderer/SelectionBoundsRenderer.h"
#include "View/ActionManager.h"
#include "View/Animation.h"
#include "View/CameraAnimation.h"
#include "View/CameraTool3D.h"
#include "View/ClipToolAdapter.h"
#include "View/CommandIds.h"
#include "View/CreateComplexBrushToolAdapter3D.h"
#include "View/CreateEntityToolAdapter.h"
#include "View/CreateSimpleBrushToolAdapter3D.h"
#include "View/FlashSelectionAnimation.h"
#include "View/FlyModeHelper.h"
#include "View/GLContextManager.h"
#include "View/Grid.h"
#include "View/InputState.h"
#include "View/MapDocument.h"
#include "View/MapViewToolBox.h"
#include "View/MoveObjectsToolAdapter.h"
#include "View/ResizeBrushesToolAdapter.h"
#include "View/RotateObjectsToolAdapter.h"
#include "View/SelectionTool.h"
#include "View/SetBrushFaceAttributesTool.h"
#include "View/VertexTool.h"
#include "View/VertexToolAdapter.h"
#include "View/wxUtils.h"

namespace TrenchBroom {
    namespace View {
        MapView3D::MapView3D(wxWindow* parent, Logger* logger, MapDocumentWPtr document, MapViewToolBox& toolBox, Renderer::MapRenderer& renderer, GLContextManager& contextManager) :
        MapViewBase(parent, logger, document, toolBox, renderer, contextManager),
        m_clipToolAdapter(NULL),
        m_createComplexBrushToolAdapter(NULL),
        m_createEntityToolAdapter(NULL),
        m_createSimpleBrushToolAdapter(NULL),
        m_moveObjectsToolAdapter(NULL),
        m_resizeBrushesToolAdapter(NULL),
        m_rotateObjectsToolAdapter(NULL),
        m_setBrushFaceAttributesTool(NULL),
        m_vertexToolAdapter(NULL),
        m_cameraTool(NULL),
        m_flyModeHelper(new FlyModeHelper(this, m_camera)) {
            bindEvents();
            bindObservers();
            initializeCamera();
            initializeToolChain(toolBox);
            setCompass(new Renderer::Compass3D(m_movementRestriction));
        }

        MapView3D::~MapView3D() {
            m_flyModeHelper->Delete();
            destroyToolChain();
            unbindObservers();
        }
        
        void MapView3D::initializeCamera() {
            m_camera.moveTo(Vec3f(-80.0f, -128.0f, 96.0f));
            m_camera.lookAt(Vec3::Null, Vec3::PosZ);
        }

        void MapView3D::initializeToolChain(MapViewToolBox& toolBox) {
            const Grid& grid = lock(m_document)->grid();
            m_clipToolAdapter = new ClipToolAdapter3D(toolBox.clipTool(), grid);
            m_createComplexBrushToolAdapter = new CreateComplexBrushToolAdapter3D(toolBox.createComplexBrushTool(), m_document);
            m_createEntityToolAdapter = new CreateEntityToolAdapter3D(toolBox.createEntityTool());
            m_createSimpleBrushToolAdapter = new CreateSimpleBrushToolAdapter3D(toolBox.createSimpleBrushTool(), m_document);
            m_moveObjectsToolAdapter = new MoveObjectsToolAdapter3D(toolBox.moveObjectsTool(), m_movementRestriction);
            m_resizeBrushesToolAdapter = new ResizeBrushesToolAdapter3D(toolBox.resizeBrushesTool());
            m_rotateObjectsToolAdapter = new RotateObjectsToolAdapter3D(toolBox.rotateObjectsTool(), m_movementRestriction);
            m_setBrushFaceAttributesTool = new SetBrushFaceAttributesTool(m_document);
            m_vertexToolAdapter = new VertexToolAdapter3D(toolBox.vertexTool(), m_movementRestriction);
            m_cameraTool = new CameraTool3D(m_document, m_camera);
            
            addTool(m_cameraTool);
            addTool(m_moveObjectsToolAdapter);
            addTool(m_rotateObjectsToolAdapter);
            addTool(m_resizeBrushesToolAdapter);
            addTool(m_createComplexBrushToolAdapter);
            addTool(m_clipToolAdapter);
            addTool(m_vertexToolAdapter);
            addTool(m_createEntityToolAdapter);
            addTool(m_setBrushFaceAttributesTool);
            addTool(toolBox.selectionTool());
            addTool(m_createSimpleBrushToolAdapter);
        }

        void MapView3D::destroyToolChain() {
            delete m_cameraTool;
            delete m_vertexToolAdapter;
            delete m_setBrushFaceAttributesTool;
            delete m_rotateObjectsToolAdapter;
            delete m_resizeBrushesToolAdapter;
            delete m_moveObjectsToolAdapter;
            delete m_createSimpleBrushToolAdapter;
            delete m_createEntityToolAdapter;
            delete m_createComplexBrushToolAdapter;
            delete m_clipToolAdapter;
        }

        bool MapView3D::cameraFlyModeActive() const {
            return m_flyModeHelper->enabled();
        }
        
        void MapView3D::toggleCameraFlyMode() {
            if (!cameraFlyModeActive()) {
                m_toolBox.disable();
                m_flyModeHelper->enable();
            } else {
                m_flyModeHelper->disable();
                m_toolBox.enable();
            }
            updateAcceleratorTable();
            Refresh();
        }

        void MapView3D::bindObservers() {
            m_camera.cameraDidChangeNotifier.addObserver(this, &MapView3D::cameraDidChange);
        }
        
        void MapView3D::unbindObservers() {
            m_camera.cameraDidChangeNotifier.removeObserver(this, &MapView3D::cameraDidChange);
        }

        void MapView3D::cameraDidChange(const Renderer::Camera* camera) {
            Refresh();
        }
        
        void MapView3D::bindEvents() {
            Bind(wxEVT_KEY_DOWN, &MapView3D::OnKeyDown, this);
            Bind(wxEVT_KEY_UP, &MapView3D::OnKeyUp, this);
            Bind(wxEVT_MOTION, &MapView3D::OnMouseMotion, this);
            
            Bind(wxEVT_SET_FOCUS, &MapView3D::OnSetFocus, this);
            Bind(wxEVT_KILL_FOCUS, &MapView3D::OnKillFocus, this);
            
            Bind(wxEVT_MENU, &MapView3D::OnToggleMovementRestriction,    this, CommandIds::Actions::ToggleMovementRestriction);
            Bind(wxEVT_MENU, &MapView3D::OnSetMovementRestrictionX,      this, CommandIds::Actions::SetMovementRestrictionX);
            Bind(wxEVT_MENU, &MapView3D::OnSetMovementRestrictionY,      this, CommandIds::Actions::SetMovementRestrictionY);
            Bind(wxEVT_MENU, &MapView3D::OnSetMovementRestrictionZ,      this, CommandIds::Actions::SetMovementRestrictionZ);
            Bind(wxEVT_MENU, &MapView3D::OnPerformCreateBrush,           this, CommandIds::Actions::PerformCreateBrush);

            Bind(wxEVT_MENU, &MapView3D::OnMoveTexturesUp,               this, CommandIds::Actions::MoveTexturesUp);
            Bind(wxEVT_MENU, &MapView3D::OnMoveTexturesDown,             this, CommandIds::Actions::MoveTexturesDown);
            Bind(wxEVT_MENU, &MapView3D::OnMoveTexturesLeft,             this, CommandIds::Actions::MoveTexturesLeft);
            Bind(wxEVT_MENU, &MapView3D::OnMoveTexturesRight,            this, CommandIds::Actions::MoveTexturesRight);
            
            Bind(wxEVT_MENU, &MapView3D::OnRotateTexturesCW,             this, CommandIds::Actions::RotateTexturesCW);
            Bind(wxEVT_MENU, &MapView3D::OnRotateTexturesCCW,            this, CommandIds::Actions::RotateTexturesCCW);
            
            Bind(wxEVT_MENU, &MapView3D::OnToggleFlyMode,                this, CommandIds::Actions::ToggleFlyMode);
            
            wxFrame* frame = findFrame(this);
            frame->Bind(wxEVT_ACTIVATE, &MapView3D::OnActivateFrame, this);
        }
        
        void MapView3D::OnKeyDown(wxKeyEvent& event) {
            if (IsBeingDeleted()) return;

            if (!m_flyModeHelper->keyDown(event)) {
                key(event);
                event.Skip();
            }
        }
        
        void MapView3D::OnKeyUp(wxKeyEvent& event) {
            if (IsBeingDeleted()) return;

            if (!m_flyModeHelper->keyUp(event)) {
                key(event);
                event.Skip();
            }
        }
        
        void MapView3D::key(wxKeyEvent& event) {
            updateVerticalMovementRestriction(event);
        }

        void MapView3D::OnMouseMotion(wxMouseEvent& event) {
            if (IsBeingDeleted()) return;

            m_flyModeHelper->motion(event);
            event.Skip();
        }

        void MapView3D::OnToggleMovementRestriction(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            m_movementRestriction.toggleRestriction();
            Refresh();
        }

        void MapView3D::OnSetMovementRestrictionX(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            
            m_movementRestriction.toggleRestriction(Math::Axis::AX);
            Refresh();
        }
        
        void MapView3D::OnSetMovementRestrictionY(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            
            m_movementRestriction.toggleRestriction(Math::Axis::AY);
            Refresh();
        }
        
        void MapView3D::OnSetMovementRestrictionZ(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            
            m_movementRestriction.toggleRestriction(Math::Axis::AZ);
            Refresh();
        }

        void MapView3D::OnPerformCreateBrush(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (m_toolBox.createComplexBrushToolActive())
                m_createComplexBrushToolAdapter->performCreateBrush();
        }

        void MapView3D::OnMoveTexturesUp(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveTextures(Vec2f(0.0f, moveTextureDistance()));
        }
        
        void MapView3D::OnMoveTexturesDown(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveTextures(Vec2f(0.0f, -moveTextureDistance()));
        }
        
        void MapView3D::OnMoveTexturesLeft(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveTextures(Vec2f(-moveTextureDistance(), 0.0f));
        }
        
        void MapView3D::OnMoveTexturesRight(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            moveTextures(Vec2f(moveTextureDistance(), 0.0f));
        }
        
        void MapView3D::OnRotateTexturesCW(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            rotateTextures(rotateTextureAngle(true));
        }
        
        void MapView3D::OnRotateTexturesCCW(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            rotateTextures(rotateTextureAngle(false));
        }
        
        float MapView3D::moveTextureDistance() const {
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
        
        void MapView3D::moveTextures(const Vec2f& offset) {
            MapDocumentSPtr document = lock(m_document);
            if (document->hasSelectedBrushFaces())
                document->moveTextures(m_camera.up(), m_camera.right(), offset);
        }
        
        float MapView3D::rotateTextureAngle(const bool clockwise) const {
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
        
        void MapView3D::rotateTextures(const float angle) {
            MapDocumentSPtr document = lock(m_document);
            if (document->hasSelectedBrushFaces())
                document->rotateTextures(angle);
        }

        void MapView3D::OnToggleFlyMode(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            toggleCameraFlyMode();
        }

        void MapView3D::OnSetFocus(wxFocusEvent& event) {
            if (IsBeingDeleted()) return;
            
            updateVerticalMovementRestriction(wxGetMouseState());
            event.Skip();
        }

        void MapView3D::OnKillFocus(wxFocusEvent& event) {
            if (IsBeingDeleted()) return;

            if (cameraFlyModeActive())
                toggleCameraFlyMode();
            updateVerticalMovementRestriction(wxGetMouseState());
            event.Skip();
        }

        void MapView3D::OnActivateFrame(wxActivateEvent& event) {
            if (IsBeingDeleted()) return;

            if (cameraFlyModeActive())
                toggleCameraFlyMode();
            updateVerticalMovementRestriction(wxGetMouseState());
            event.Skip();
        }

        void MapView3D::updateVerticalMovementRestriction(const wxKeyboardState& state) {
            m_movementRestriction.toggleVerticalRestriction(state.AltDown());
            Refresh();
        }

        PickRequest MapView3D::doGetPickRequest(const int x, const int y) const {
            return PickRequest(Ray3(m_camera.pickRay(x, y)), m_camera);
        }

        Model::PickResult MapView3D::doPick(const Ray3& pickRay) const {
            MapDocumentSPtr document = lock(m_document);
            const Model::EditorContext& editorContext = document->editorContext();
            Model::PickResult pickResult = Model::PickResult::byDistance(editorContext);

            document->pick(pickRay, pickResult);
            return pickResult;
        }

        void MapView3D::doUpdateViewport(const int x, const int y, const int width, const int height) {
            m_camera.setViewport(Renderer::Camera::Viewport(x, y, width, height));
        }

        Vec3 MapView3D::doGetPasteObjectsDelta(const BBox3& bounds, const BBox3& referenceBounds) const {
            MapDocumentSPtr document = lock(m_document);
            const Grid& grid = document->grid();
            
            const wxMouseState mouseState = wxGetMouseState();
            const wxPoint clientCoords = ScreenToClient(mouseState.GetPosition());
            
            if (HitTest(clientCoords) == wxHT_WINDOW_INSIDE) {
                const Ray3f pickRay = m_camera.pickRay(clientCoords.x, clientCoords.y);
                
                const Model::EditorContext& editorContext = document->editorContext();
                Model::PickResult pickResult = Model::PickResult::byDistance(editorContext);

                document->pick(Ray3(pickRay), pickResult);
                const Model::Hit& hit = pickResult.query().pickable().type(Model::Brush::BrushHit).first();
                
                if (hit.isMatch()) {
                    const Model::BrushFace* face = Model::hitToFace(hit);
                    const Plane3 dragPlane = alignedOrthogonalDragPlane(hit.hitPoint(), face->boundary().normal);
                    return grid.moveDeltaForBounds(dragPlane, bounds, document->worldBounds(), pickRay, hit.hitPoint());
                } else {
                    const Vec3 point = m_camera.defaultPoint(pickRay);
                    const Plane3 dragPlane = alignedOrthogonalDragPlane(point, -Vec3(m_camera.direction()));
                    return grid.moveDeltaForBounds(dragPlane, bounds, document->worldBounds(), pickRay, point);
                }
            } else {
                const Vec3 oldMin = bounds.min;
                const Vec3 oldCenter = bounds.center();
                const Vec3 newCenter = m_camera.defaultPoint();
                const Vec3 newMin = oldMin + (newCenter - oldCenter);
                return grid.snap(newMin);
            }
        }
        
        bool MapView3D::doCanSelectTall() {
            return false;
        }
        
        void MapView3D::doSelectTall() {}

        void MapView3D::doFocusCameraOnSelection() {
            MapDocumentSPtr document = lock(m_document);
            const Model::NodeList& nodes = document->selectedNodes().nodes();
            assert(!nodes.empty());
            
            const Vec3 newPosition = focusCameraOnObjectsPosition(nodes);
            moveCameraToPosition(newPosition);
        }
        
        class MapView3D::ComputeCameraCenterPositionVisitor : public Model::ConstNodeVisitor {
        private:
            const Vec3 m_cameraPosition;
            const Vec3 m_cameraDirection;
            FloatType m_minDist;
            Vec3 m_center;
            size_t m_count;
        public:
            ComputeCameraCenterPositionVisitor(const Vec3& cameraPosition, const Vec3& cameraDirection) :
            m_cameraPosition(cameraPosition),
            m_cameraDirection(cameraDirection),
            m_minDist(std::numeric_limits<FloatType>::max()),
            m_count(0) {}
            
            Vec3 position() const {
                return m_center / static_cast<FloatType>(m_count);
            }
        private:
            void doVisit(const Model::World* world)   {}
            void doVisit(const Model::Layer* layer)   {}
            void doVisit(const Model::Group* group)   {}
            
            void doVisit(const Model::Entity* entity) {
                if (!entity->hasChildren()) {
                    const Vec3::List vertices = bBoxVertices(entity->bounds());
                    for (size_t i = 0; i < vertices.size(); ++i)
                        addPoint(vertices[i]);
                }
            }
            
            void doVisit(const Model::Brush* brush)   {
                const Model::Brush::VertexList vertices = brush->vertices();
                Model::Brush::VertexList::const_iterator it, end;
                for (it = vertices.begin(), end = vertices.end(); it != end; ++it)
                    addPoint((*it)->position());
            }
            
            void addPoint(const Vec3& point) {
                const Vec3 toPosition = point - m_cameraPosition;
                m_minDist = std::min(m_minDist, toPosition.dot(m_cameraDirection));
                m_center += point;
                ++m_count;
            }
        };

        class MapView3D::ComputeCameraCenterOffsetVisitor : public Model::ConstNodeVisitor {
        private:
            const Vec3f m_cameraPosition;
            const Vec3f m_cameraDirection;
            Plane3f m_frustumPlanes[4];
            float m_offset;
        public:
            ComputeCameraCenterOffsetVisitor(const Vec3f& cameraPosition, const Vec3f& cameraDirection, const Plane3f frustumPlanes[4]) :
            m_cameraPosition(cameraPosition),
            m_cameraDirection(cameraDirection),
            m_offset(std::numeric_limits<float>::min()) {
                for (size_t i = 0; i < 4; ++i)
                    m_frustumPlanes[i] = frustumPlanes[i];
            }
            
            float offset() const {
                return m_offset;
            }
        private:
            void doVisit(const Model::World* world)   {}
            void doVisit(const Model::Layer* layer)   {}
            void doVisit(const Model::Group* group)   {}
            
            void doVisit(const Model::Entity* entity) {
                if (!entity->hasChildren()) {
                    const Vec3::List vertices = bBoxVertices(entity->bounds());
                    for (size_t i = 0; i < vertices.size(); ++i) {
                        for (size_t j = 0; j < 4; ++j)
                            addPoint(vertices[i], m_frustumPlanes[j]);
                    }
                }
            }
            
            void doVisit(const Model::Brush* brush)   {
                const Model::Brush::VertexList vertices = brush->vertices();
                Model::Brush::VertexList::const_iterator it, end;
                for (it = vertices.begin(), end = vertices.end(); it != end; ++it) {
                    const Model::BrushVertex* vertex = *it;
                    for (size_t j = 0; j < 4; ++j)
                        addPoint(vertex->position(), m_frustumPlanes[j]);
                }
            }
            
            void addPoint(const Vec3f point, const Plane3f& plane) {
                const Ray3f ray(m_cameraPosition, -m_cameraDirection);
                const Plane3f newPlane(point + 64.0f * plane.normal, plane.normal);
                const float dist = newPlane.intersectWithRay(ray);
                if (!Math::isnan(dist) && dist > 0.0f)
                    m_offset = std::max(m_offset, dist);
            }
        };

        Vec3f MapView3D::focusCameraOnObjectsPosition(const Model::NodeList& nodes) {
            ComputeCameraCenterPositionVisitor center(m_camera.position(), m_camera.direction());
            Model::Node::acceptAndRecurse(nodes.begin(), nodes.end(), center);

            const Vec3 newPosition = center.position();
            
            // act as if the camera were there already:
            const Vec3f oldPosition = m_camera.position();
            m_camera.moveTo(Vec3f(newPosition));
            
            Plane3f frustumPlanes[4];
            m_camera.frustumPlanes(frustumPlanes[0], frustumPlanes[1], frustumPlanes[2], frustumPlanes[3]);

            ComputeCameraCenterOffsetVisitor offset(m_camera.position(), m_camera.direction(), frustumPlanes);
            Model::Node::acceptAndRecurse(nodes.begin(), nodes.end(), offset);
            
            // jump back
            m_camera.moveTo(oldPosition);
            return newPosition - m_camera.direction() * offset.offset();
        }
        
        void MapView3D::doMoveCameraToPosition(const Vec3& position) {
            animateCamera(position, m_camera.direction(), m_camera.up());
        }
        
        void MapView3D::animateCamera(const Vec3f& position, const Vec3f& direction, const Vec3f& up, const wxLongLong duration) {
            CameraAnimation* animation = new CameraAnimation(m_camera, position, direction, up, duration);
            m_animationManager->runAnimation(animation, true);
        }
        
        void MapView3D::doMoveCameraToCurrentTracePoint() {
            MapDocumentSPtr document = lock(m_document);
            
            assert(document->isPointFileLoaded());
            Model::PointFile* pointFile = document->pointFile();
            assert(pointFile->hasNextPoint());
            
            const Vec3f position = pointFile->currentPoint() + Vec3f(0.0f, 0.0f, 16.0f);
            const Vec3f direction = pointFile->currentDirection();
            animateCamera(position, direction, Vec3f::PosZ);
        }

        Vec3 MapView3D::doGetMoveDirection(const Math::Direction direction) const {
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
                    return -doGetMoveDirection(Math::Direction_Forward);
                case Math::Direction_Left:
                    return -doGetMoveDirection(Math::Direction_Right);
                case Math::Direction_Right: {
                    Vec3 dir = m_camera.right().firstAxis();
                    if (dir == doGetMoveDirection(Math::Direction_Forward))
                        dir = crossed(dir, Vec3::PosZ);
                    return dir;
                }
                case Math::Direction_Up:
                    return Vec3::PosZ;
                case Math::Direction_Down:
                    return Vec3::NegZ;
                    switchDefault()
            }
        }

        Vec3 MapView3D::doComputePointEntityPosition(const BBox3& bounds) const {
            MapDocumentSPtr document = lock(m_document);
            
            Vec3 delta;
            View::Grid& grid = document->grid();
            
            const BBox3& worldBounds = document->worldBounds();
            
            const Model::Hit& hit = pickResult().query().pickable().type(Model::Brush::BrushHit).occluded().first();
            if (hit.isMatch()) {
                const Model::BrushFace* face = Model::hitToFace(hit);
                return grid.moveDeltaForBounds(face->boundary(), bounds, worldBounds, pickRay(), hit.hitPoint());
            } else {
                const Vec3 newPosition = Renderer::Camera::defaultPoint(pickRay());
                const Vec3 defCenter = bounds.center();
                return grid.moveDeltaForPoint(defCenter, worldBounds, newPosition - defCenter);
            }
        }

        ActionContext MapView3D::doGetActionContext() const {
            if (cameraFlyModeActive())
                return ActionContext_FlyMode;
            return ActionContext_Default;
        }
        
        wxAcceleratorTable MapView3D::doCreateAccelerationTable(ActionContext context) const {
            ActionManager& actionManager = ActionManager::instance();
            return actionManager.createViewAcceleratorTable(context, ActionView_Map3D);
        }
        
        bool MapView3D::doCancel() {
            if (cameraFlyModeActive()) {
                toggleCameraFlyMode();
                return true;
            }
            return false;
        }
        
        Renderer::RenderContext MapView3D::doCreateRenderContext() {
            return Renderer::RenderContext(Renderer::RenderContext::RenderMode_3D, m_camera, fontManager(), shaderManager());
        }

        void MapView3D::doRenderGrid(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {}

        void MapView3D::doRenderMap(Renderer::MapRenderer& renderer, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            renderer.render(renderContext, renderBatch);

            MapDocumentSPtr document = lock(m_document);
            if (renderContext.showSelectionGuide() && document->hasSelectedNodes()) {
                const BBox3& bounds = document->selectionBounds();
                Renderer::SelectionBoundsRenderer boundsRenderer(bounds);
                boundsRenderer.render(renderContext, renderBatch);
                
                Renderer::BoundsGuideRenderer* guideRenderer = new Renderer::BoundsGuideRenderer(m_document);
                guideRenderer->setColor(pref(Preferences::SelectionBoundsColor));
                guideRenderer->setBounds(bounds);
                renderBatch.addOneShot(guideRenderer);
            }
        }
        
        void MapView3D::doRenderTools(MapViewToolBox& toolBox, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            renderTools(renderContext, renderBatch);
        }
        
        void MapView3D::doAfterPopupMenu() {
            updateVerticalMovementRestriction(wxGetMouseState());
            Refresh();
        }
        
        void MapView3D::doLinkCamera(CameraLinkHelper& helper) {}
    }
}
