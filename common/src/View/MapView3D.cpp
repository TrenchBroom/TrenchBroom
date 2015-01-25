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
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushVertex.h"
#include "Model/Entity.h"
#include "Model/HitAdapter.h"
#include "Model/HitQuery.h"
#include "Model/PickResult.h"
#include "Model/PointFile.h"
#include "Renderer/Compass.h"
#include "Renderer/MapRenderer.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "View/ActionManager.h"
#include "View/Animation.h"
#include "View/CameraAnimation.h"
#include "View/CameraTool3D.h"
#include "View/ClipToolAdapter.h"
#include "View/CommandIds.h"
#include "View/CreateEntityToolAdapter.h"
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
        m_compass(new Renderer::Compass(m_movementRestriction)),
        m_clipToolAdapter(NULL),
        m_createEntityToolAdapter(NULL),
        m_moveObjectsToolAdapter(NULL),
        m_resizeBrushesToolAdapter(NULL),
        m_rotateObjectsToolAdapter(NULL),
        m_setBrushFaceAttributesTool(NULL),
        m_vertexToolAdapter(NULL),
        m_cameraTool(NULL),
        m_flyModeHelper(new FlyModeHelper(this, m_camera)) {
            bindEvents();
            bindObservers();
            initializeToolChain(toolBox);
        }

        MapView3D::~MapView3D() {
            unbindObservers();
            
            delete m_flyModeHelper;
            delete m_cameraTool;
            delete m_vertexToolAdapter;
            delete m_setBrushFaceAttributesTool;
            delete m_rotateObjectsToolAdapter;
            delete m_resizeBrushesToolAdapter;
            delete m_moveObjectsToolAdapter;
            delete m_createEntityToolAdapter;
            delete m_clipToolAdapter;
            delete m_compass;
        }
        
        void MapView3D::initializeToolChain(MapViewToolBox& toolBox) {
            const Grid& grid = lock(m_document)->grid();
            m_clipToolAdapter = new ClipToolAdapter3D(toolBox.clipTool(), grid);
            m_createEntityToolAdapter = new CreateEntityToolAdapter3D(toolBox.createEntityTool());
            m_moveObjectsToolAdapter = new MoveObjectsToolAdapter3D(toolBox.moveObjectsTool(), m_movementRestriction);
            m_resizeBrushesToolAdapter = new ResizeBrushesToolAdapter3D(toolBox.resizeBrushesTool());
            m_rotateObjectsToolAdapter = new RotateObjectsToolAdapter3D(toolBox.rotateObjectsTool(), m_movementRestriction);
            m_setBrushFaceAttributesTool = new SetBrushFaceAttributesTool(m_document);
            m_vertexToolAdapter = new VertexToolAdapter3D(toolBox.vertexTool(), m_movementRestriction);
            m_cameraTool = new CameraTool3D(m_document, m_camera);
            
            addTool(m_cameraTool);
            addTool(m_clipToolAdapter);
            addTool(m_rotateObjectsToolAdapter);
            addTool(m_vertexToolAdapter);
            addTool(m_moveObjectsToolAdapter);
            addTool(m_resizeBrushesToolAdapter);
            addTool(m_createEntityToolAdapter);
            addTool(m_setBrushFaceAttributesTool);
            addTool(toolBox.selectionTool());
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
            Bind(wxEVT_KEY_DOWN, &MapView3D::OnKey, this);
            Bind(wxEVT_KEY_UP, &MapView3D::OnKey, this);
            
            Bind(wxEVT_KILL_FOCUS, &MapView3D::OnKillFocus, this);
            
            Bind(wxEVT_MENU, &MapView3D::OnToggleMovementRestriction,    this, CommandIds::Actions::ToggleMovementRestriction);
            
            Bind(wxEVT_MENU, &MapView3D::OnMoveTexturesUp,               this, CommandIds::Actions::MoveTexturesUp);
            Bind(wxEVT_MENU, &MapView3D::OnMoveTexturesDown,             this, CommandIds::Actions::MoveTexturesDown);
            Bind(wxEVT_MENU, &MapView3D::OnMoveTexturesLeft,             this, CommandIds::Actions::MoveTexturesLeft);
            Bind(wxEVT_MENU, &MapView3D::OnMoveTexturesRight,            this, CommandIds::Actions::MoveTexturesRight);
            
            Bind(wxEVT_MENU, &MapView3D::OnRotateTexturesCW,             this, CommandIds::Actions::RotateTexturesCW);
            Bind(wxEVT_MENU, &MapView3D::OnRotateTexturesCCW,            this, CommandIds::Actions::RotateTexturesCCW);
            
            Bind(wxEVT_MENU, &MapView3D::OnToggleFlyMode,                this, CommandIds::Actions::ToggleFlyMode);
            
            /*
            Bind(wxEVT_MENU, &MapView3D::OnPopupReparentBrushes,         this, CommandIds::CreateEntityPopupMenu::ReparentBrushes);
            Bind(wxEVT_MENU, &MapView3D::OnPopupMoveBrushesToWorld,      this, CommandIds::CreateEntityPopupMenu::MoveBrushesToWorld);
            Bind(wxEVT_MENU, &MapView3D::OnPopupCreatePointEntity,       this, CommandIds::CreateEntityPopupMenu::LowestPointEntityItem, CommandIds::CreateEntityPopupMenu::HighestPointEntityItem);
            Bind(wxEVT_MENU, &MapView3D::OnPopupCreateBrushEntity,       this, CommandIds::CreateEntityPopupMenu::LowestBrushEntityItem, CommandIds::CreateEntityPopupMenu::HighestBrushEntityItem);
            
            Bind(wxEVT_UPDATE_UI, &MapView3D::OnUpdatePopupMenuItem, this, CommandIds::CreateEntityPopupMenu::ReparentBrushes);
            Bind(wxEVT_UPDATE_UI, &MapView3D::OnUpdatePopupMenuItem, this, CommandIds::CreateEntityPopupMenu::MoveBrushesToWorld);
            Bind(wxEVT_UPDATE_UI, &MapView3D::OnUpdatePopupMenuItem, this, CommandIds::CreateEntityPopupMenu::LowestPointEntityItem, CommandIds::CreateEntityPopupMenu::HighestPointEntityItem);
            Bind(wxEVT_UPDATE_UI, &MapView3D::OnUpdatePopupMenuItem, this, CommandIds::CreateEntityPopupMenu::LowestBrushEntityItem, CommandIds::CreateEntityPopupMenu::HighestBrushEntityItem);
            */
            
            wxFrame* frame = findFrame(this);
            frame->Bind(wxEVT_ACTIVATE, &MapView3D::OnActivateFrame, this);
        }
        
        void MapView3D::OnKey(wxKeyEvent& event) {
            m_movementRestriction.setVerticalRestriction(event.AltDown());
            Refresh();
            event.Skip();
        }
        
        void MapView3D::OnToggleMovementRestriction(wxCommandEvent& event) {
            m_movementRestriction.toggleHorizontalRestriction(m_camera);
            Refresh();
        }

        void MapView3D::OnMoveTexturesUp(wxCommandEvent& event) {
            moveTextures(Vec2f(0.0f, moveTextureDistance()));
        }
        
        void MapView3D::OnMoveTexturesDown(wxCommandEvent& event) {
            moveTextures(Vec2f(0.0f, -moveTextureDistance()));
        }
        
        void MapView3D::OnMoveTexturesLeft(wxCommandEvent& event) {
            moveTextures(Vec2f(-moveTextureDistance(), 0.0f));
        }
        
        void MapView3D::OnMoveTexturesRight(wxCommandEvent& event) {
            moveTextures(Vec2f(moveTextureDistance(), 0.0f));
        }
        
        void MapView3D::OnRotateTexturesCW(wxCommandEvent& event) {
            rotateTextures(rotateTextureAngle(true));
        }
        
        void MapView3D::OnRotateTexturesCCW(wxCommandEvent& event) {
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
            toggleCameraFlyMode();
        }

        void MapView3D::OnKillFocus(wxFocusEvent& event) {
            if (cameraFlyModeActive())
                toggleCameraFlyMode();
            event.Skip();
        }

        void MapView3D::OnActivateFrame(wxActivateEvent& event) {
            if (cameraFlyModeActive())
                toggleCameraFlyMode();
            event.Skip();
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

        Vec3 MapView3D::doGetPasteObjectsDelta(const BBox3& bounds) const {
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
                    const Vec3 snappedHitPoint = grid.snap(hit.hitPoint());
                    const Plane3 dragPlane = alignedOrthogonalDragPlane(snappedHitPoint, face->boundary().normal);
                    return grid.moveDeltaForBounds(dragPlane, bounds, document->worldBounds(), pickRay, snappedHitPoint);
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
        
        void MapView3D::doCenterCameraOnSelection() {
            MapDocumentSPtr document = lock(m_document);
            const Model::EntityList& entities = document->selectedNodes().entities();
            const Model::BrushList& brushes = document->selectedNodes().brushes();
            assert(!entities.empty() || !brushes.empty());
            
            const Vec3 newPosition = centerCameraOnObjectsPosition(entities, brushes);
            moveCameraToPosition(newPosition);
        }
        
        Vec3f MapView3D::centerCameraOnObjectsPosition(const Model::EntityList& entities, const Model::BrushList& brushes) {
            Model::EntityList::const_iterator entityIt, entityEnd;
            Model::BrushList::const_iterator brushIt, brushEnd;
            
            float minDist = std::numeric_limits<float>::max();
            Vec3 center;
            size_t count = 0;
            
            for (entityIt = entities.begin(), entityEnd = entities.end(); entityIt != entityEnd; ++entityIt) {
                const Model::Entity* entity = *entityIt;
                if (!entity->hasChildren()) {
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
                if (!entity->hasChildren()) {
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
                    DEFAULT_SWITCH()
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

        void MapView3D::doRenderMap(Renderer::MapRenderer& renderer, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            renderer.render(renderContext, renderBatch);
        }
        
        void MapView3D::doRenderTools(MapViewToolBox& toolBox, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            renderTools(renderContext, renderBatch);
        }
        
        void MapView3D::doRenderExtras(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_compass->render(renderBatch);
        }
        
        void MapView3D::doLinkCamera(CameraLinkHelper& helper) {}
    }
}
