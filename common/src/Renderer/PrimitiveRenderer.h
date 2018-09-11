/*
 Copyright (C) 2010-2017 Kristian Duske
 
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
#include "Color.h"
#include "Renderer/IndexRangeMapBuilder.h"
#include "Renderer/IndexRangeRenderer.h"
#include "Renderer/Renderable.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexSpec.h"

#include <vecmath/vec.h>
#include <vecmath/utils.h>

#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class ActiveShader;
        class RenderContext;
        class Vbo;
        
        class PrimitiveRenderer : public DirectRenderable {
        public:
            typedef enum {
                OP_Hide,
                OP_Show,
                OP_Transparent
            } OcclusionPolicy;
            
            typedef enum {
                CP_CullBackfaces,
                CP_ShowBackfaces
            } CullingPolicy;
        private:
            typedef VertexSpecs::P3::Vertex Vertex;

            class LineRenderAttributes {
            private:
                Color m_color;
                float m_lineWidth;
                OcclusionPolicy m_occlusionPolicy;
                CullingPolicy m_cullingPolicy;
            public:
                LineRenderAttributes(const Color& color, float lineWidth, OcclusionPolicy occlusionPolicy);
                bool operator<(const LineRenderAttributes& other) const;
                
                void render(IndexRangeRenderer& renderer, ActiveShader& shader) const;
            };
            
            typedef std::map<LineRenderAttributes, IndexRangeMapBuilder<Vertex::Spec> > LineMeshMap;
            LineMeshMap m_lineMeshes;
            
            typedef std::map<LineRenderAttributes, IndexRangeRenderer> LineMeshRendererMap;
            LineMeshRendererMap m_lineMeshRenderers;
            
            class TriangleRenderAttributes {
            private:
                Color m_color;
                OcclusionPolicy m_occlusionPolicy;
                CullingPolicy m_cullingPolicy;
            public:
                TriangleRenderAttributes(const Color& color, OcclusionPolicy occlusionPolicy, CullingPolicy cullingPolicy);
                bool operator<(const TriangleRenderAttributes& other) const;
                
                void render(IndexRangeRenderer& renderer, ActiveShader& shader) const;
            };
            
            typedef std::map<TriangleRenderAttributes, IndexRangeMapBuilder<Vertex::Spec> > TriangleMeshMap;
            TriangleMeshMap m_triangleMeshes;
            
            typedef std::map<TriangleRenderAttributes, IndexRangeRenderer> TriangleMeshRendererMap;
            TriangleMeshRendererMap m_triangleMeshRenderers;
        public:
            void renderLine(const Color& color, float lineWidth, OcclusionPolicy occlusionPolicy, const vm::vec3f& start, const vm::vec3f& end);
            void renderLines(const Color& color, float lineWidth, OcclusionPolicy occlusionPolicy, const vm::vec3f::List& positions);
            void renderLineStrip(const Color& color, float lineWidth, OcclusionPolicy occlusionPolicy, const vm::vec3f::List& positions);
            
            void renderCoordinateSystemXY(const Color& x, const Color& y, float lineWidth, OcclusionPolicy occlusionPolicy, const vm::bbox3f& bounds);
            void renderCoordinateSystemXZ(const Color& x, const Color& z, float lineWidth, OcclusionPolicy occlusionPolicy, const vm::bbox3f& bounds);
            void renderCoordinateSystemYZ(const Color& y, const Color& z, float lineWidth, OcclusionPolicy occlusionPolicy, const vm::bbox3f& bounds);
            void renderCoordinateSystem3D(const Color& x, const Color& y, const Color& z, float lineWidth, OcclusionPolicy occlusionPolicy, const vm::bbox3f& bounds);
            
            void renderPolygon(const Color& color, float lineWidth, OcclusionPolicy occlusionPolicy, const vm::vec3f::List& positions);
            void renderFilledPolygon(const Color& color, OcclusionPolicy occlusionPolicy, CullingPolicy cullingPolicy, const vm::vec3f::List& positions);
            
            void renderCylinder(const Color& color, float radius, size_t segments, OcclusionPolicy occlusionPolicy, CullingPolicy cullingPolicy, const vm::vec3f& start, const vm::vec3f& end);
        private:
            void doPrepareVertices(Vbo& vertexVbo) override;
            void prepareLines(Vbo& vertexVbo);
            void prepareTriangles(Vbo& vertexVbo);

            void doRender(RenderContext& renderContext) override;
            void renderLines(RenderContext& renderContext);
            void renderTriangles(RenderContext& renderContext);
        };
    }
}

#endif /* defined(TrenchBroom_PrimitiveRenderer) */
