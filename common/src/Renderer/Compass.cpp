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

#include "Compass.h"

#include "CollectionUtils.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Shader.h"
#include "Renderer/Transformation.h"
#include "Renderer/Vertex.h"
#include "Renderer/VertexSpec.h"
#include "View/MovementRestriction.h"

#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        const size_t Compass::m_segments = 32;
        const float Compass::m_shaftLength = 28.0f;
        const float Compass::m_shaftRadius = 1.2f;
        const float Compass::m_headLength = 7.0f;
        const float Compass::m_headRadius = 3.5f;

        Compass::Compass() {
            makeArrows();
            makeBackground();
        }

        void Compass::prepare(Vbo& vbo) {
            m_strip.prepare(vbo);
            m_set.prepare(vbo);
            m_fans.prepare(vbo);
            m_backgroundOutline.prepare(vbo);
            m_background.prepare(vbo);
        }
        
        void Compass::render(RenderContext& renderContext, const View::MovementRestriction& restriction) {
            glClear(GL_DEPTH_BUFFER_BIT);
            
            const int viewWidth = renderContext.camera().viewport().width;
            const int viewHeight = renderContext.camera().viewport().height;
            
            const Mat4x4f projection = orthoMatrix(0.0f, 1000.0f, -viewWidth / 2.0f, viewHeight / 2.0f, viewWidth / 2.0f, -viewHeight / 2.0f);
            const Mat4x4f view = viewMatrix(Vec3f::PosY, Vec3f::PosZ) * translationMatrix(500.0f * Vec3f::PosY);
            const ReplaceTransformation ortho(renderContext.transformation(), projection, view);
            
            const Mat4x4f compassTransformation = translationMatrix(Vec3f(-viewWidth / 2.0f + 55.0f, 0.0f, -viewHeight / 2.0f + 55.0f)) * scalingMatrix<4>(2.0f);
            const MultiplyModelMatrix compass(renderContext.transformation(), compassTransformation);
            
            PreferenceManager& prefs = PreferenceManager::instance();
            const Mat4x4f cameraTransformation = cameraRotationMatrix(renderContext.camera());
            
            renderBackground(renderContext);
            glClear(GL_DEPTH_BUFFER_BIT);

            if (restriction.isRestricted(Math::Axis::AX)) {
                renderSolidAxis(  renderContext, cameraTransformation, prefs.get(Preferences::ZAxisColor));
                renderSolidAxis(  renderContext, cameraTransformation * Mat4x4f::Rot90YCCW, prefs.get(Preferences::XAxisColor));
                renderAxisOutline(renderContext, cameraTransformation * Mat4x4f::Rot90XCW, prefs.get(Preferences::CompassAxisOutlineColor));
                renderSolidAxis(  renderContext, cameraTransformation * Mat4x4f::Rot90XCW, prefs.get(Preferences::YAxisColor));
            } else if (restriction.isRestricted(Math::Axis::AY)) {
                renderSolidAxis(  renderContext, cameraTransformation, prefs.get(Preferences::ZAxisColor));
                renderSolidAxis(  renderContext, cameraTransformation * Mat4x4f::Rot90XCW, prefs.get(Preferences::YAxisColor));
                renderAxisOutline(renderContext, cameraTransformation * Mat4x4f::Rot90YCCW, prefs.get(Preferences::CompassAxisOutlineColor));
                renderSolidAxis(  renderContext, cameraTransformation * Mat4x4f::Rot90YCCW, prefs.get(Preferences::XAxisColor));
            } else if (restriction.isRestricted(Math::Axis::AZ)) {
                renderSolidAxis(  renderContext, cameraTransformation * Mat4x4f::Rot90YCCW, prefs.get(Preferences::XAxisColor));
                renderSolidAxis(  renderContext, cameraTransformation * Mat4x4f::Rot90XCW, prefs.get(Preferences::YAxisColor));
                renderAxisOutline(renderContext, cameraTransformation, prefs.get(Preferences::CompassAxisOutlineColor));
                renderSolidAxis(  renderContext, cameraTransformation, prefs.get(Preferences::ZAxisColor));
            } else {
                renderSolidAxis(renderContext, cameraTransformation, prefs.get(Preferences::ZAxisColor));
                renderSolidAxis(renderContext, cameraTransformation * Mat4x4f::Rot90YCCW, prefs.get(Preferences::XAxisColor));
                renderSolidAxis(renderContext, cameraTransformation * Mat4x4f::Rot90XCW, prefs.get(Preferences::YAxisColor));
            }
        }

        void Compass::makeArrows() {
            const Vec3f shaftOffset(0.0f, 0.0f, -(m_shaftLength + m_headLength) / 2.0f + 2.0f);
            const Vec3f headOffset = Vec3f(0.0f, 0.0f, m_shaftLength) + shaftOffset;
            
            VertsAndNormals shaft = cylinder3D(m_shaftRadius, m_shaftLength, m_segments);
            for (size_t i = 0; i < shaft.vertices.size(); ++i)
                shaft.vertices[i] += shaftOffset;
            
            VertsAndNormals head = cone3D(m_headRadius, m_headLength, m_segments);
            for (size_t i = 0; i < head.vertices.size(); ++i)
                head.vertices[i] += headOffset;
            
            VertsAndNormals shaftCap = circle3D(m_shaftRadius, m_segments);
            for (size_t i = 0; i < shaftCap.vertices.size(); ++i) {
                shaftCap.vertices[i] = Mat4x4f::Rot180X * shaftCap.vertices[i] + shaftOffset;
                shaftCap.normals[i] = Mat4x4f::Rot180X * shaftCap.normals[i];
            }
            
            VertsAndNormals headCap = circle3D(m_headRadius, m_segments);
            for (size_t i = 0; i < headCap.vertices.size(); ++i) {
                headCap.vertices[i] = Mat4x4f::Rot180X * headCap.vertices[i] + headOffset;
                headCap.normals[i] = Mat4x4f::Rot180X * headCap.normals[i];
            }
            
            typedef VertexSpecs::P3N::Vertex Vertex;
            Vertex::List shaftVertices = Vertex::fromLists(shaft.vertices, shaft.normals, shaft.vertices.size());
            Vertex::List headVertices = Vertex::fromLists(head.vertices, head.normals, head.vertices.size());
            Vertex::List capVertices = VectorUtils::concatenate(Vertex::fromLists(shaftCap.vertices, shaftCap.normals, shaftCap.vertices.size()),
                                                                      Vertex::fromLists(headCap.vertices, headCap.normals, headCap.vertices.size()));
            VertexArray::IndexArray indices(2);
            indices[0] = 0;
            indices[1] = static_cast<GLint>(shaftCap.vertices.size());
            
            VertexArray::CountArray counts(2);
            counts[0] = static_cast<GLsizei>(shaftCap.vertices.size());
            counts[1] = static_cast<GLsizei>(headCap.vertices.size());
            
            m_strip = VertexArray::swap(GL_TRIANGLE_STRIP, shaftVertices);
            m_set = VertexArray::swap(GL_TRIANGLES, headVertices);
            m_fans = VertexArray::swap(GL_TRIANGLE_FAN, capVertices, indices, counts);
        }
        
        void Compass::makeBackground() {
            typedef VertexSpecs::P2::Vertex Vertex;
            Vec2f::List circ = circle2D((m_shaftLength + m_headLength) / 2.0f + 5.0f, 0.0f, Math::Cf::twoPi(), m_segments);
            Vertex::List verts = Vertex::fromLists(circ, circ.size());
            
            m_background = VertexArray::swap(GL_TRIANGLE_FAN, verts);
            m_backgroundOutline = VertexArray::swap(GL_LINE_LOOP, verts);
        }

        Mat4x4f Compass::cameraRotationMatrix(const Camera& camera) const {
            Mat4x4f rotation;
            rotation[0] = camera.right();
            rotation[1] = camera.direction();
            rotation[2] = camera.up();
            
            bool invertible = true;
            invertMatrix(rotation, invertible);
            assert(invertible);
            return rotation;
        }

        void Compass::renderBackground(RenderContext& renderContext) {
            PreferenceManager& prefs = PreferenceManager::instance();

            const MultiplyModelMatrix rotate(renderContext.transformation(), Mat4x4f::Rot90XCCW);
            ActiveShader shader(renderContext.shaderManager(), Shaders::CompassBackgroundShader);
            shader.set("Color", prefs.get(Preferences::CompassBackgroundColor));
            m_background.render();
            shader.set("Color", prefs.get(Preferences::CompassBackgroundOutlineColor));
            m_backgroundOutline.render();
        }

        void Compass::renderSolidAxis(RenderContext& renderContext, const Mat4x4f& transformation, const Color& color) {
            ActiveShader shader(renderContext.shaderManager(), Shaders::CompassShader);
            shader.set("CameraPosition", Vec3f(0.0f, 500.0f, 0.0f));
            shader.set("LightDirection", Vec3f(0.0f, 0.5f, 1.0f).normalized());
            shader.set("LightDiffuse", Color(1.0f, 1.0f, 1.0f, 1.0f));
            shader.set("LightSpecular", Color(0.3f, 0.3f, 0.3f, 1.0f));
            shader.set("GlobalAmbient", Color(0.2f, 0.2f, 0.2f, 1.0f));
            shader.set("MaterialShininess", 32.0f);
            
            shader.set("MaterialDiffuse", color);
            shader.set("MaterialAmbient", color);
            shader.set("MaterialSpecular", color);
            
            renderAxis(renderContext, transformation);
        }
        
        void Compass::renderAxisOutline(RenderContext& renderContext, const Mat4x4f& transformation, const Color& color) {
            glDepthMask(GL_FALSE);
            glLineWidth(3.0f);
            glPolygonMode(GL_FRONT, GL_LINE);
            
            ActiveShader shader(renderContext.shaderManager(), Shaders::CompassOutlineShader);
            shader.set("Color", color);
            renderAxis(renderContext, transformation);
            
            glDepthMask(GL_TRUE);
            glLineWidth(1.0f);
            glPolygonMode(GL_FRONT, GL_FILL);
        }

        void Compass::renderAxis(RenderContext& renderContext, const Mat4x4f& transformation) {
            const MultiplyModelMatrix apply(renderContext.transformation(), transformation);
            
            m_strip.render();
            m_set.render();
            m_fans.render();
        }
    }
}
