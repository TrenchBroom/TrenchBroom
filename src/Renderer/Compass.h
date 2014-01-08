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

#ifndef __TrenchBroom__Compass__
#define __TrenchBroom__Compass__

#include "Renderer/VertexArray.h"

#include "Color.h"
#include "VecMath.h"

namespace TrenchBroom {
    namespace View {
        class MovementRestriction;
    }
    
    namespace Renderer {
        class Camera;
        class RenderContext;
        class Vbo;
        
        class Compass {
        private:
            static const size_t m_segments;
            static const float m_shaftLength;
            static const float m_shaftRadius;
            static const float m_headLength;
            static const float m_headRadius;

            VertexArray m_strip;
            VertexArray m_set;
            VertexArray m_fans;
            
            VertexArray m_backgroundOutline;
            VertexArray m_background;
        public:
            Compass();
            void prepare(Vbo& vbo);
            void render(RenderContext& renderContext, const View::MovementRestriction& restriction);
        private:
            void makeArrows();
            void makeBackground();
            
            Mat4x4f cameraRotationMatrix(const Camera& camera) const;
            void renderBackground(RenderContext& renderContext);
            void renderSolidAxis(RenderContext& renderContext, const Mat4x4f& transformation, const Color& color);
            void renderAxisOutline(RenderContext& renderContext, const Mat4x4f& transformation, const Color& color);
            void renderAxis(RenderContext& renderContext, const Mat4x4f& transformation);
        };
    }
}

#endif /* defined(__TrenchBroom__Compass__) */
