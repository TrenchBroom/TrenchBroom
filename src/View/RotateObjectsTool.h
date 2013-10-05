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

#ifndef __TrenchBroom__RotateObjectsTool__
#define __TrenchBroom__RotateObjectsTool__

#include "View/Tool.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace View {
        class RotateObjectsTool : public Tool<ActivationPolicy, PickingPolicy, MousePolicy, MouseDragPolicy, RenderPolicy> {
        public:
            RotateObjectsTool(BaseTool* next, MapDocumentPtr document, ControllerPtr controller);
        private:
            bool initiallyActive() const;
            bool doActivate(const InputState& inputState);
            bool doDeactivate(const InputState& inputState);
            
            void doPick(const InputState& inputState, Model::PickResult& pickResult) const;

            void doMouseMove(const InputState& inputState);
            
            bool doStartMouseDrag(const InputState& inputState);
            bool doMouseDrag(const InputState& inputState);
            void doEndMouseDrag(const InputState& inputState);
            void doCancelMouseDrag(const InputState& inputState);
            
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext);
        };
    }
}

#endif /* defined(__TrenchBroom__RotateObjectsTool__) */
