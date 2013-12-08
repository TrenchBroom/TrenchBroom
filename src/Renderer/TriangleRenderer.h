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

#ifndef __TrenchBroom__TriangleRenderer__
#define __TrenchBroom__TriangleRenderer__

#include "Color.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexSpec.h"
#include "Renderer/VertexArray.h"

#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
        
        class TriangleRenderer {
        private:
            Vbo::Ptr m_vbo;
            VertexArray m_vertexArray;
            Color m_color;
            bool m_useColor;
            Color m_tintColor;
            bool m_applyTinting;
            bool m_prepared;
        public:
            TriangleRenderer();
            TriangleRenderer(const VertexArray& vertexArray);
            TriangleRenderer(const TriangleRenderer& other);
            TriangleRenderer& operator= (TriangleRenderer other);
            
            friend void swap(TriangleRenderer& left, TriangleRenderer& right);
            
            void setUseColor(bool useColor);
            void setColor(const Color& color);
            void setApplyTinting(bool applyTinting);
            void setTintColor(const Color& tintColor);
            
            void render(RenderContext& context);
        private:
            void prepare();
        };
    }
}

#endif /* defined(__TrenchBroom__TriangleRenderer__) */
