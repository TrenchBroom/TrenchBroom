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
#include "View/UVGridTool.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace View {
        class UVViewHelper;
        
        class UVShearTool : public UVGridTool {
        private:
            Vec2 m_axisLength;
        public:
            UVShearTool(MapDocumentWPtr document, ControllerWPtr controller, UVViewHelper& helper);
        private:
            bool checkIfDragApplies(const InputState& inputState, const Hit& xHit, const Hit& yHit) const;
            String getActionName() const;
            
            void startDrag(const Vec2f& pos);
            Vec2f performDrag(const Vec2f& delta);
            Vec2f snap(const Vec2f& position) const;
            
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext);
        };
    }
}

#endif /* defined(__TrenchBroom__UVShearTool__) */
