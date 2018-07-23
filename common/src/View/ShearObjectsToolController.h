#/*
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

#ifndef TrenchBroom_ShearObjectsToolController
#define TrenchBroom_ShearObjectsToolController

#include "Model/Hit.h"
#include "Renderer/EdgeRenderer.h"
#include "View/ToolController.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Model {
        class PickResult;
    }
    
    namespace Renderer {
        class RenderBatch;
        class RenderContext;
    }
    
    namespace View {
        class InputState;
        class ShearObjectsTool;
        
        class ShearObjectsToolController : public ToolControllerBase<PickingPolicy, KeyPolicy, MousePolicy, RestrictedDragPolicy, RenderPolicy, NoDropPolicy> {
        protected:
            ShearObjectsTool* m_tool;
        private:
            MapDocumentWPtr m_document;

            // debug visuals
            Vec3 m_debugInitialPoint;
            Vec3 m_lastDragDebug;
            Vec3 m_currentDragDebug;
            Line3 m_handleLineDebug;

        public:
            explicit ShearObjectsToolController(ShearObjectsTool* tool, MapDocumentWPtr document);
            ~ShearObjectsToolController() override;
        private:
            Tool* doGetTool() override;

            void doPick(const InputState& inputState, Model::PickResult& pickResult) override;
            virtual void doPick(const Ray3 &pickRay, const Renderer::Camera &camera, Model::PickResult &pickResult) = 0;
            
            void doModifierKeyChange(const InputState& inputState) override;
            
            void doMouseMove(const InputState& inputState) override;

//            bool doStartMouseDrag(const InputState& inputState) override;
//            bool doMouseDrag(const InputState& inputState) override;
//            void doEndMouseDrag(const InputState& inputState) override;
//            void doCancelMouseDrag() override;

            // RestrictedDragPolicy
            DragInfo doStartDrag(const InputState& inputState) override;
            DragResult doDrag(const InputState& inputState, const Vec3& lastHandlePosition, const Vec3& nextHandlePosition) override;
            void doEndDrag(const InputState& inputState) override;
            void doCancelDrag() override;

            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const override;

            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override;

            bool doCancel() override;

        protected:
            bool handleInput(const InputState& inputState) const;
        };
        
        class ShearObjectsToolController2D : public ShearObjectsToolController {
        public:
            explicit ShearObjectsToolController2D(ShearObjectsTool* tool, MapDocumentWPtr document);
        private:
            void doPick(const Ray3 &pickRay, const Renderer::Camera &camera, Model::PickResult &pickResult) override;
        };
        
        class ShearObjectsToolController3D : public ShearObjectsToolController {
        public:
            explicit ShearObjectsToolController3D(ShearObjectsTool* tool, MapDocumentWPtr document);
        private:
            void doPick(const Ray3 &pickRay, const Renderer::Camera &camera, Model::PickResult &pickResult) override;
        };
    }
}

#endif /* defined(TrenchBroom_ShearObjectsToolController) */

