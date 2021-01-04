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

#pragma once

#include "Color.h"
#include "Renderer/Renderable.h"
#include "Renderer/GLVertexType.h"

#include <map>
#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class ActiveShader;
        template <typename VertexSpec> class IndexRangeMapBuilder;
        class IndexRangeRenderer;

        enum class PrimitiveRendererOcclusionPolicy {
            Hide,
            Show,
            Transparent
        };

        enum class PrimitiveRendererCullingPolicy {
            CullBackfaces,
            ShowBackfaces
        };

        class PrimitiveRenderer : public DirectRenderable {
        public:
        private:
            using Vertex = GLVertexTypes::P3::Vertex;

            class LineRenderAttributes {
            private:
                Color m_color;
                float m_lineWidth;
                PrimitiveRendererOcclusionPolicy m_occlusionPolicy;
            public:
                LineRenderAttributes(const Color& color, float lineWidth, PrimitiveRendererOcclusionPolicy occlusionPolicy);
                bool operator<(const LineRenderAttributes& other) const;

                void render(IndexRangeRenderer& renderer, ActiveShader& shader) const;
            };

            using LineMeshMap = std::map<LineRenderAttributes, IndexRangeMapBuilder<Vertex::Type>>;
            LineMeshMap m_lineMeshes;

            using LineMeshRendererMap = std::map<LineRenderAttributes, IndexRangeRenderer>;
            LineMeshRendererMap m_lineMeshRenderers;

            class TriangleRenderAttributes {
            private:
                Color m_color;
                PrimitiveRendererOcclusionPolicy m_occlusionPolicy;
                PrimitiveRendererCullingPolicy m_cullingPolicy;
            public:
                TriangleRenderAttributes(const Color& color, PrimitiveRendererOcclusionPolicy occlusionPolicy, PrimitiveRendererCullingPolicy cullingPolicy);
                bool operator<(const TriangleRenderAttributes& other) const;

                void render(IndexRangeRenderer& renderer, ActiveShader& shader) const;
            };

            using TriangleMeshMap = std::map<TriangleRenderAttributes, IndexRangeMapBuilder<Vertex::Type>>;
            TriangleMeshMap m_triangleMeshes;

            using TriangleMeshRendererMap = std::map<TriangleRenderAttributes, IndexRangeRenderer>;
            TriangleMeshRendererMap m_triangleMeshRenderers;
        public:
            void renderLine(const Color& color, float lineWidth, PrimitiveRendererOcclusionPolicy occlusionPolicy, const vm::vec3f& start, const vm::vec3f& end);
            void renderLines(const Color& color, float lineWidth, PrimitiveRendererOcclusionPolicy occlusionPolicy, const std::vector<vm::vec3f>& positions);
            void renderLineStrip(const Color& color, float lineWidth, PrimitiveRendererOcclusionPolicy occlusionPolicy, const std::vector<vm::vec3f>& positions);

            void renderCoordinateSystemXY(const Color& x, const Color& y, float lineWidth, PrimitiveRendererOcclusionPolicy occlusionPolicy, const vm::bbox3f& bounds);
            void renderCoordinateSystemXZ(const Color& x, const Color& z, float lineWidth, PrimitiveRendererOcclusionPolicy occlusionPolicy, const vm::bbox3f& bounds);
            void renderCoordinateSystemYZ(const Color& y, const Color& z, float lineWidth, PrimitiveRendererOcclusionPolicy occlusionPolicy, const vm::bbox3f& bounds);
            void renderCoordinateSystem3D(const Color& x, const Color& y, const Color& z, float lineWidth, PrimitiveRendererOcclusionPolicy occlusionPolicy, const vm::bbox3f& bounds);

            void renderPolygon(const Color& color, float lineWidth, PrimitiveRendererOcclusionPolicy occlusionPolicy, const std::vector<vm::vec3f>& positions);
            void renderFilledPolygon(const Color& color, PrimitiveRendererOcclusionPolicy occlusionPolicy, PrimitiveRendererCullingPolicy cullingPolicy, const std::vector<vm::vec3f>& positions);

            void renderCylinder(const Color& color, float radius, size_t segments, PrimitiveRendererOcclusionPolicy occlusionPolicy, PrimitiveRendererCullingPolicy cullingPolicy, const vm::vec3f& start, const vm::vec3f& end);
        private:
            void doPrepareVertices(VboManager& vboManager) override;
            void prepareLines(VboManager& vboManager);
            void prepareTriangles(VboManager& vboManager);

            void doRender(RenderContext& renderContext) override;
            void renderLines(RenderContext& renderContext);
            void renderTriangles(RenderContext& renderContext);
        };
    }
}

