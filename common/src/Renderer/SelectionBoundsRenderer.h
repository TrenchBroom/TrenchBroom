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

#ifndef TrenchBroom_SelectionBoundsRenderer
#define TrenchBroom_SelectionBoundsRenderer

#include "TrenchBroom.h"
#include "VecMath.h"

namespace TrenchBroom {
    namespace Renderer {
        class RenderBatch;
        class RenderContext;
        
        class SelectionBoundsRenderer {
        private:
            const BBox3 m_bounds;
            
            class SizeTextAnchor2D;
            class SizeTextAnchor3D;
            class MinMaxTextAnchor3D;
        public:
            SelectionBoundsRenderer(const BBox3& bounds);
            
            void render(RenderContext& renderContext, RenderBatch& renderBatch);
        private:
            void renderBounds(RenderContext& renderContext, RenderBatch& renderBatch);
            void renderSize(RenderContext& renderContext, RenderBatch& renderBatch);
            void renderSize2D(RenderContext& renderContext, RenderBatch& renderBatch);
            void renderSize3D(RenderContext& renderContext, RenderBatch& renderBatch);
            void renderMinMax(RenderContext& renderContext, RenderBatch& renderBatch);
        };
    }
}

#endif /* defined(TrenchBroom_SelectionBoundsRenderer) */
