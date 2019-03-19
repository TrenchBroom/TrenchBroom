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

#include "MapView2D.h"
#include "Logger.h"
#include "Macros.h"
#include "Assets/EntityDefinitionManager.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Model/CollectContainedNodesVisitor.h"
#include "Model/CompareHits.h"
#include "Model/Entity.h"
#include "Model/HitAdapter.h"
#include "Model/HitQuery.h"
#include "Model/PickResult.h"
#include "Model/PointFile.h"
#include "Model/World.h"
#include "Renderer/Compass2D.h"
#include "Renderer/GridRenderer.h"
#include "Renderer/MapRenderer.h"
#include "Renderer/RenderContext.h"
#include "Renderer/SelectionBoundsRenderer.h"
#include "View/Animation.h"
#include "View/CameraAnimation.h"
#include "View/CameraLinkHelper.h"
#include "View/CameraTool2D.h"
#include "View/ClipToolController.h"
#include "View/CreateEntityToolController.h"
#include "View/CreateSimpleBrushToolController2D.h"
#include "View/EdgeTool.h"
#include "View/EdgeToolController.h"
#include "View/FaceTool.h"
#include "View/FaceToolController.h"
#include "View/FlashSelectionAnimation.h"
#include "View/GLContextManager.h"
#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/MapViewToolBox.h"
#include "View/MoveObjectsToolController.h"
#include "View/ResizeBrushesToolController.h"
#include "View/RotateObjectsToolController.h"
#include "View/ScaleObjectsToolController.h"
#include "View/ShearObjectsToolController.h"
#include "View/SelectionTool.h"
#include "View/VertexTool.h"
#include "View/VertexToolController.h"
#include "View/wxUtils.h"

#include <vecmath/util.h>

#include <QWidget>

namespace TrenchBroom {
    namespace View {
        MapView2D::MapView2D(QWidget* parent, Logger* logger, MapDocumentWPtr document, MapViewToolBox& toolBox, Renderer::MapRenderer& renderer, GLContextManager& contextManager, const ViewPlane viewPlane) :
        MapViewBase(parent, logger, document, toolBox, renderer, contextManager),
        m_camera(){
            bindObservers();
            initializeCamera(viewPlane);
            initializeToolChain(toolBox);

            // FIXME: Not sure if QWidget::SetName() maps to QWidget::setWhatsThis()?
            // FIXME: Actually, was SetName used for persistence?
            switch (viewPlane) {
            case ViewPlane_XY:
                widgetContainer()->setWhatsThis("XY View");
                break;
            case ViewPlane_YZ:
                widgetContainer()->setWhatsThis("YZ View");
                break;
            case ViewPlane_XZ:
                widgetContainer()->setWhatsThis("XZ View");
                break;
            switchDefault()
            }
        }

        MapView2D::~MapView2D() {
            unbindObservers();
        }

        void MapView2D::initializeCamera(const ViewPlane viewPlane) {
            switch (viewPlane) {
                case MapView2D::ViewPlane_XY:
                    m_camera.setDirection(vm::vec3f::neg_z, vm::vec3f::pos_y);
                    m_camera.moveTo(vm::vec3f(0.0f, 0.0f, 16384.0f));
                    break;
                case MapView2D::ViewPlane_XZ:
                    m_camera.setDirection(vm::vec3f::pos_y, vm::vec3f::pos_z);
                    m_camera.moveTo(vm::vec3f(0.0f, -16384.0f, 0.0f));
                    break;
                case MapView2D::ViewPlane_YZ:
                    m_camera.setDirection(vm::vec3f::neg_x, vm::vec3f::pos_z);
                    m_camera.moveTo(vm::vec3f(16384.0f, 0.0f, 0.0f));
                    break;
            }
            m_camera.setNearPlane(1.0f);
            m_camera.setFarPlane(32768.0f);
            
        }

        void MapView2D::initializeToolChain(MapViewToolBox& toolBox) {
            addTool(new CameraTool2D(m_camera));
            addTool(new MoveObjectsToolController(toolBox.moveObjectsTool()));
            addTool(new RotateObjectsToolController2D(toolBox.rotateObjectsTool()));
            addTool(new ScaleObjectsToolController2D(toolBox.scaleObjectsTool(), m_document));
            addTool(new ShearObjectsToolController2D(toolBox.shearObjectsTool(), m_document));
            addTool(new ResizeBrushesToolController2D(toolBox.resizeBrushesTool()));
            addTool(new ClipToolController2D(toolBox.clipTool()));
            addTool(new VertexToolController(toolBox.vertexTool()));
            addTool(new EdgeToolController(toolBox.edgeTool()));
            addTool(new FaceToolController(toolBox.faceTool()));
            addTool(new CreateEntityToolController2D(toolBox.createEntityTool()));
            addTool(new SelectionTool(m_document));
            addTool(new CreateSimpleBrushToolController2D(toolBox.createSimpleBrushTool(), m_document));
        }

