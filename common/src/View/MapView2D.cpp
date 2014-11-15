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

#include "MapView2D.h"
#include "Logger.h"
#include "Model/CompareHitsBySize.h"
#include "Renderer/MapRenderer.h"
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
#include "View/wxUtils.h"

namespace TrenchBroom {
    namespace View {
        InputSource inputSource(MapView2D::ViewPlane viewPlane);
        InputSource inputSource(const MapView2D::ViewPlane viewPlane) {
            switch (viewPlane) {
                case MapView2D::ViewPlane_XY:
                    return IS_MapViewXY;
                case MapView2D::ViewPlane_XZ:
                    return IS_MapViewXZ;
                case MapView2D::ViewPlane_YZ:
                    return IS_MapViewYZ;
            }
        }
        
        MapView2D::MapView2D(wxWindow* parent, Logger* logger, MapDocumentWPtr document, MapViewToolBox& toolBox, Renderer::MapRenderer& renderer, Renderer::Vbo& vbo, const ViewPlane viewPlane, GLContextHolder::Ptr sharedContext) :
        MapViewBase(parent, logger, document, toolBox, renderer, vbo, inputSource(viewPlane), sharedContext),
        m_camera() {
            bindEvents();
            bindObservers();
            initializeCamera(viewPlane);
        }

        MapView2D::~MapView2D() {
        }
        
        void MapView2D::initializeCamera(const ViewPlane viewPlane) {
            switch (viewPlane) {
                case MapView2D::ViewPlane_XY:
                    m_camera.setDirection(Vec3f::NegZ, Vec3f::PosY);
                    m_camera.moveTo(Vec3f(0.0f, 0.0f, 16384.0f));
                    break;
                case MapView2D::ViewPlane_XZ:
                    m_camera.setDirection(Vec3f::PosY, Vec3f::PosZ);
                    m_camera.moveTo(Vec3f(0.0f, -16384.0f, 0.0f));
                    break;
                case MapView2D::ViewPlane_YZ:
                    m_camera.setDirection(Vec3f::NegX, Vec3f::PosZ);
                    m_camera.moveTo(Vec3f(16384.0f, 0.0f, 0.0f));
                    break;
            }
            m_camera.setNearPlane(1.0f);
            m_camera.setFarPlane(32768.0f);
            
        }

        void MapView2D::bindEvents() {
            /*
            Bind(wxEVT_KEY_DOWN, &MapView2D::OnKey, this);
            Bind(wxEVT_KEY_UP, &MapView2D::OnKey, this);
            */
            
            /*
            Bind(wxEVT_MENU, &MapView2D::OnPopupReparentBrushes,         this, CommandIds::CreateEntityPopupMenu::ReparentBrushes);
            Bind(wxEVT_MENU, &MapView2D::OnPopupMoveBrushesToWorld,      this, CommandIds::CreateEntityPopupMenu::MoveBrushesToWorld);
            Bind(wxEVT_MENU, &MapView2D::OnPopupCreatePointEntity,       this, CommandIds::CreateEntityPopupMenu::LowestPointEntityItem, CommandIds::CreateEntityPopupMenu::HighestPointEntityItem);
            Bind(wxEVT_MENU, &MapView2D::OnPopupCreateBrushEntity,       this, CommandIds::CreateEntityPopupMenu::LowestBrushEntityItem, CommandIds::CreateEntityPopupMenu::HighestBrushEntityItem);
            
            Bind(wxEVT_UPDATE_UI, &MapView2D::OnUpdatePopupMenuItem, this, CommandIds::CreateEntityPopupMenu::ReparentBrushes);
            Bind(wxEVT_UPDATE_UI, &MapView2D::OnUpdatePopupMenuItem, this, CommandIds::CreateEntityPopupMenu::MoveBrushesToWorld);
            Bind(wxEVT_UPDATE_UI, &MapView2D::OnUpdatePopupMenuItem, this, CommandIds::CreateEntityPopupMenu::LowestPointEntityItem, CommandIds::CreateEntityPopupMenu::HighestPointEntityItem);
            Bind(wxEVT_UPDATE_UI, &MapView2D::OnUpdatePopupMenuItem, this, CommandIds::CreateEntityPopupMenu::LowestBrushEntityItem, CommandIds::CreateEntityPopupMenu::HighestBrushEntityItem);
            */
        }

        Hits MapView2D::doPick(const Ray3& pickRay) const {
            Hits hits = Model::hitsBySize(pickRay.direction.firstComponent());
            lock(m_document)->pick(pickRay, hits);
            return hits;
        }
        
        Renderer::Camera* MapView2D::doGetCamera() {
            return &m_camera;
        }
        
        const Renderer::Camera* MapView2D::doGetCamera() const {
            return &m_camera;
        }
        
        void MapView2D::doCenterCameraOnSelection() {
            const MapDocumentSPtr document = lock(m_document);
            assert(!document->selectedNodes().empty());

            const BBox3& bounds = document->selectionBounds();
            moveCameraToPosition(bounds.center());
        }

        void MapView2D::doMoveCameraToPosition(const Vec3& point) {
            animateCamera(Vec3f(point), m_camera.direction(), m_camera.up());
        }

        void MapView2D::animateCamera(const Vec3f& position, const Vec3f& direction, const Vec3f& up, const wxLongLong duration) {
            const Vec3f actualPosition = position.dot(m_camera.up()) * m_camera.up() + position.dot(m_camera.right()) * m_camera.right() + m_camera.position().dot(m_camera.direction()) * m_camera.direction();
            CameraAnimation* animation = new CameraAnimation(m_camera, actualPosition, m_camera.direction(), m_camera.up(), duration);
            m_animationManager->runAnimation(animation, true);
        }

        ActionContext MapView2D::doGetActionContext() const {
            return ActionContext_Default;
        }
        
        wxAcceleratorTable MapView2D::doCreateAccelerationTable(ActionContext context) const {
            ActionManager& actionManager = ActionManager::instance();
            return actionManager.createViewAcceleratorTable(context, ActionView_Map2D);
        }
        
        bool MapView2D::doCancel() {
            return false;
        }
        
        Renderer::RenderContext MapView2D::doCreateRenderContext() const {
            return Renderer::RenderContext(Renderer::RenderContext::RenderMode_2D, m_camera, contextHolder()->fontManager(), contextHolder()->shaderManager());
        }

        void MapView2D::doRenderMap(Renderer::MapRenderer& renderer, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            renderer.render(renderContext, renderBatch);
        }
        
        void MapView2D::doRenderTools(MapViewToolBox& toolBox, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            renderTools(renderContext, renderBatch);
        }
        
        void MapView2D::doRenderExtras(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
        }
    }
}
