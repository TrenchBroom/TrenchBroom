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

#ifndef __TrenchBroom__UVOffsetTool__
#define __TrenchBroom__UVOffsetTool__

#include "Model/ModelTypes.h"
#include "View/Tool.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace View {
        class UVViewHelper;
        
        class UVOffsetTool : public ToolImpl<NoActivationPolicy, NoPickingPolicy, NoMousePolicy, MouseDragPolicy, NoDropPolicy, NoRenderPolicy> {
        private:
            const UVViewHelper& m_helper;
            Vec2f m_lastPoint;
        public:
            UVOffsetTool(MapDocumentWPtr document, const UVViewHelper& helper);
        private:
            bool doStartMouseDrag(const InputState& inputState);
            bool doMouseDrag(const InputState& inputState);
            void doEndMouseDrag(const InputState& inputState);
            void doCancelMouseDrag();
            
            Vec2f computeHitPoint(const Ray3& ray) const;
            Vec2f snapDelta(const Vec2f& delta) const;
        };
    }
}

#endif /* defined(__TrenchBroom__UVOffsetTool__) */
