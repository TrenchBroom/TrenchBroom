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

#ifndef TrenchBroom_PrimitiveRenderer
#define TrenchBroom_PrimitiveRenderer

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Color.h"
#include "Renderer/IndexRangeMapBuilder.h"
#include "Renderer/IndexRangeRenderer.h"
#include "Renderer/Renderable.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexSpec.h"

#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
        class Vbo;
        
        class PrimitiveRenderer : public DirectRenderable {
        private:
            typedef VertexSpecs::P3C4::Vertex Vertex;
            typedef std::map<float, IndexRangeMapBuilder<Vertex::Spec> > LineMeshMap;
            LineMeshMap m_lineMeshes;
            IndexRangeMapBuilder<Vertex::Spec> m_triangleMesh;
            
            typedef std::map<float, IndexRangeRenderer> LineMeshRendererMap;
            LineMeshRendererMap m_lineMeshRenderers;
            
            IndexRangeRenderer m_triangleMeshRenderer;
        public:
            void renderLine(const Color& color, float lineWidth, const Vec3f& start, const Vec3f& end);
            void renderLines(const Color& color, float lineWidth, const Vec3f::List& positions);
            void renderCoordinateSystemXY(const Color& x, const Color& y, float lineWidth, const BBox3f& bounds);
            void renderCoordinateSystemXZ(const Color& x, const Color& z, float lineWidth, const BBox3f& bounds);
            void renderCoordinateSystemYZ(const Color& y, const Color& z, float lineWidth, const BBox3f& bounds);
            void renderCoordinateSystem3D(const Color& x, const Color& y, const Color& z, float lineWidth, const BBox3f& bounds);
            
            void renderPolygon(const Color& color, float lineWidth, const Vec3f::List& positions);
            void renderFilledPolygon(const Color& color, const Vec3f::List& positions);
            
            void renderCircle(const Color& color, float lineWidth, const Vec3f& position, Math::Axis::Type normal, size_t segments, float radius, const Vec3f& startAxis, const Vec3f& endAxis);
            void renderCircle(const Color& color, float lineWidth, const Vec3f& position, Math::Axis::Type normal, size_t segments, float radius, float startAngle = 0.0f, float angleLength = Math::Cf::twoPi());
            
            void renderFilledCircle(const Color& color, const Vec3f& position, Math::Axis::Type normal, size_t segments, float radius, const Vec3f& startAxis, const Vec3f& endAxis);
            void renderFilledCircle(const Color& color, const Vec3f& position, Math::Axis::Type normal, size_t segments, float radius, float startAngle = 0.0f, float angleLength = Math::Cf::twoPi());
        private:
            void doPrepareVertices(Vbo& vertexVbo);
            void doRender(RenderContext& renderContext);
            void renderLines(RenderContext& renderContext);
            void renderTriangles(RenderContext& renderContext);
        };
    }
}

#endif /* defined(TrenchBroom_PrimitiveRenderer) */
