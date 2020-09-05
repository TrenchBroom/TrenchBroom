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

#ifndef TrenchBroom_GridRenderer
#define TrenchBroom_GridRenderer

#include "FloatType.h"
#include "Renderer/Renderable.h"
#include "Renderer/VertexArray.h"
#include "Renderer/GLVertexType.h"

#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class OrthographicCamera;
        class RenderContext;
        class VboManager;

        class GridRenderer : public DirectRenderable {
        private:
            using Vertex = GLVertexTypes::P3::Vertex;
            VertexArray m_vertexArray;
        public:
            GridRenderer(const OrthographicCamera& camera, const vm::bbox3& worldBounds);
        private:
            static std::vector<Vertex> vertices(const OrthographicCamera& camera, const vm::bbox3& worldBounds);

            void doPrepareVertices(VboManager& vboManager) override;
            void doRender(RenderContext& renderContext) override;
        };
    }
}

#endif /* defined(TrenchBroom_GridRenderer) */