        void MapView2D::bindObservers() {
            m_camera.cameraDidChangeNotifier.addObserver(this, &MapView2D::cameraDidChange);
        }
        
        void MapView2D::unbindObservers() {
            m_camera.cameraDidChangeNotifier.removeObserver(this, &MapView2D::cameraDidChange);
        }
        
        void MapView2D::cameraDidChange(const Renderer::Camera* camera) {
            Refresh();
        }

        PickRequest MapView2D::doGetPickRequest(const int x, const int y) const {
            return PickRequest(vm::ray3(m_camera.pickRay(x, y)), m_camera);
        }
        
        Model::PickResult MapView2D::doPick(const vm::ray3& pickRay) const {
            auto document = lock(m_document);
            const auto& editorContext = document->editorContext();
            const auto axis = firstComponent(pickRay.direction);
            
            auto pickResult = Model::PickResult::bySize(editorContext, axis);
            document->pick(pickRay, pickResult);
            
            return pickResult;
        }

        void MapView2D::initializeGL() {
            MapViewBase::initializeGL();
            setCompass(new Renderer::Compass2D());
        }

        void MapView2D::doUpdateViewport(const int x, const int y, const int width, const int height) {
            m_camera.setViewport(Renderer::Camera::Viewport(x, y, width, height));
        }

        vm::vec3 MapView2D::doGetPasteObjectsDelta(const vm::bbox3& bounds, const vm::bbox3& referenceBounds) const {
            auto document = lock(m_document);
            const auto& grid = document->grid();
            const auto& worldBounds = document->worldBounds();

            const auto& pickRay = MapView2D::pickRay();
            
            const auto toMin = referenceBounds.min - pickRay.origin;
            const auto toMax = referenceBounds.max - pickRay.origin;
            const auto anchor = dot(toMin, pickRay.direction) > dot(toMax, pickRay.direction) ? referenceBounds.min : referenceBounds.max;
            const auto dragPlane = vm::plane3(anchor, -pickRay.direction);
            
            const auto distance = intersect(pickRay, dragPlane);;
            if (vm::isnan(distance)) {
                return vm::vec3::zero;
            } else {
                const auto hitPoint = pickRay.pointAtDistance(distance);
                return grid.moveDeltaForBounds(dragPlane, bounds, worldBounds, pickRay, hitPoint);
            }
        }
        
        bool MapView2D::doCanSelectTall() {
            return true;
        }
        
        void MapView2D::doSelectTall() {
            const MapDocumentSPtr document = lock(m_document);
            const vm::bbox3& worldBounds = document->worldBounds();
            
            const FloatType min = dot(worldBounds.min, vm::vec3(m_camera.direction()));
            const FloatType max = dot(worldBounds.max, vm::vec3(m_camera.direction()));
            
            const vm::plane3 minPlane(min, vm::vec3(m_camera.direction()));
            const vm::plane3 maxPlane(max, vm::vec3(m_camera.direction()));
            
            const Model::BrushList& selectionBrushes = document->selectedNodes().brushes();
            assert(!selectionBrushes.empty());
            
            const Model::BrushBuilder brushBuilder(document->world(), worldBounds);
            Model::BrushList tallBrushes(0);
            tallBrushes.reserve(selectionBrushes.size());
            
            for (const Model::Brush* selectionBrush : selectionBrushes) {
                std::vector<vm::vec3> tallVertices(0);
                tallVertices.reserve(2 * selectionBrush->vertexCount());
                
                for (const Model::BrushVertex* vertex : selectionBrush->vertices()) {
                    tallVertices.push_back(minPlane.projectPoint(vertex->position()));
                    tallVertices.push_back(maxPlane.projectPoint(vertex->position()));
                }

                Model::Brush* tallBrush = brushBuilder.createBrush(tallVertices, Model::BrushFace::NoTextureName);
                tallBrushes.push_back(tallBrush);
            }

            Transaction transaction(document, "Select Tall");
            document->deleteObjects();

            Model::CollectContainedNodesVisitor<Model::BrushList::const_iterator> visitor(std::begin(tallBrushes), std::end(tallBrushes), document->editorContext());
            document->world()->acceptAndRecurse(visitor);
            document->select(visitor.nodes());

            VectorUtils::clearAndDelete(tallBrushes);
        }

        void MapView2D::doFocusCameraOnSelection(const bool animate) {
            const auto document = lock(m_document);
            const auto& bounds = document->referenceBounds();
            const auto diff = bounds.center() - vm::vec3(m_camera.position());
            const auto delta = diff * vm::vec3(m_camera.up() + m_camera.right());
            moveCameraToPosition(vm::vec3(m_camera.position()) + delta, animate);
        }
        
        void MapView2D::doMoveCameraToPosition(const vm::vec3& position, const bool animate) {
            if (animate) {
                animateCamera(vm::vec3f(position), m_camera.direction(), m_camera.up());
            } else {
                m_camera.moveTo(vm::vec3f(position));
            }
        }
        
