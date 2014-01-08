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
#include "Renderer/Vbo.h"
#include "Renderer/Vertex.h"
#include "Renderer/VertexSpec.h"

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
        
        class MoveIndicatorRenderer {
        public:
            typedef enum {
                HorizontalXY,
                HorizontalX,
                HorizontalY,
                Vertical
            } Direction;
        private:
            typedef VertexSpecs::P2::Vertex Vertex;
            static const float HalfWidth;
            static const float Height;
            Vbo m_vbo;
        public:
            MoveIndicatorRenderer();
            void render(RenderContext& renderContext, const Vec3f& position, const Direction direction);
        private:
            void makeSolidXArrows(const float offset, Vertex::List& vertices) const;
            void makeSolidYArrows(const float offset, Vertex::List& vertices) const;
            void makeOutlineXArrows(const float offset, Vertex::List& vertices) const;
            void makeOutlineYArrows(const float offset, Vertex::List& vertices) const;
        };
    }
}

#endif /* defined(__TrenchBroom__MoveIndicatorRenderer__) */
