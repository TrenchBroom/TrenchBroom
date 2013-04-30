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

#ifndef __TrenchBroom__MovementIndicator__
#define __TrenchBroom__MovementIndicator__

#include "Utility/Color.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
        class ShaderProgram;
        class VertexArray;
        class Vbo;
        
        class MovementIndicator {
        public:
            typedef enum {
                Horizontal,
                HorizontalX,
                HorizontalY,
                Vertical
            } Direction;
        private:
            static const float Width2;
            static const float Height;
            
            Direction m_direction;
            Vec3f m_position;
            Color m_outlineColor;
            Color m_fillColor;
            VertexArray* m_outline;
            VertexArray* m_triangles;
            bool m_valid;
            
            void validate(Vbo& vbo);
            void buildXArrows(const float offset, Vec2f::List& triangles, Vec2f::List& outline) const;
            void buildYArrows(const float offset, Vec2f::List& triangles, Vec2f::List& outline) const;
            void renderArrow(const Mat4f& matrix, ShaderProgram& shader, RenderContext& context) const;
        public:
            MovementIndicator();
            ~MovementIndicator();

            inline void setDirection(Direction direction) {
                if (direction == m_direction)
                    return;
                m_direction = direction;
                m_valid = false;
            }
            
            inline void setColor(const Color& outlineColor, const Color& fillColor) {
                m_outlineColor = outlineColor;
                m_fillColor = fillColor;
            }
            
            inline void setPosition(const Vec3f& position) {
                m_position = position;
            }
            
            void render(Vbo& vbo, RenderContext& context);
        };
    }
}

#endif /* defined(__TrenchBroom__MovementIndicator__) */
