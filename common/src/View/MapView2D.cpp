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
#include "Model/Brush.h"
#include "Model/BrushVertex.h"
#include "Model/Entity.h"
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
#include "View/MapDocument.h"
#include "View/MapViewToolBox.h"
#include "View/wxUtils.h"

namespace TrenchBroom {
    namespace View {
        MapView2D::MapView2D(wxWindow* parent, Logger* logger, MapDocumentWPtr document, MapViewToolBox& toolBox, Renderer::MapRenderer& renderer, Renderer::Vbo& vbo, GLContextHolder::Ptr sharedContext) :
        MapViewBase(parent, logger, document, toolBox, renderer, vbo, sharedContext),
        m_camera() {
            bindEvents();
            bindObservers();
        }

        MapView2D::~MapView2D() {
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

        Renderer::Camera* MapView2D::doGetCamera() {
            return &m_camera;
        }
        
        const Renderer::Camera* MapView2D::doGetCamera() const {
            return &m_camera;
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
