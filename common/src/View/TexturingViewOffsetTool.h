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

#ifndef __TrenchBroom__TexturingViewOffsetTool__
#define __TrenchBroom__TexturingViewOffsetTool__

#include "Model/ModelTypes.h"
#include "View/Tool.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace View {
        class TexturingViewHelper;
        
        class TexturingViewOffsetTool : public ToolImpl<NoActivationPolicy, NoPickingPolicy, NoMousePolicy, MouseDragPolicy, NoDropPolicy, NoRenderPolicy> {
        private:
            const TexturingViewHelper& m_helper;
            Vec2f m_lastPoint;
        public:
            TexturingViewOffsetTool(MapDocumentWPtr document, ControllerWPtr controller, const TexturingViewHelper& helper);
        private:
            bool doStartMouseDrag(const InputState& inputState);
            bool doMouseDrag(const InputState& inputState);
            void doEndMouseDrag(const InputState& inputState);
            void doCancelMouseDrag(const InputState& inputState);
            
            Vec3 computeTexPoint(const Ray3& ray, const Model::BrushFace* face) const;
        };
    }
}

#endif /* defined(__TrenchBroom__TexturingViewOffsetTool__) */
