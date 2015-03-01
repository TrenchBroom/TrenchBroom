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

#ifndef __TrenchBroom__CreateBrushToolAdapter3D__
#define __TrenchBroom__CreateBrushToolAdapter3D__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "View/ToolAdapter.h"

#include <vector>

namespace TrenchBroom {
    namespace View {
        class CreateBrushTool;
        class Grid;

        class CreateBrushToolAdapter3D : public ToolAdapterBase<NoPickingPolicy, KeyPolicy, MousePolicy, PlaneDragPolicy, RenderPolicy, NoDropPolicy> {
        private:
            class PointHistory {
            private:
                typedef std::vector<size_t> IndexList;
                Vec3::List m_points;
                IndexList m_indices;
            public:
                bool empty() const;
                const Vec3::List& points() const;
                
                void push(const Vec3& point);
                void push(const Vec3::List& points);
                void pop();
                void clear();
            };
        private:
            class DragHelper;
            class CreatePolygonDragHelper;
            
            CreateBrushTool* m_tool;
            const Grid& m_grid;
            PointHistory m_history;
            
            CreatePolygonDragHelper* m_createPolygonDragHelper;
            DragHelper* m_dragHelper;
        public:
            CreateBrushToolAdapter3D(CreateBrushTool* tool, const Grid& grid);
        public:
            virtual ~CreateBrushToolAdapter3D();
        private:
            Tool* doGetTool();

            void doModifierKeyChange(const InputState& inputState);
            bool doMouseClick(const InputState& inputState);

            bool doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint);
            bool doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint);
            void doEndPlaneDrag(const InputState& inputState);
            void doCancelPlaneDrag();
            void doResetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint);

            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const;
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);

            bool doCancel();
        private:
            void updateTool();
        };
    }
}

#endif /* defined(__TrenchBroom__CreateBrushToolAdapter3D__) */
