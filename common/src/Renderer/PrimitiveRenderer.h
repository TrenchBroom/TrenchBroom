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

#ifndef __TrenchBroom__PrimitiveRenderer__
#define __TrenchBroom__PrimitiveRenderer__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Color.h"
#include "Renderer/LineMesh.h"
#include "Renderer/LineMeshRenderer.h"
#include "Renderer/TriangleMesh.h"
#include "Renderer/TriangleMeshRenderer.h"
#include "Renderer/Renderable.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexSpec.h"

#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
        class Vbo;
        
        class PrimitiveRenderer : public Renderable {
        private:
            typedef VertexSpecs::P3C4::Vertex Vertex;

            LineMesh<Vertex::Spec> m_lineMesh;
            TriangleMesh<Vertex::Spec> m_triangleMesh;

            LineMeshRenderer m_lineRenderer;
            SimpleTriangleMeshRenderer m_triangleRenderer;
        public:
            void renderLine(const Color& color, const Vec3f& start, const Vec3f& end);
            void renderLines(const Color& color, const Vec3f::List& positions);
            void renderCoordinateSystem(const BBox3f& bounds, const Color& x, const Color& y, const Color& z);
            
            void renderCircle(const Color& color, const Vec3f& position, Math::Axis::Type normal, size_t segments, float radius, const Vec3f& startAxis, const Vec3f& endAxis);
            void renderCircle(const Color& color, const Vec3f& position, Math::Axis::Type normal, size_t segments, float radius, float startAngle = 0.0f, float angleLength = Math::Cf::twoPi());
            
            void renderFilledCircle(const Color& color, const Vec3f& position, Math::Axis::Type normal, size_t segments, float radius, const Vec3f& startAxis, const Vec3f& endAxis);
            void renderFilledCircle(const Color& color, const Vec3f& position, Math::Axis::Type normal, size_t segments, float radius, float startAngle = 0.0f, float angleLength = Math::Cf::twoPi());
        private:
            void doPrepare(Vbo& vbo);
            void doRender(RenderContext& renderContext);
        };
    }
}

#endif /* defined(__TrenchBroom__PrimitiveRenderer__) */
