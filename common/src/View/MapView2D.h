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

#ifndef __TrenchBroom__MapView2D__
#define __TrenchBroom__MapView2D__

#include "MathUtils.h"
#include "Model/ModelTypes.h"
#include "Renderer/OrthographicCamera.h"
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
        class CameraTool2D;
        class FlyModeHelper;
        class MapView2D : public MapViewBase {
        public:
            typedef enum {
                ViewPlane_XY,
                ViewPlane_XZ,
                ViewPlane_YZ
            } ViewPlane;
        private:
            Renderer::OrthographicCamera m_camera;
            CameraTool2D* m_cameraTool;
        public:
            MapView2D(wxWindow* parent, Logger* logger, MapDocumentWPtr document, MapViewToolBox& toolBox, Renderer::MapRenderer& renderer, Renderer::Vbo& vbo, ViewPlane viewPlane, GLContextHolder::Ptr sharedContext);
            ~MapView2D();
        private:
            void initializeCamera(ViewPlane viewPlane);
            void initializeToolChain(MapViewToolBox& toolBox);
        private: // interaction events
            void bindEvents();
        private: // implement ToolBoxConnector interface
            Hits doPick(const Ray3& pickRay) const;
        private: // implement MapViewBase interface
            Renderer::Camera* doGetCamera();
            const Renderer::Camera* doGetCamera() const;
            void doCenterCameraOnSelection();
            void doMoveCameraToPosition(const Vec3& point);
            void animateCamera(const Vec3f& position, const Vec3f& direction, const Vec3f& up, const wxLongLong duration = 150);
            
            ActionContext doGetActionContext() const;
            wxAcceleratorTable doCreateAccelerationTable(ActionContext context) const;
            bool doCancel();
            Renderer::RenderContext doCreateRenderContext() const;
            void doRenderMap(Renderer::MapRenderer& renderer, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void doRenderTools(MapViewToolBox& toolBox, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void doRenderExtras(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
        };
    }
}

#endif /* defined(__TrenchBroom__MapView2D__) */
