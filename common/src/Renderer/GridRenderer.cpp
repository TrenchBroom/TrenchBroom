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

#include "GridRenderer.h"

#include "TrenchBroom.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/OrthographicCamera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Shaders.h"

#include <vecmath/vec.h>

namespace TrenchBroom {
    namespace Renderer {
        GridRenderer::GridRenderer(const OrthographicCamera& camera, const vm::bbox3& worldBounds) :
        m_vertexArray(VertexArray::copy(vertices(camera, worldBounds))) {}

        GridRenderer::Vertex::List GridRenderer::vertices(const OrthographicCamera& camera, const vm::bbox3& worldBounds) {
            const Camera::Viewport& viewport = camera.zoomedViewport();
            const float w = float(viewport.width) / 2.0f;
            const float h = float(viewport.height) / 2.0f;

            const vm::vec3f& p = camera.position();
            switch (firstComponent(camera.direction())) {
                case vm::axis::x:
                    return Vertex::List({
                        Vertex(vm::vec3f(float(worldBounds.min.x()), p.y() - w, p.z() - h)),
                        Vertex(vm::vec3f(float(worldBounds.min.x()), p.y() - w, p.z() + h)),
                        Vertex(vm::vec3f(float(worldBounds.min.x()), p.y() + w, p.z() + h)),
                        Vertex(vm::vec3f(float(worldBounds.min.x()), p.y() + w, p.z() - h))
                    });
                case vm::axis::y:
                    return Vertex::List({
                        Vertex(vm::vec3f(p.x() - w, float(worldBounds.max.y()), p.z() - h)),
                        Vertex(vm::vec3f(p.x() - w, float(worldBounds.max.y()), p.z() + h)),
                        Vertex(vm::vec3f(p.x() + w, float(worldBounds.max.y()), p.z() + h)),
                        Vertex(vm::vec3f(p.x() + w, float(worldBounds.max.y()), p.z() - h))
                    });
                case vm::axis::z:
                    return Vertex::List({
                        Vertex(vm::vec3f(p.x() - w, p.y() - h, float(worldBounds.min.z()))),
                        Vertex(vm::vec3f(p.x() - w, p.y() + h, float(worldBounds.min.z()))),
                        Vertex(vm::vec3f(p.x() + w, p.y() + h, float(worldBounds.min.z()))),
                        Vertex(vm::vec3f(p.x() + w, p.y() - h, float(worldBounds.min.z())))
                    });
                default:
                    // Should not happen.
                    return Vertex::List();
            }
        }

        void GridRenderer::doPrepareVertices(Vbo& vertexVbo) {
            m_vertexArray.prepare(vertexVbo);
        }

        void GridRenderer::doRender(RenderContext& renderContext) {
            if (renderContext.showGrid()) {
                const Camera& camera = renderContext.camera();

                ActiveShader shader(renderContext.shaderManager(), Shaders::Grid2DShader);
                shader.set("Normal", -camera.direction());
                shader.set("RenderGrid", renderContext.showGrid());
                shader.set("GridSize", static_cast<float>(renderContext.gridSize()));
                shader.set("GridAlpha", pref(Preferences::GridAlpha));
                shader.set("GridColor", pref(Preferences::GridColor2D));
                shader.set("CameraZoom", camera.zoom());

                m_vertexArray.render(GL_QUADS);
            }
        }
    }
}
