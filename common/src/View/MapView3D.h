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

#include <vecmath/scalar.h>
#include "Model/Model_Forward.h"
#include "Renderer/PerspectiveCamera.h"
#include "View/MapViewBase.h"
#include "View/ViewTypes.h"

#include <vector>

class QKeyEvent;

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
            Q_OBJECT
        private:
            Renderer::PerspectiveCamera m_camera;
            FlyModeHelper* m_flyModeHelper;
            bool m_ignoreCameraChangeEvents;
        public:
            MapView3D(MapDocumentWPtr document, MapViewToolBox& toolBox, Renderer::MapRenderer& renderer,
                      GLContextManager& contextManager, Logger* logger);
            ~MapView3D() override;
        private:
            void initializeCamera();
            void initializeToolChain(MapViewToolBox& toolBox);
        private: // notification
            void bindObservers();
            void unbindObservers();
            void cameraDidChange(const Renderer::Camera* camera);
            void preferenceDidChange(const IO::Path& path);
        protected: // QWidget overrides
            void keyPressEvent(QKeyEvent* event) override;
            void keyReleaseEvent(QKeyEvent* event) override;
            void focusInEvent(QFocusEvent* event) override;
            void focusOutEvent(QFocusEvent* event) override;
        protected: // QOpenGLWidget overrides
            void initializeGL() override;
        private: // interaction events
            void bindEvents();
        private: // other events
            void updateFlyMode();
            void resetFlyModeKeys();
        private: // implement ToolBoxConnector interface
            PickRequest doGetPickRequest(int x, int y) const override;
            Model::PickResult doPick(const vm::ray3& pickRay) const override;
        private: // implement RenderView interface
            void doUpdateViewport(int x, int y, int width, int height) override;
        private: // implement MapView interface
            vm::vec3 doGetPasteObjectsDelta(const vm::bbox3& bounds, const vm::bbox3& referenceBounds) const override;

            bool doCanSelectTall() override;
            void doSelectTall() override;

            void doFocusCameraOnSelection(bool animate) override;

            class ComputeCameraCenterPositionVisitor;
            class ComputeCameraCenterOffsetVisitor;
            vm::vec3 focusCameraOnObjectsPosition(const std::vector<Model::Node*>& nodes);

            void doMoveCameraToPosition(const vm::vec3& position, bool animate) override;
            void animateCamera(const vm::vec3f& position, const vm::vec3f& direction, const vm::vec3f& up, const int duration = DefaultCameraAnimationDuration);

            void doMoveCameraToCurrentTracePoint() override;
        private: // implement MapViewBase interface
            vm::vec3 doGetMoveDirection(vm::direction direction) const override;
            vm::vec3 doComputePointEntityPosition(const vm::bbox3& bounds) const override;

            ActionContext::Type doGetActionContext() const override;
            ActionView doGetActionView() const override;
            bool doCancel() override;

            Renderer::RenderContext::RenderMode doGetRenderMode() override;
            Renderer::Camera& doGetCamera() override;
            void doPreRender() override;
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
