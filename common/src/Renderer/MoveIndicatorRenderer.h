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

#ifndef __TrenchBroom__MoveIndicatorRenderer__
#define __TrenchBroom__MoveIndicatorRenderer__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Renderer/Renderable.h"
#include "Renderer/Vertex.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexSpec.h"

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
        class Vbo;
        
        class MoveIndicatorRenderer : public Renderable {
        public:
            typedef enum {
                Direction_XY,
                Direction_X,
                Direction_Y,
                Direction_Z
            } Direction;
        private:
            typedef VertexSpecs::P2::Vertex Vertex;
            static const float HalfWidth;
            static const float Height;
            
            Vec3f m_position;
            Direction m_direction;
            VertexArray m_triangleArray;
            VertexArray m_outlineArray;
        public:
            MoveIndicatorRenderer(const Vec3f& position, Direction direction);
        private:
            void doPrepare(Vbo& vbo);
            void doRender(RenderContext& renderContext);
        private:
            void makeSolidXArrows(float offset, Vertex::List& vertices) const;
            void makeSolidYArrows(float offset, Vertex::List& vertices) const;
            void makeOutlineXArrows(float offset, Vertex::List& vertices) const;
            void makeOutlineYArrows(float offset, Vertex::List& vertices) const;
        };
    }
}

#endif /* defined(__TrenchBroom__MoveIndicatorRenderer__) */
