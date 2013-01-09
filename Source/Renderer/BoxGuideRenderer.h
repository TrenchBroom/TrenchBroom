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

#ifndef __TrenchBroom__BoxGuideRenderer__
#define __TrenchBroom__BoxGuideRenderer__

#include "Model/Filter.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        class Picker;
    }
    
    namespace Renderer {
        class RenderContext;
        class VertexArray;
        class Vbo;
        
        class BoxGuideRenderer {
            BBox m_bounds;
            Model::Picker& m_picker;
            Model::VisibleFilter m_filter;
            VertexArray* m_boxArray;
            VertexArray* m_spikeArray;
            VertexArray* m_pointArray;
            
            void addSpike(const Vec3f& startPoint, const Vec3f& direction, const Color& color, Vec3f::List& hitPoints);
        public:
            BoxGuideRenderer(const BBox& bounds, Model::Picker& picker, Model::Filter& defaultFilter);
            ~BoxGuideRenderer();
            
            void render(Vbo& vbo, RenderContext& context);
        };
    }
}

#endif /* defined(__TrenchBroom__BoxGuideRenderer__) */
