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
#include "Model/BrushVertex.h"
#include "Model/Entity.h"
#include "Model/PointFile.h"
#include "Renderer/Compass.h"
#include "Renderer/MapRenderer.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Vbo.h"
#include "View/ActionManager.h"
#include "View/Animation.h"
#include "View/CameraAnimation.h"
#include "View/CommandIds.h"
#include "View/FlashSelectionAnimation.h"
#include "View/FlyModeHelper.h"
#include "View/Grid.h"
#include "View/InputState.h"
#include "View/MapDocument.h"
#include "View/MapViewToolBox.h"
#include "View/wxUtils.h"

namespace TrenchBroom {
    namespace View {
        MapView3D::MapView3D(wxWindow* parent, Logger* logger, MapDocumentWPtr document, MapViewToolBox& toolBox, Renderer::MapRenderer& renderer, Renderer::Vbo& vbo) :
        MapViewBase(parent, logger, document, toolBox, renderer, vbo, IS_MapView3D, buildAttribs()),
        m_camera(),
        m_compass(new Renderer::Compass(toolBox.movementRestriction())),
        m_flyModeHelper(new FlyModeHelper(this, m_camera)) {
            bindEvents();
            bindObservers();
        }

        MapView3D::~MapView3D() {
            delete m_flyModeHelper;
            delete m_compass;
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

        void MapView3D::moveCameraToNextTracePoint() {
            MapDocumentSPtr document = lock(m_document);
            
            assert(document->isPointFileLoaded());
            Model::PointFile* pointFile = document->pointFile();
            assert(pointFile->hasNextPoint());
            
            const Vec3f position = pointFile->nextPoint() + Vec3f(0.0f, 0.0f, 16.0f);
            const Vec3f direction = pointFile->currentDirection();
            animateCamera(position, direction, Vec3f::PosZ);
        }
        
        void MapView3D::moveCameraToPreviousTracePoint() {
            MapDocumentSPtr document = lock(m_document);
            
            assert(document->isPointFileLoaded());
            Model::PointFile* pointFile = document->pointFile();
            assert(pointFile->hasPreviousPoint());
            
            const Vec3f position = pointFile->previousPoint() + Vec3f(0.0f, 0.0f, 16.0f);
            const Vec3f direction = pointFile->currentDirection();
            animateCamera(position, direction, Vec3f::PosZ);
        }

        void MapView3D::bindEvents() {
            /*
            Bind(wxEVT_KEY_DOWN, &MapView3D::OnKey, this);
            Bind(wxEVT_KEY_UP, &MapView3D::OnKey, this);
            */
            
            Bind(wxEVT_KILL_FOCUS, &MapView3D::OnKillFocus, this);
            
            /*
            Bind(wxEVT_MENU, &MapView3D::OnToggleMovementRestriction,    this, CommandIds::Actions::ToggleMovementRestriction);
             */
            
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

        Hits MapView3D::doPick(const Ray3& pickRay) const {
            Hits hits = hitsByDistance();
            lock(m_document)->pick(pickRay, hits);
            return hits;
        }

        Renderer::Camera* MapView3D::doGetCamera() {
            return &m_camera;
        }
        
        const Renderer::Camera* MapView3D::doGetCamera() const {
            return &m_camera;
        }
        
        void MapView3D::doCenterCameraOnSelection() {
            MapDocumentSPtr document = lock(m_document);
            const Model::EntityList& entities = document->selectedNodes().entities();
            const Model::BrushList& brushes = document->selectedNodes().brushes();
            assert(!entities.empty() || !brushes.empty());
            
            const Vec3 newPosition = centerCameraOnObjectsPosition(entities, brushes);
            moveCameraToPosition(newPosition);
        }

        void MapView3D::doMoveCameraToPosition(const Vec3& position) {
            animateCamera(position, camera()->direction(), camera()->up());
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
                        const Vec3f toPosition = vertex - camera()->position();
                        minDist = std::min(minDist, toPosition.dot(camera()->direction()));
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
                    const Vec3f toPosition = Vec3f(vertex->position) - camera()->position();
                    minDist = std::min(minDist, toPosition.dot(camera()->direction()));
                    center += vertex->position;
                    ++count;
                }
            }
            
            center /= static_cast<FloatType>(count);
            
            // act as if the camera were there already:
            const Vec3f oldPosition = camera()->position();
            camera()->moveTo(Vec3f(center));
            
            float offset = std::numeric_limits<float>::max();
            
            Plane3f frustumPlanes[4];
            camera()->frustumPlanes(frustumPlanes[0], frustumPlanes[1], frustumPlanes[2], frustumPlanes[3]);
            
            for (entityIt = entities.begin(), entityEnd = entities.end(); entityIt != entityEnd; ++entityIt) {
                const Model::Entity* entity = *entityIt;
                if (!entity->hasChildren()) {
                    const Vec3::List vertices = bBoxVertices(entity->bounds());
                    for (size_t i = 0; i < vertices.size(); ++i) {
                        const Vec3f vertex(vertices[i]);
                        
                        for (size_t j = 0; j < 4; ++j) {
                            const Plane3f& plane = frustumPlanes[j];
                            const float dist = (vertex - camera()->position()).dot(plane.normal) - 8.0f; // adds a bit of a border
                            offset = std::min(offset, -dist / camera()->direction().dot(plane.normal));
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
                        const float dist = (Vec3f(vertex->position) - camera()->position()).dot(plane.normal) - 8.0f; // adds a bit of a border
                        offset = std::min(offset, -dist / camera()->direction().dot(plane.normal));
                    }
                }
            }
            
            // jump back
            camera()->moveTo(oldPosition);
            
            return center + camera()->direction() * offset;
        }
        
        void MapView3D::animateCamera(const Vec3f& position, const Vec3f& direction, const Vec3f& up, const wxLongLong duration) {
            CameraAnimation* animation = new CameraAnimation(m_camera, position, direction, up, duration);
            m_animationManager->runAnimation(animation, true);
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
        
        Renderer::RenderContext MapView3D::doCreateRenderContext() const {
            return Renderer::RenderContext(Renderer::RenderContext::RenderMode_3D, m_camera, contextHolder()->fontManager(), contextHolder()->shaderManager());
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

        const GLContextHolder::GLAttribs& MapView3D::buildAttribs() {
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
    }
}
