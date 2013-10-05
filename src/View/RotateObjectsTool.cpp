/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "RotateObjectsTool.h"

namespace TrenchBroom {
    namespace View {
        RotateObjectsTool::RotateObjectsTool(BaseTool* next, MapDocumentPtr document, ControllerPtr controller) :
        Tool(next, document, controller) {}

        bool RotateObjectsTool::initiallyActive() const {
            return false;
        }
        
        bool RotateObjectsTool::doActivate(const InputState& inputState) {
            return true;
        }
        
        bool RotateObjectsTool::doDeactivate(const InputState& inputState) {
            return true;
        }
        
        void RotateObjectsTool::doPick(const InputState& inputState, Model::PickResult& pickResult) const {
        }
        
        void RotateObjectsTool::doMouseMove(const InputState& inputState) {
        }
        
        bool RotateObjectsTool::doStartMouseDrag(const InputState& inputState) {
            return false;
        }
        
        bool RotateObjectsTool::doMouseDrag(const InputState& inputState) {
            return true;
        }
        
        void RotateObjectsTool::doEndMouseDrag(const InputState& inputState) {
        }
            
        void RotateObjectsTool::doCancelMouseDrag(const InputState& inputState) {
        }
        
        void RotateObjectsTool::doRender(const InputState& inputState, Renderer::RenderContext& renderContext) {
        }
    }
}
