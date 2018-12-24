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

#include "MapView3D.h"
#include "Logger.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "TemporarilySetAny.h"
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
#include "View/ClipToolController.h"
#include "View/CommandIds.h"
#include "View/CreateComplexBrushToolController3D.h"
#include "View/CreateEntityToolController.h"
#include "View/CreateSimpleBrushToolController3D.h"
#include "View/EdgeTool.h"
#include "View/EdgeToolController.h"
#include "View/FaceTool.h"
#include "View/FaceToolController.h"
#include "View/FlashSelectionAnimation.h"
#include "View/FlyModeHelper.h"
#include "View/GLContextManager.h"
#include "View/Grid.h"
#include "View/InputState.h"
#include "View/MapDocument.h"
#include "View/MapViewToolBox.h"
#include "View/MoveObjectsToolController.h"
#include "View/ResizeBrushesToolController.h"
#include "View/RotateObjectsToolController.h"
#include "View/ScaleObjectsToolController.h"
#include "View/ShearObjectsToolController.h"
#include "View/SelectionTool.h"
#include "View/SetBrushFaceAttributesTool.h"
#include "View/VertexTool.h"
#include "View/VertexToolController.h"
#include "View/wxUtils.h"

#include <vecmath/util.h>

#include <wx/frame.h>

namespace TrenchBroom {
    namespace View {
        MapView3D::MapView3D(QWidget* parent, Logger* logger, MapDocumentWPtr document, MapViewToolBox& toolBox, Renderer::MapRenderer& renderer, GLContextManager& contextManager) :
        MapViewBase(parent, logger, document, toolBox, renderer, contextManager),
        m_flyModeHelper(new FlyModeHelper(m_camera)),
        m_ignoreCameraChangeEvents(false) {
            bindEvents();
            bindObservers();
            initializeCamera();
            initializeToolChain(toolBox);
            setCompass(new Renderer::Compass3D());

            // FIXME: Replace
            //SetName("MapView3D");

            m_camera.setFov(pref(Preferences::CameraFov));
        }

        MapView3D::~MapView3D() {
            unbindObservers();
        }
        
        void MapView3D::initializeCamera() {
            m_camera.moveTo(vm::vec3f(-80.0f, -128.0f, 96.0f));
            m_camera.lookAt(vm::vec3f::zero, vm::vec3f::pos_z);
        }

        void MapView3D::initializeToolChain(MapViewToolBox& toolBox) {
            addTool(new CameraTool3D(m_document, m_camera));
            addTool(new MoveObjectsToolController(toolBox.moveObjectsTool()));
            addTool(new RotateObjectsToolController3D(toolBox.rotateObjectsTool()));
            addTool(new ScaleObjectsToolController3D(toolBox.scaleObjectsTool(), m_document));
            addTool(new ShearObjectsToolController3D(toolBox.shearObjectsTool(), m_document));
            addTool(new ResizeBrushesToolController3D(toolBox.resizeBrushesTool()));
            addTool(new CreateComplexBrushToolController3D(toolBox.createComplexBrushTool()));
            addTool(new ClipToolController3D(toolBox.clipTool()));
            addTool(new VertexToolController(toolBox.vertexTool()));
            addTool(new EdgeToolController(toolBox.edgeTool()));
            addTool(new FaceToolController(toolBox.faceTool()));
            addTool(new CreateEntityToolController3D(toolBox.createEntityTool()));
            addTool(new SetBrushFaceAttributesTool(m_document));
            addTool(new SelectionTool(m_document));
            addTool(new CreateSimpleBrushToolController3D(toolBox.createSimpleBrushTool(), m_document));
        }

        void MapView3D::bindObservers() {
            m_camera.cameraDidChangeNotifier.addObserver(this, &MapView3D::cameraDidChange);

            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &MapView3D::preferenceDidChange);
        }
        
