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

#ifndef TrenchBroom_MapView3D
#define TrenchBroom_MapView3D

#include "MathUtils.h"
#include "Model/ModelTypes.h"
#include "Renderer/PerspectiveCamera.h"
#include "View/Action.h"
#include "View/MapViewBase.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    class Logger;

    namespace Model {
        class PickResult;
    }
    
    namespace Renderer {
        class MapRenderer;
        class RenderBatch;
        class RenderContext;
    }
    
    namespace View {
        class MapView3D : public MapViewBase {
        private:
            Renderer::PerspectiveCamera m_camera;
            FlyModeHelper* m_flyModeHelper;
        public:
            MapView3D(wxWindow* parent, Logger* logger, MapDocumentWPtr document, MapViewToolBox& toolBox, Renderer::MapRenderer& renderer, GLContextManager& contextManager);
            ~MapView3D() override;
        private:
            void initializeCamera();
            void initializeToolChain(MapViewToolBox& toolBox);
        public: // camera control
            bool cameraFlyModeActive() const;
            void toggleCameraFlyMode();
        private: // notification
            void bindObservers();
            void unbindObservers();
            void cameraDidChange(const Renderer::Camera* camera);
        private: // interaction events
            void bindEvents();
            
            void OnKeyDown(wxKeyEvent& event);
            void OnKeyUp(wxKeyEvent& event);

            void OnMouseMotion(wxMouseEvent& event);
            
            void OnPerformCreateBrush(wxCommandEvent& event);

            void OnMoveTexturesUp(wxCommandEvent& event);
            void OnMoveTexturesDown(wxCommandEvent& event);
            void OnMoveTexturesLeft(wxCommandEvent& event);
            void OnMoveTexturesRight(wxCommandEvent& event);
            void OnRotateTexturesCW(wxCommandEvent& event);
            void OnRotateTexturesCCW(wxCommandEvent& event);

            float moveTextureDistance() const;
            void moveTextures(const vec2f& offset);
            float rotateTextureAngle(bool clockwise) const;
            void rotateTextures(float angle);
        private: // tool mode events
            void OnToggleFlyMode(wxCommandEvent& event);
        private: // other events
            void OnKillFocus(wxFocusEvent& event);
            void OnActivateFrame(wxActivateEvent& event);
        private: // implement ToolBoxConnector interface
            PickRequest doGetPickRequest(int x, int y) const override;
            Model::PickResult doPick(const Ray3& pickRay) const override;
        private: // implement RenderView interface
            void doUpdateViewport(int x, int y, int width, int height) override;
        private: // implement MapView interface
            vec3 doGetPasteObjectsDelta(const bbox3& bounds, const bbox3& referenceBounds) const override;

            bool doCanSelectTall() override;
            void doSelectTall() override;

            void doFocusCameraOnSelection(bool animate) override;
            
            class ComputeCameraCenterPositionVisitor;
            class ComputeCameraCenterOffsetVisitor;
            vec3 focusCameraOnObjectsPosition(const Model::NodeList& nodes);

            void doMoveCameraToPosition(const vec3& position, bool animate) override;
            void animateCamera(const vec3f& position, const vec3f& direction, const vec3f& up, const wxLongLong duration = DefaultCameraAnimationDuration);
            
            void doMoveCameraToCurrentTracePoint() override;
        private: // implement MapViewBase interface
            vec3 doGetMoveDirection(Math::Direction direction) const override;
            vec3 doComputePointEntityPosition(const bbox3& bounds) const override;
            
            ActionContext doGetActionContext() const override;
            wxAcceleratorTable doCreateAccelerationTable(ActionContext context) const override;
            bool doCancel() override;
            
            Renderer::RenderContext::RenderMode doGetRenderMode() override;
            Renderer::Camera& doGetCamera() override;
            void doRenderGrid(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override;
            void doRenderMap(Renderer::MapRenderer& renderer, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override;
            void doRenderTools(MapViewToolBox& toolBox, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override;
            
            bool doBeforePopupMenu() override;
        private: // implement CameraLinkableView interface
            void doLinkCamera(CameraLinkHelper& linkHelper) override;
        };
    }
}

#endif /* defined(TrenchBroom_MapView3D) */
