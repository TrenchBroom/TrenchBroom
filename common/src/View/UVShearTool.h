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

#ifndef __TrenchBroom__UVShearTool__
#define __TrenchBroom__UVShearTool__

#include "Hit.h"
#include "View/Tool.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace View {
        class UVViewHelper;
        
        class UVShearTool : public ToolImpl<NoActivationPolicy, PickingPolicy, NoMousePolicy, MouseDragPolicy, NoDropPolicy, NoRenderPolicy> {
        private:
            static const Hit::HitType XHandleHit;
            static const Hit::HitType YHandleHit;
        private:
            UVViewHelper& m_helper;
            
            Vec2b m_selector;
            Vec3 m_xAxis;
            Vec3 m_yAxis;
            Vec2f m_initialHit;
            Vec2f m_lastHit;
        public:
            UVShearTool(MapDocumentWPtr document, UVViewHelper& helper);
        private:
            void doPick(const InputState& inputState, Hits& hits);
            
            bool doStartMouseDrag(const InputState& inputState);
            bool doMouseDrag(const InputState& inputState);
            void doEndMouseDrag(const InputState& inputState);
            void doCancelMouseDrag();
            
            Vec2f getHit(const Ray3& pickRay) const;
        };
    }
}

#endif /* defined(__TrenchBroom__UVShearTool__) */