        void MapView3D::unbindObservers() {
            m_camera.cameraDidChangeNotifier.removeObserver(this, &MapView3D::cameraDidChange);

            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &MapView3D::preferenceDidChange);
        }

        void MapView3D::cameraDidChange(const Renderer::Camera* camera) {
            if (!m_ignoreCameraChangeEvents) {
                // Don't refresh if the camera was changed in doPreRender!
                update();
            }
        }

        void MapView3D::preferenceDidChange(const IO::Path& path) {
            if (path == Preferences::CameraFov.path()) {
                m_camera.setFov(pref(Preferences::CameraFov));
                update();
            }
        }

        void MapView3D::bindEvents() {
            // FIXME: implement these
#if 0
            Bind(wxEVT_MENU, &MapView3D::OnPerformCreateBrush,           this, CommandIds::Actions::PerformCreateBrush);

            Bind(wxEVT_MENU, &MapView3D::OnMoveTexturesUp,               this, CommandIds::Actions::MoveTexturesUp);
            Bind(wxEVT_MENU, &MapView3D::OnMoveTexturesDown,             this, CommandIds::Actions::MoveTexturesDown);
            Bind(wxEVT_MENU, &MapView3D::OnMoveTexturesLeft,             this, CommandIds::Actions::MoveTexturesLeft);
            Bind(wxEVT_MENU, &MapView3D::OnMoveTexturesRight,            this, CommandIds::Actions::MoveTexturesRight);
            
            Bind(wxEVT_MENU, &MapView3D::OnRotateTexturesCW,             this, CommandIds::Actions::RotateTexturesCW);
            Bind(wxEVT_MENU, &MapView3D::OnRotateTexturesCCW,            this, CommandIds::Actions::RotateTexturesCCW);
            
            Bind(wxEVT_MENU, &MapView3D::OnResetZoom,                    this, CommandIds::Actions::ResetZoom);

            wxFrame* frame = findFrame(this);
            frame->Bind(wxEVT_ACTIVATE, &MapView3D::OnActivateFrame, this);

            Bind(wxEVT_IDLE, &MapView3D::OnIdle, this);
#endif
        }

        void MapView3D::OnIdle(wxIdleEvent& event) {
            if (m_flyModeHelper->anyKeyDown()) {
                update();
                event.RequestMore();
            }
        }

        void MapView3D::keyPressEvent(QKeyEvent* event) {
            if (!m_flyModeHelper->keyDown(event)) {
                // Not handled, use the default Qt event handling
                MapViewBase::keyPressEvent(event);
            }
        }
        
        void MapView3D::keyReleaseEvent(QKeyEvent* event) {
            if (!m_flyModeHelper->keyUp(event)) {
                // Not handled, use the default Qt event handling
                MapViewBase::keyReleaseEvent(event);
            }
        }
        
        void MapView3D::OnPerformCreateBrush(wxCommandEvent& event) {
            if (m_toolBox.createComplexBrushToolActive())
                m_toolBox.performCreateComplexBrush();
        }

        void MapView3D::OnMoveTexturesUp(wxCommandEvent& event) {
            moveTextures(vm::vec2f(0.0f, moveTextureDistance()));
        }
        
        void MapView3D::OnMoveTexturesDown(wxCommandEvent& event) {
            moveTextures(vm::vec2f(0.0f, -moveTextureDistance()));
        }
        
        void MapView3D::OnMoveTexturesLeft(wxCommandEvent& event) {
            moveTextures(vm::vec2f(-moveTextureDistance(), 0.0f));
        }
        
        void MapView3D::OnMoveTexturesRight(wxCommandEvent& event) {
            moveTextures(vm::vec2f(moveTextureDistance(), 0.0f));
        }
        
        void MapView3D::OnRotateTexturesCW(wxCommandEvent& event) {
            rotateTextures(rotateTextureAngle(true));
        }
        
        void MapView3D::OnRotateTexturesCCW(wxCommandEvent& event) {
            rotateTextures(rotateTextureAngle(false));
        }

        void MapView3D::OnResetZoom(wxCommandEvent& event) {
            m_camera.setZoom(1.0f);
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
        
        void MapView3D::moveTextures(const vm::vec2f& offset) {
            MapDocumentSPtr document = lock(m_document);
            if (document->hasSelectedBrushFaces())
                document->moveTextures(m_camera.up(), m_camera.right(), offset);
        }
        
        float MapView3D::rotateTextureAngle(const bool clockwise) const {
            const Grid& grid = lock(m_document)->grid();
            const float gridAngle = static_cast<float>(vm::toDegrees(grid.angle()));
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

        void MapView3D::focusOutEvent(QFocusEvent* event) {
            m_flyModeHelper->resetKeys();

            MapViewBase::focusOutEvent(event);
        }

        void MapView3D::OnActivateFrame(wxActivateEvent& event) {
            m_flyModeHelper->resetKeys();
            event.Skip();
        }

        PickRequest MapView3D::doGetPickRequest(const int x, const int y) const {
            return PickRequest(vm::ray3(m_camera.pickRay(x, y)), m_camera);
        }

        Model::PickResult MapView3D::doPick(const vm::ray3& pickRay) const {
            MapDocumentSPtr document = lock(m_document);
            const Model::EditorContext& editorContext = document->editorContext();
            Model::PickResult pickResult = Model::PickResult::byDistance(editorContext);

            document->pick(pickRay, pickResult);
            return pickResult;
        }

        void MapView3D::doUpdateViewport(const int x, const int y, const int width, const int height) {
            m_camera.setViewport(Renderer::Camera::Viewport(x, y, width, height));
        }

        vm::vec3 MapView3D::doGetPasteObjectsDelta(const vm::bbox3& bounds, const vm::bbox3& referenceBounds) const {
            auto document = lock(m_document);
            const auto& grid = document->grid();
            
            const auto mouseState = wxGetMouseState();
            const auto clientCoords = ScreenToClient(mouseState.GetPosition());
            
            if (HitTest(clientCoords) == wxHT_WINDOW_INSIDE) {
                const auto pickRay = vm::ray3(m_camera.pickRay(clientCoords.x, clientCoords.y));
                
                const auto& editorContext = document->editorContext();
                auto pickResult = Model::PickResult::byDistance(editorContext);

                document->pick(pickRay, pickResult);
                const auto& hit = pickResult.query().pickable().type(Model::Brush::BrushHit).first();
                
                if (hit.isMatch()) {
                    const auto* face = Model::hitToFace(hit);
                    const auto dragPlane = alignedOrthogonalPlane(hit.hitPoint(), face->boundary().normal);
                    return grid.moveDeltaForBounds(dragPlane, bounds, document->worldBounds(), pickRay, hit.hitPoint());
                } else {
                    const auto point = vm::vec3(grid.snap(m_camera.defaultPoint(pickRay)));
                    const auto dragPlane = alignedOrthogonalPlane(point, -vm::vec3(firstAxis(m_camera.direction())));
                    return grid.moveDeltaForBounds(dragPlane, bounds, document->worldBounds(), pickRay, point);
                }
            } else {
                const auto oldMin = bounds.min;
                const auto oldCenter = bounds.center();
                const auto newCenter = vm::vec3(m_camera.defaultPoint());
                const auto newMin = oldMin + (newCenter - oldCenter);
                return grid.snap(newMin);
            }
        }
        
        bool MapView3D::doCanSelectTall() {
            return false;
        }
        
        void MapView3D::doSelectTall() {}

        void MapView3D::doFocusCameraOnSelection(const bool animate) {
            auto document = lock(m_document);
            const auto& nodes = document->selectedNodes().nodes();
            if (!nodes.empty()) {
                const auto newPosition = focusCameraOnObjectsPosition(nodes);
                moveCameraToPosition(newPosition, animate);
            }
        }
        
        class MapView3D::ComputeCameraCenterPositionVisitor : public Model::ConstNodeVisitor {
        private:
            const vm::vec3 m_cameraPosition;
            const vm::vec3 m_cameraDirection;
            FloatType m_minDist;
            vm::vec3 m_center;
            size_t m_count;
        public:
            ComputeCameraCenterPositionVisitor(const vm::vec3& cameraPosition, const vm::vec3& cameraDirection) :
            m_cameraPosition(cameraPosition),
            m_cameraDirection(cameraDirection),
            m_minDist(std::numeric_limits<FloatType>::max()),
            m_count(0) {}
            
            vm::vec3 position() const {
                return m_center / static_cast<FloatType>(m_count);
            }
        private:
            void doVisit(const Model::World* world) override   {}
            void doVisit(const Model::Layer* layer) override   {}
            void doVisit(const Model::Group* group) override   {}
            
            void doVisit(const Model::Entity* entity) override {
                if (!entity->hasChildren()) {
                    const auto& bounds = entity->bounds();
                    bounds.forEachVertex([&](const vm::vec3& v) { addPoint(v); });
                }
            }
            
            void doVisit(const Model::Brush* brush) override   {
                for (const Model::BrushVertex* vertex : brush->vertices()) {
                    addPoint(vertex->position());
                }
            }
            
            void addPoint(const vm::vec3& point) {
                const vm::vec3 toPosition = point - m_cameraPosition;
                m_minDist = vm::min(m_minDist, dot(toPosition, m_cameraDirection));
                m_center = m_center + point;
                ++m_count;
            }
        };

        class MapView3D::ComputeCameraCenterOffsetVisitor : public Model::ConstNodeVisitor {
        private:
            const vm::vec3f m_cameraPosition;
            const vm::vec3f m_cameraDirection;
            vm::plane3f m_frustumPlanes[4];
            float m_offset;
        public:
            ComputeCameraCenterOffsetVisitor(const vm::vec3f& cameraPosition, const vm::vec3f& cameraDirection, const vm::plane3f frustumPlanes[4]) :
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
            void doVisit(const Model::World* world) override   {}
            void doVisit(const Model::Layer* layer) override   {}
            void doVisit(const Model::Group* group) override   {}
            
            void doVisit(const Model::Entity* entity) override {
                if (!entity->hasChildren()) {
                    const auto& bounds = entity->bounds();
                    bounds.forEachVertex([&](const vm::vec3& v) {
                        for (size_t j = 0; j < 4; ++j) {
                            addPoint(vm::vec3f(v), m_frustumPlanes[j]);
                        }
                    });
                }
            }
            
            void doVisit(const Model::Brush* brush) override   {
                for (const auto* vertex : brush->vertices()) {
                    for (size_t j = 0; j < 4; ++j) {
                        addPoint(vm::vec3f(vertex->position()), m_frustumPlanes[j]);
                    }
                }
            }
            
            void addPoint(const vm::vec3f point, const vm::plane3f& plane) {
                const auto ray = vm::ray3f(m_cameraPosition, -m_cameraDirection);
                const auto newPlane = vm::plane3f(point + 64.0f * plane.normal, plane.normal);
                const auto dist = intersect(ray, newPlane);;
                if (!vm::isnan(dist) && dist > 0.0f) {
                    m_offset = std::max(m_offset, dist);
                }
            }
        };

        vm::vec3 MapView3D::focusCameraOnObjectsPosition(const Model::NodeList& nodes) {
            ComputeCameraCenterPositionVisitor center(vm::vec3(m_camera.position()), vm::vec3(m_camera.direction()));
            Model::Node::acceptAndRecurse(std::begin(nodes), std::end(nodes), center);

            const auto newPosition = center.position();
            
            // act as if the camera were there already:
            const auto oldPosition = m_camera.position();
            m_camera.moveTo(vm::vec3f(newPosition));
            
            vm::plane3f frustumPlanes[4];
            m_camera.frustumPlanes(frustumPlanes[0], frustumPlanes[1], frustumPlanes[2], frustumPlanes[3]);

            ComputeCameraCenterOffsetVisitor offset(m_camera.position(), m_camera.direction(), frustumPlanes);
            Model::Node::acceptAndRecurse(std::begin(nodes), std::end(nodes), offset);
            
            // jump back
            m_camera.moveTo(oldPosition);
            return newPosition - vm::vec3(m_camera.direction() * offset.offset());
        }
        
        void MapView3D::doMoveCameraToPosition(const vm::vec3& position, const bool animate) {
            if (animate) {
                animateCamera(vm::vec3f(position), m_camera.direction(), m_camera.up());
            } else {
                m_camera.moveTo(vm::vec3f(position));
            }
        }
        
        void MapView3D::animateCamera(const vm::vec3f& position, const vm::vec3f& direction, const vm::vec3f& up, const wxLongLong duration) {
            CameraAnimation* animation = new CameraAnimation(m_camera, position, direction, up, duration);
            m_animationManager->runAnimation(animation, true);
        }
        
        void MapView3D::doMoveCameraToCurrentTracePoint() {
            MapDocumentSPtr document = lock(m_document);
            
            assert(document->isPointFileLoaded());
            Model::PointFile* pointFile = document->pointFile();
            assert(pointFile->hasNextPoint());
            
            const vm::vec3f position = pointFile->currentPoint() + vm::vec3f(0.0f, 0.0f, 16.0f);
            const vm::vec3f direction = pointFile->currentDirection();
            animateCamera(position, direction, vm::vec3f::pos_z);
        }

        vm::vec3 MapView3D::doGetMoveDirection(const vm::direction direction) const {
            switch (direction) {
                case vm::direction::forward: {
                    const auto plane = vm::plane3(vm::vec3(m_camera.position()), vm::vec3::pos_z);
                    const auto projectedDirection = plane.projectVector(vm::vec3(m_camera.direction()));
                    if (isZero(projectedDirection, vm::C::almostZero())) {
                        // camera is looking straight down or up
                        if (m_camera.direction().z() < 0.0) {
                            return vm::vec3(firstAxis(m_camera.up()));
                        } else {
                            return vm::vec3(-firstAxis(m_camera.up()));
                        }
                    }
                    return firstAxis(projectedDirection);
                }
                case vm::direction::backward:
                    return -doGetMoveDirection(vm::direction::forward);
                case vm::direction::left:
                    return -doGetMoveDirection(vm::direction::right);
                case vm::direction::right: {
                    auto dir = vm::vec3(firstAxis(m_camera.right()));
                    if (dir == doGetMoveDirection(vm::direction::forward)) {
                        dir = cross(dir, vm::vec3::pos_z);
                    }
                    return dir;
                }
                case vm::direction::up:
                    return vm::vec3::pos_z;
                case vm::direction::down:
                    return vm::vec3::neg_z;
                switchDefault()
            }
        }

        vm::vec3 MapView3D::doComputePointEntityPosition(const vm::bbox3& bounds) const {
            auto document = lock(m_document);
            
            vm::vec3 delta;
            auto& grid = document->grid();
            
            const auto& worldBounds = document->worldBounds();
            
            const auto& hit = pickResult().query().pickable().type(Model::Brush::BrushHit).occluded().first();
            if (hit.isMatch()) {
                const auto* face = Model::hitToFace(hit);
                return grid.moveDeltaForBounds(face->boundary(), bounds, worldBounds, pickRay(), hit.hitPoint());
            } else {
                const auto newPosition = Renderer::Camera::defaultPoint(pickRay());
                const auto defCenter = bounds.center();
                return grid.moveDeltaForPoint(defCenter, worldBounds, newPosition - defCenter);
            }
        }

        ActionContext MapView3D::doGetActionContext() const {
            return ActionContext_Default;
        }
        
        wxAcceleratorTable MapView3D::doCreateAccelerationTable(ActionContext context) const {
            ActionManager& actionManager = ActionManager::instance();
            return actionManager.createViewAcceleratorTable(context, ActionView_Map3D);
        }
        
        bool MapView3D::doCancel() {
            return false;
        }
        
        Renderer::RenderContext::RenderMode MapView3D::doGetRenderMode() {
            return Renderer::RenderContext::RenderMode_3D;
        }
        
        Renderer::Camera& MapView3D::doGetCamera() {
            return m_camera;
        }

        void MapView3D::doPreRender() {
            const TemporarilySetBool ignoreCameraUpdates(m_ignoreCameraChangeEvents);
            m_flyModeHelper->pollAndUpdate();
        }

        void MapView3D::doRenderGrid(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {}

        void MapView3D::doRenderMap(Renderer::MapRenderer& renderer, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            renderer.render(renderContext, renderBatch);
            
            MapDocumentSPtr document = lock(m_document);
            if (renderContext.showSelectionGuide() && document->hasSelectedNodes()) {
                const vm::bbox3& bounds = document->selectionBounds();
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
        
        bool MapView3D::doBeforePopupMenu() {
            m_flyModeHelper->resetKeys();
            return true;
        }

        void MapView3D::doLinkCamera(CameraLinkHelper& helper) {}
    }
}
