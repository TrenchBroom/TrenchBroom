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

#include "CreateBrushToolAdapter2D.h"

#include "View/CreateBrushTool.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        CreateBrushToolAdapter2D::CreateBrushToolAdapter2D(CreateBrushTool* tool) :
        m_tool(tool) {
            assert(m_tool != NULL);
        }

        CreateBrushToolAdapter2D::~CreateBrushToolAdapter2D() {}
        
        Tool* CreateBrushToolAdapter2D::doGetTool() {
            return m_tool;
        }
        
        bool CreateBrushToolAdapter2D::doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            return false;
        }
        
        bool CreateBrushToolAdapter2D::doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) {
            return true;
        }
        
        void CreateBrushToolAdapter2D::doEndPlaneDrag(const InputState& inputState) {}
        
        void CreateBrushToolAdapter2D::doCancelPlaneDrag() {}
        
        void CreateBrushToolAdapter2D::doResetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {}
        
        void CreateBrushToolAdapter2D::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {}
        
        void CreateBrushToolAdapter2D::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {}

        bool CreateBrushToolAdapter2D::doCancel() {
            return false;
        }
    }
}
