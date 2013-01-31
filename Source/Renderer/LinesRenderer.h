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

#ifndef __TrenchBroom__LinesRenderer__
#define __TrenchBroom__LinesRenderer__

#include "Utility/Color.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
        class Vbo;
        class VertexArray;
        
        class LinesRenderer {
        private:
            Color m_color;
            Color m_occludedColor;
            Vec3f::List m_vertices;
            VertexArray* m_vertexArray;
            
            bool m_valid;
        public:
            LinesRenderer();
            ~LinesRenderer();
            
            inline void setColor(const Color& color, const Color& occludedColor) {
                m_color = color;
                m_occludedColor = occludedColor;
            }
            
            inline void add(const Vec3f& start, const Vec3f& end) {
                m_vertices.push_back(start);
                m_vertices.push_back(end);
                m_valid = false;
            }
            
            void clear() {
                m_vertices.clear();
                m_valid = false;
            }
            
            void render(Vbo& vbo, RenderContext& context);
        };
    }
}

#endif /* defined(__TrenchBroom__LinesRenderer__) */