        void MapView2D::animateCamera(const vm::vec3f& position, const vm::vec3f& direction, const vm::vec3f& up, const int duration) {
            const auto actualPosition = dot(position, m_camera.up()) * m_camera.up() + dot(position, m_camera.right()) * m_camera.right() + dot(m_camera.position(), m_camera.direction()) * m_camera.direction();
            auto* animation = new CameraAnimation(m_camera, actualPosition, m_camera.direction(), m_camera.up(), duration);
            m_animationManager->runAnimation(animation, true);
        }
        
        void MapView2D::doMoveCameraToCurrentTracePoint() {
            auto document = lock(m_document);
            
            assert(document->isPointFileLoaded());
            auto* pointFile = document->pointFile();
            assert(pointFile->hasNextPoint());
            
            const auto position = vm::vec3(pointFile->currentPoint());
            moveCameraToPosition(position, true);
        }

        vm::vec3 MapView2D::doGetMoveDirection(const vm::direction direction) const {
            switch (direction) {
                case vm::direction::forward:
                    return vm::vec3(firstAxis(m_camera.direction()));
                case vm::direction::backward:
                    return vm::vec3(-firstAxis(m_camera.direction()));
                case vm::direction::left:
                    return vm::vec3(-firstAxis(m_camera.right()));
                case vm::direction::right:
                    return vm::vec3(firstAxis(m_camera.right()));
                case vm::direction::up:
                    return vm::vec3(firstAxis(m_camera.up()));
                case vm::direction::down:
                    return vm::vec3(-firstAxis(m_camera.up()));
                switchDefault()
            }
        }

        vm::vec3 MapView2D::doComputePointEntityPosition(const vm::bbox3& bounds) const {
            auto document = lock(m_document);

            vm::vec3 delta;
            const auto& grid = document->grid();
            
            const auto& worldBounds = document->worldBounds();
            
            const auto& hit = pickResult().query().pickable().type(Model::Brush::BrushHit).occluded().selected().first();
            if (hit.isMatch()) {
                const auto* face = Model::hitToFace(hit);
                return grid.moveDeltaForBounds(face->boundary(), bounds, worldBounds, pickRay(), hit.hitPoint());
            } else {
                const auto referenceBounds = document->referenceBounds();
                const auto& pickRay = MapView2D::pickRay();
                
                const auto toMin = referenceBounds.min - pickRay.origin;
                const auto toMax = referenceBounds.max - pickRay.origin;
                const auto anchor = dot(toMin, pickRay.direction) > dot(toMax, pickRay.direction) ? referenceBounds.min : referenceBounds.max;
                const auto dragPlane = vm::plane3(anchor, -pickRay.direction);
                
                const auto distance = intersect(pickRay, dragPlane);
                if (vm::isnan(distance)) {
                    return vm::vec3::zero;
                } else {
                    const auto hitPoint = pickRay.pointAtDistance(distance);
                    return grid.moveDeltaForBounds(dragPlane, bounds, worldBounds, pickRay, hitPoint);
                }
            }
        }

        ActionContext MapView2D::doGetActionContext() const {
            return ActionContext_Default;
        }

        ActionView MapView2D::doGetActionView() const {
            return ActionView_Map2D;
        }

// FIXME: Port to Qt
#if 0
        wxAcceleratorTable MapView2D::doCreateAccelerationTable(ActionContext context) const {
            auto document = lock(m_document);
            const auto& tags = document->smartTags();
            const auto& entityDefinitions = document->entityDefinitionManager().definitions();
            auto& actionManager = ActionManager::instance();
            return actionManager.createViewAcceleratorTable(context, ActionView_Map2D, tags, entityDefinitions);
        }
#endif

        bool MapView2D::doCancel() {
            return false;
        }
        
        Renderer::RenderContext::RenderMode MapView2D::doGetRenderMode() {
            return Renderer::RenderContext::RenderMode_2D;
        }
        
        Renderer::Camera& MapView2D::doGetCamera() {
            return m_camera;
        }
        
        void MapView2D::doRenderGrid(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            MapDocumentSPtr document = lock(m_document);
            renderBatch.addOneShot(new Renderer::GridRenderer(m_camera, document->worldBounds()));
        }

        void MapView2D::doRenderMap(Renderer::MapRenderer& renderer, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            renderer.render(renderContext, renderBatch);

            MapDocumentSPtr document = lock(m_document);
            if (renderContext.showSelectionGuide() && document->hasSelectedNodes()) {
                const vm::bbox3& bounds = document->selectionBounds();
                Renderer::SelectionBoundsRenderer boundsRenderer(bounds);
                boundsRenderer.render(renderContext, renderBatch);
            }
        }
        
        void MapView2D::doRenderTools(MapViewToolBox& toolBox, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            renderTools(renderContext, renderBatch);
        }
        
        void MapView2D::doRenderExtras(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {}
        
        void MapView2D::doLinkCamera(CameraLinkHelper& helper) {
            helper.addCamera(&m_camera);
        }
    }
}
