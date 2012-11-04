/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ClipTool.h"

#include "Controller/ClipHandle.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        bool ClipTool::handleActivated(InputEvent& event) {
            return true;
        }
        
        bool ClipTool::handleDeactivated(InputEvent& event) {
            return true;
        }

        bool ClipTool::handleMouseMoved(InputEvent& event) {
            
        }

        bool ClipTool::handleBeginPlaneDrag(InputEvent& event, Plane& dragPlane, Vec3f& initialDragPoint) {
            assert(active());
            return true;
        }
        
        bool ClipTool::handlePlaneDrag(InputEvent& event, const Vec3f& lastMousePoint, const Vec3f& curMousePoint, Vec3f& referencePoint) {
            assert(active());
            return true;
        }
        
        void ClipTool::handleEndPlaneDrag(InputEvent& event) {
            assert(active());
        }

        ClipTool::ClipTool(View::DocumentViewHolder& documentViewHolder, InputController& inputController) :
        DragTool(documentViewHolder, inputController),
        m_handle(ClipHandle(5.0f)) {}

        bool ClipTool::suppressOtherFeedback(InputEvent& event) {
            return active();
        }

        void ClipTool::toggleClipSide() {
            assert(active());
        }
        
        bool ClipTool::canPerformClip() {
            assert(active());
            return m_handle.numPoints() > 0;
        }
        
        void ClipTool::performClip() {
            assert(active());
            assert(canPerformClip());
        }
    }
}