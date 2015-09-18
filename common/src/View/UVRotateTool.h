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

#ifndef TrenchBroom_UVRotateTool
#define TrenchBroom_UVRotateTool

#include "Model/Hit.h"
#include "View/Tool.h"
#include "View/ToolAdapter.h"
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
        class UVViewHelper;

        class UVRotateTool : public ToolAdapterBase<PickingPolicy, NoKeyPolicy, NoMousePolicy, MouseDragPolicy, RenderPolicy, NoDropPolicy>, public Tool {
        public:
            static const Model::Hit::HitType AngleHandleHit;
        private:
            static const float CenterHandleRadius;
            static const float RotateHandleRadius;
            static const float RotateHandleWidth;
            
            MapDocumentWPtr m_document;
            UVViewHelper& m_helper;

            float m_initalAngle;
        public:
            UVRotateTool(MapDocumentWPtr document, UVViewHelper& helper);
        private:
            Tool* doGetTool();
            
            void doPick(const InputState& inputState, Model::PickResult& pickResult);
            
            bool doStartMouseDrag(const InputState& inputState);
            bool doMouseDrag(const InputState& inputState);
            
            float measureAngle(const Vec2f& point) const;
            float snapAngle(float angle) const;
            
            void doEndMouseDrag(const InputState& inputState);
            void doCancelMouseDrag();

            class Render;
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            
            bool doCancel();
        };
    }
}

#endif /* defined(TrenchBroom_UVRotateTool) */
