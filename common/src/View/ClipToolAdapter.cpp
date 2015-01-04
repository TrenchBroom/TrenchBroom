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

#include "ClipToolAdapter.h"

#include "Renderer/Camera.h"

namespace TrenchBroom {
    namespace View {
        ClipToolAdapter2D::ClipToolAdapter2D(ClipTool* tool, const Grid& grid) :
        ClipToolAdapter(tool, grid) {}

        bool ClipToolAdapter2D::doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            if (!canStartDrag(inputState))
                return false;
            
        }
        
        bool ClipToolAdapter2D::doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) {}
        
        void ClipToolAdapter2D::doEndPlaneDrag(const InputState& inputState) {}
        
        void ClipToolAdapter2D::doCancelPlaneDrag() {}
        
        void ClipToolAdapter2D::doResetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {}

        bool ClipToolAdapter2D::doAddClipPoint(const InputState& inputState) {}

        
        ClipToolAdapter3D::ClipToolAdapter3D(ClipTool* tool, const Grid& grid) :
        ClipToolAdapter(tool, grid) {}

        bool ClipToolAdapter3D::doStartMouseDrag(const InputState& inputState) {
            if (!canStartDrag(inputState))
                return false;
        }
        
        bool ClipToolAdapter3D::doMouseDrag(const InputState& inputState) {}
        
        void ClipToolAdapter3D::doEndMouseDrag(const InputState& inputState) {}
        
        void ClipToolAdapter3D::doCancelMouseDrag() {}
        
        bool ClipToolAdapter3D::doAddClipPoint(const InputState& inputState) {}
    }
}
