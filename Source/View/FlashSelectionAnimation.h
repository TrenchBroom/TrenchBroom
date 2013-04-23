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

#ifndef __TrenchBroom__FlashSelectionAnimation__
#define __TrenchBroom__FlashSelectionAnimation__

#include "View/Animation.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Renderer {
        class MapRenderer;
    }
    
    namespace View {
        class MapGLCanvas;
    }
    
    namespace View {
        class FlashSelectionAnimation : public Animation {
        private:
            Renderer::MapRenderer& m_renderer;
            View::MapGLCanvas& m_canvas;
        protected:
            void doUpdate(double progress);
        public:
            FlashSelectionAnimation(Renderer::MapRenderer& renderer, View::MapGLCanvas& canvas, wxLongLong duration);
            
            Type type() const;
        };
    }
}

#endif /* defined(__TrenchBroom__FlashSelectionAnimation__) */
