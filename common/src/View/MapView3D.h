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

#ifndef __TrenchBroom__MapView3D__
#define __TrenchBroom__MapView3D__

#include "MathUtils.h"
#include "Model/ModelTypes.h"
#include "Renderer/PerspectiveCamera.h"
#include "View/Action.h"
#include "View/MapViewBase.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    class Logger;

    namespace Renderer {
        class Compass;
        class MapRenderer;
        class RenderBatch;
        class RenderContext;
    }
    
    namespace View {
        class FlyModeHelper;
        class MapView3D : public MapViewBase {
        private:
            Renderer::PerspectiveCamera m_camera;
            Renderer::Compass* m_compass;
            FlyModeHelper* m_flyModeHelper;
        public:
            MapView3D(wxWindow* parent, Logger* logger, MapDocumentWPtr document, MapViewToolBox& toolBox, Renderer::MapRenderer& renderer, Renderer::Vbo& vbo);
            ~MapView3D();
            
        public: // camera control
            bool cameraFlyModeActive() const;
            void toggleCameraFlyMode();
            
            void moveCameraToNextTracePoint();
            void moveCameraToPreviousTracePoint();
        private: // interaction events
            void bindEvents();
            
            void OnMoveTexturesUp(wxCommandEvent& event);
            void OnMoveTexturesDown(wxCommandEvent& event);
            void OnMoveTexturesLeft(wxCommandEvent& event);
            void OnMoveTexturesRight(wxCommandEvent& event);
            void OnRotateTexturesCW(wxCommandEvent& event);
            void OnRotateTexturesCCW(wxCommandEvent& event);

            float moveTextureDistance() const;
            void moveTextures(const Vec2f& offset);
            float rotateTextureAngle(bool clockwise) const;
            void rotateTextures(float angle);
        private: // tool mode events
            void OnToggleFlyMode(wxCommandEvent& event);
        private: // other events
            void OnKillFocus(wxFocusEvent& event);
            void OnActivateFrame(wxActivateEvent& event);
        private: // implement MapViewBase interface
            Renderer::Camera* doGetCamera();
            const Renderer::Camera* doGetCamera() const;
            void doCenterCameraOnSelection();
            void doMoveCameraToPosition(const Vec3& point);
            
            Vec3f centerCameraOnObjectsPosition(const Model::EntityList& entities, const Model::BrushList& brushes);
            void animateCamera(const Vec3f& position, const Vec3f& direction, const Vec3f& up, const wxLongLong duration = 150);
            
            ActionContext doGetActionContext() const;
            wxAcceleratorTable doCreateAccelerationTable(ActionContext context) const;
            bool doCancel();
            Renderer::RenderContext doCreateRenderContext() const;
            void doRenderMap(Renderer::MapRenderer& renderer, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void doRenderTools(MapViewToolBox& toolBox, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void doRenderExtras(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
        private:
            static const GLContextHolder::GLAttribs& buildAttribs();
        };
    }
}

#endif /* defined(__TrenchBroom__MapView3D__) */
