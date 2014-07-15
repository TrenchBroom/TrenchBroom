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

#ifndef __TrenchBroom__SetFaceAttribsTool__
#define __TrenchBroom__SetFaceAttribsTool__

#include "View/Tool.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace View {
        class SetFaceAttribsTool : public ToolImpl<NoActivationPolicy, NoPickingPolicy, MousePolicy, NoMouseDragPolicy, NoDropPolicy, NoRenderPolicy> {
        public:
            SetFaceAttribsTool(MapDocumentWPtr document, ControllerWPtr controller);
        private:
            bool doMouseUp(const InputState& inputState);
            bool doMouseDoubleClick(const InputState& inputState);
            
            bool applies(const InputState& inputState) const;
            bool copyAttributes(const InputState& inputState) const;
            
            bool performCopy(const InputState& inputState, bool applyToBrush);
        };
    }
}

#endif /* defined(__TrenchBroom__SetFaceAttribsTool__) */
