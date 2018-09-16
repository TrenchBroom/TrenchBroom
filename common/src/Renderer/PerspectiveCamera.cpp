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

#include "PerspectiveCamera.h"

#include "Color.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Shaders.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexSpec.h"

#include <vecmath/forward.h>
#include <vecmath/vec.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/ray.h>
#include <vecmath/plane.h>
#include <vecmath/intersection.h>

#include <limits>

namespace TrenchBroom {
    namespace Renderer {
        PerspectiveCamera::PerspectiveCamera() :
        Camera(),
        m_fov(90.0) {}
        
        PerspectiveCamera::PerspectiveCamera(const float fov, const float nearPlane, const float farPlane, const Viewport& viewport, const vm::vec3f& position, const vm::vec3f& direction, const vm::vec3f& up)
        : Camera(nearPlane, farPlane, viewport, position, direction, up),
        m_fov(fov) {
            assert(m_fov > 0.0);
        }
        
        float PerspectiveCamera::fov() const {
            return m_fov;
        }
        
        void PerspectiveCamera::setFov(const float fov) {
            assert(fov > 0.0f);
            if (fov == m_fov)
                return;
            m_fov = fov;
            m_valid = false;
            cameraDidChangeNotifier(this);
        }
        
        vm::ray3f PerspectiveCamera::doGetPickRay(const vm::vec3f& point) const {
            const vm::vec3f direction = normalize(point - position());
            return vm::ray3f(position(), direction);
        }
        
        Camera::ProjectionType PerspectiveCamera::doGetProjectionType() const {
            return Projection_Perspective;
        }

        void PerspectiveCamera::doValidateMatrices(vm::mat4x4f& projectionMatrix, vm::mat4x4f& viewMatrix) const {
            const Viewport& viewport = unzoomedViewport();
            projectionMatrix = vm::perspectiveMatrix(fov(), nearPlane(), farPlane(), viewport.width, viewport.height);
            viewMatrix = vm::viewMatrix(direction(), up()) * translationMatrix(-position());
        }
        
        void PerspectiveCamera::doComputeFrustumPlanes(vm::plane3f& topPlane, vm::plane3f& rightPlane, vm::plane3f& bottomPlane, vm::plane3f& leftPlane) const {
            const vm::vec2f frustum = getFrustum();
            const vm::vec3f center = position() + direction() * nearPlane();
            
            vm::vec3f d = center + up() * frustum.y() - position();
            topPlane = vm::plane3f(position(), normalize(cross(right(), d)));
            
            d = center + right() * frustum.x() - position();
            rightPlane = vm::plane3f(position(), normalize(cross(d, up())));
            
            d = center - up() * frustum.y() - position();
            bottomPlane = vm::plane3f(position(), normalize(cross(d, right())));
            
            d = center - right() * frustum.x() - position();
            leftPlane = vm::plane3f(position(), normalize(cross(up(), d)));
        }
        
        void PerspectiveCamera::doRenderFrustum(RenderContext& renderContext, Vbo& vbo, const float size, const Color& color) const {
            typedef VertexSpecs::P3C4::Vertex Vertex;
            Vertex::List triangleVertices(6);
            Vertex::List lineVertices(8 * 2);
            
            vm::vec3f verts[4];
            getFrustumVertices(size, verts);
            
            triangleVertices[0] = Vertex(position(), Color(color, 0.7f));
            for (size_t i = 0; i < 4; ++i) {
                triangleVertices[i + 1] = Vertex(verts[i], Color(color, 0.2f));
            }
            triangleVertices[5] = Vertex(verts[0], Color(color, 0.2f));
            
            for (size_t i = 0; i < 4; ++i) {
                lineVertices[2 * i + 0] = Vertex(position(), color);
                lineVertices[2 * i + 1] = Vertex(verts[i], color);
            }
            
            for (size_t i = 0; i < 4; ++i) {
                lineVertices[8 + 2 * i + 0] = Vertex(verts[i], color);
                lineVertices[8 + 2 * i + 1] = Vertex(verts[vm::succ(i, 4)], color);
            }
            
            auto triangleArray = VertexArray::ref(triangleVertices);
            auto lineArray = VertexArray::ref(lineVertices);
            
            ActivateVbo activate(vbo);
            triangleArray.prepare(vbo);
            lineArray.prepare(vbo);
            
            ActiveShader shader(renderContext.shaderManager(), Shaders::VaryingPCShader);
            triangleArray.render(GL_TRIANGLE_FAN);
            lineArray.render(GL_LINES);
        }
        
        float PerspectiveCamera::doPickFrustum(const float size, const vm::ray3f& ray) const {
            vm::vec3f verts[4];
            getFrustumVertices(size, verts);
            
            auto minDistance = std::numeric_limits<float>::max();
            for (size_t i = 0; i < 4; ++i) {
                const auto distance = vm::intersect(ray, position(), verts[i], verts[vm::succ(i, 4)]);
                if (!vm::isnan(distance)) {
                    minDistance = vm::min(distance, minDistance);
                }
            }
            return minDistance;
        }

        void PerspectiveCamera::getFrustumVertices(const float size, vm::vec3f (&verts)[4]) const {
            const auto frustum = getFrustum();
            
            verts[0] = position() + (direction() * nearPlane() + frustum.y() * up() - frustum.x() * right()) / nearPlane() * size; // top left
            verts[1] = position() + (direction() * nearPlane() + frustum.y() * up() + frustum.x() * right()) / nearPlane() * size; // top right
            verts[2] = position() + (direction() * nearPlane() - frustum.y() * up() + frustum.x() * right()) / nearPlane() * size; // bottom right
            verts[3] = position() + (direction() * nearPlane() - frustum.y() * up() - frustum.x() * right()) / nearPlane() * size; // bottom left
        }

        vm::vec2f PerspectiveCamera::getFrustum() const {
            const auto& viewport = unzoomedViewport();
            const auto v = std::tan(vm::toRadians(fov()) / 2.0f) * 0.75f * nearPlane();
            const auto h = v * static_cast<float>(viewport.width) / static_cast<float>(viewport.height);
            return vm::vec2f(h, v);
        }

        float PerspectiveCamera::doGetPerspectiveScalingFactor(const vm::vec3f& position) const {
            const auto perpDist = perpendicularDistanceTo(position);
            return perpDist / viewportFrustumDistance();
        }

        float PerspectiveCamera::viewportFrustumDistance() const {
            const auto height = static_cast<float>(unzoomedViewport().height);
            return (height / 2.0f) / std::tan(vm::toRadians(m_fov) / 2.0f);
        }
    }
}
