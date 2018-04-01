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

#ifndef TrenchBroom_MapView2D
#define TrenchBroom_MapView2D

#include "MathUtils.h"
#include "Model/ModelTypes.h"
#include "Renderer/OrthographicCamera.h"
#include "View/Action.h"
#include "View/MapViewBase.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    class Logger;

    namespace Model {
        class PickResult;
    }
    
    namespace Renderer {
        class Compass;
        class MapRenderer;
        class RenderBatch;
        class RenderContext;
    }
    
    namespace View {
        class MapView2D : public MapViewBase {
        public:
            typedef enum {
                ViewPlane_XY,
                ViewPlane_XZ,
                ViewPlane_YZ
            } ViewPlane;
        private:
            Renderer::OrthographicCamera m_camera;
        public:
            MapView2D(wxWindow* parent, Logger* logger, MapDocumentWPtr document, MapViewToolBox& toolBox, Renderer::MapRenderer& renderer, GLContextManager& contextManager, ViewPlane viewPlane);
            ~MapView2D() override;
        private:
            void initializeCamera(ViewPlane viewPlane);
            void initializeToolChain(MapViewToolBox& toolBox);
        private: // notification
            void bindObservers();
            void unbindObservers();
            void cameraDidChange(const Renderer::Camera* camera);
        private: // implement ToolBoxConnector interface
            PickRequest doGetPickRequest(int x, int y) const override;
            Model::PickResult doPick(const Ray3& pickRay) const override;
        private: // implement RenderView interface
            void doUpdateViewport(int x, int y, int width, int height) override;
        private: // implement MapView interface
            Vec3 doGetPasteObjectsDelta(const BBox3& bounds, const BBox3& referenceBounds) const override;
            bool doCanSelectTall() override;
            void doSelectTall() override;
            void doFocusCameraOnSelection(bool animate) override;
            
            void doMoveCameraToPosition(const Vec3& position, bool animate) override;
            void animateCamera(const Vec3f& position, const Vec3f& direction, const Vec3f& up, const wxLongLong duration = DefaultCameraAnimationDuration);
            
            void doMoveCameraToCurrentTracePoint() override;
        private: // implement MapViewBase interface
            Vec3 doGetMoveDirection(Math::Direction direction) const override;
            Vec3 doComputePointEntityPosition(const BBox3& bounds) const override;

            ActionContext doGetActionContext() const override;
            wxAcceleratorTable doCreateAccelerationTable(ActionContext context) const override;
            bool doCancel() override;
            
            Renderer::RenderContext::RenderMode doGetRenderMode() override;
            Renderer::Camera& doGetCamera() override;
            void doRenderGrid(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override;
            void doRenderMap(Renderer::MapRenderer& renderer, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override;
            void doRenderTools(MapViewToolBox& toolBox, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override;
            void doRenderExtras(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override;
        private: // implement CameraLinkableView interface
            void doLinkCamera(CameraLinkHelper& linkHelper) override;
        };
    }
}

#endif /* defined(TrenchBroom_MapView2D) */
