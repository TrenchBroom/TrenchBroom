/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "CompassRenderer.h"

#include "Controller/Input.h"
#include "Renderer/ApplyMatrix.h"
#include "Renderer/IndexedVertexArray.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/Shader/Shader.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"
#include "Utility/Preferences.h"

#include <algorithm>
#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        const float CompassRenderer::m_shaftLength = 28.0f;
        const float CompassRenderer::m_shaftRadius = 1.2f;
        const float CompassRenderer::m_headLength = 7.0f;
        const float CompassRenderer::m_headRadius = 3.5f;

        void CompassRenderer::validate(Vbo& vbo) {
            assert(!m_valid);
            
            Vec3f::List shaftVertices;
            Vec3f::List shaftNormals;
            Vec3f::List shaftCapVertices;
            Vec3f::List shaftCapNormals;
            Vec3f::List headVertices;
            Vec3f::List headNormals;
            Vec3f::List headCapVertices;
            Vec3f::List headCapNormals;
            
            const Vec3f offset(0.0f, 0.0f, m_shaftLength / 2.0f);
            
            cylinder(m_shaftLength, m_shaftRadius, m_segments, shaftVertices, shaftNormals);
            for (size_t i = 0; i < shaftVertices.size(); i++)
                shaftVertices[i] -= offset;
            
            cone(m_headLength, m_headRadius, m_segments, headVertices, headNormals);
            for (size_t i = 0; i < headVertices.size(); i++)
                headVertices[i] += offset;
            
            circle(m_headRadius, m_segments, headCapVertices, headCapNormals);
            headCapVertices = Mat4f::Rot180X * headCapVertices;
            headCapNormals = Mat4f::Rot180X * headCapNormals;
            for (size_t i = 0; i < headCapVertices.size(); i++)
                headCapVertices[i] += offset;
            
            circle(m_shaftRadius, m_segments, shaftCapVertices, shaftCapNormals);
            shaftCapVertices = Mat4f::Rot180X * shaftCapVertices;
            shaftCapNormals = Mat4f::Rot180X * shaftCapNormals;
            for (size_t i = 0; i < shaftCapVertices.size(); i++)
                shaftCapVertices[i] -= offset;
            
            m_strip = new VertexArray(vbo, GL_TRIANGLE_STRIP, shaftVertices.size(),
                                      Attribute::position3f(),
                                      Attribute::normal3f(),
                                      0);
            m_set = new VertexArray(vbo, GL_TRIANGLES, headVertices.size(),
                                    Attribute::position3f(),
                                    Attribute::normal3f(),
                                    0);
            m_fans = new IndexedVertexArray(vbo, GL_TRIANGLE_FAN, headCapVertices.size() + shaftCapVertices.size(),
                                            Attribute::position3f(),
                                            Attribute::normal3f(),
                                            0);
            
            SetVboState mapVbo(vbo, Vbo::VboMapped);
            m_strip->addAttributes(shaftVertices, shaftNormals);
            m_set->addAttributes(headVertices, headNormals);
            m_fans->addAttributes(headCapVertices, headCapNormals);
            m_fans->endPrimitive();
            m_fans->addAttributes(shaftCapVertices, shaftCapNormals);
            m_fans->endPrimitive();

            m_valid = true;
        }
        
        const Mat4f CompassRenderer::cameraRotationMatrix(const Camera& camera) const {
            Mat4f rotation;
            rotation[0] = camera.right();
            rotation[1] = camera.direction();
            rotation[2] = camera.up();
            
            bool invertible;
            invertMatrix(rotation, invertible);
            assert(invertible);
            return rotation;
        }

        void CompassRenderer::renderColoredAxis(RenderContext& context, const Mat4f& rotation, const Color& color) {
            ActivateShader compassShader(context.shaderManager(), Shaders::CompassShader);
            compassShader.setUniformVariable("CameraPosition", Vec3f(0.0f, 500.0f, 0.0f));
            compassShader.setUniformVariable("LightDirection", Vec3f(0.0f, 0.5f, 1.0f).normalized());
            compassShader.setUniformVariable("LightDiffuse", Color(1.0f, 1.0f, 1.0f, 1.0f));
            compassShader.setUniformVariable("LightSpecular", Color(0.3f, 0.3f, 0.3f, 1.0f));
            compassShader.setUniformVariable("GlobalAmbient", Color(0.2f, 0.2f, 0.2f, 1.0f));
            compassShader.setUniformVariable("MaterialShininess", 32.0f);

            compassShader.setUniformVariable("MaterialDiffuse", color);
            compassShader.setUniformVariable("MaterialAmbient", color);
            compassShader.setUniformVariable("MaterialSpecular", color);
            
            renderAxis(context, rotation);
        }
        
        void CompassRenderer::renderOutlinedAxis(RenderContext& context, const Mat4f& rotation, const Color& color) {
            glDepthMask(GL_FALSE);
            glLineWidth(3.0f);
            glPolygonMode(GL_FRONT, GL_LINE);
            
            ActivateShader compassOutlineShader(context.shaderManager(), Shaders::CompassOutlineShader);
            compassOutlineShader.setUniformVariable("Color", color);
            renderAxis(context, rotation);
            
            glDepthMask(GL_TRUE);
            glLineWidth(1.0f);
            glPolygonMode(GL_FRONT, GL_FILL);
        }

        void CompassRenderer::renderAxis(RenderContext& context, const Mat4f& rotation) {
            ApplyModelMatrix applyRotation(context.transformation(), rotation);
            m_strip->render();
            m_set->render();
            m_fans->render();
        }

        CompassRenderer::CompassRenderer() :
        m_strip(NULL),
        m_set(NULL),
        m_fans(NULL),
        m_valid(false) {}
        
        CompassRenderer::~CompassRenderer() {
            delete m_strip;
            m_strip = NULL;
            delete m_set;
            m_set = NULL;
            delete m_fans;
            m_fans = NULL;
            m_valid = false;
        }
        
        void CompassRenderer::render(Vbo& vbo, RenderContext& context) {
            SetVboState activateVbo(vbo, Vbo::VboActive);
            if (!m_valid)
                validate(vbo);
            
            glFrontFace(GL_CCW);
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            const Controller::AxisRestriction restriction = context.inputState().axisRestriction();
            const Mat4f cameraRotation = cameraRotationMatrix(context.camera());
            
            // the rendering order of the axes depends on which axis should have an outline
            
            if (restriction.restricted(Axis::AX)) {
                renderColoredAxis(context, cameraRotation,prefs.getColor(Preferences::ZColor));
                renderColoredAxis(context, cameraRotation * Mat4f::Rot90YCCW, prefs.getColor(Preferences::XColor));
                renderOutlinedAxis(context, cameraRotation * Mat4f::Rot90XCW, Color(1.0f, 1.0f, 1.0f, 1.0f));
                renderColoredAxis(context, cameraRotation * Mat4f::Rot90XCW, prefs.getColor(Preferences::YColor));
            } else if (restriction.restricted(Axis::AY)) {
                renderColoredAxis(context, cameraRotation,prefs.getColor(Preferences::ZColor));
                renderColoredAxis(context, cameraRotation * Mat4f::Rot90XCW, prefs.getColor(Preferences::YColor));
                renderOutlinedAxis(context, cameraRotation * Mat4f::Rot90YCCW, Color(1.0f, 1.0f, 1.0f, 1.0f));
                renderColoredAxis(context, cameraRotation * Mat4f::Rot90YCCW, prefs.getColor(Preferences::XColor));
            } else if (restriction.restricted(Axis::AZ)) {
                renderColoredAxis(context, cameraRotation * Mat4f::Rot90YCCW, prefs.getColor(Preferences::XColor));
                renderColoredAxis(context, cameraRotation * Mat4f::Rot90XCW, prefs.getColor(Preferences::YColor));
                renderOutlinedAxis(context, cameraRotation, Color(1.0f, 1.0f, 1.0f, 1.0f));
                renderColoredAxis(context, cameraRotation,prefs.getColor(Preferences::ZColor));
            } else {
                renderColoredAxis(context, cameraRotation,prefs.getColor(Preferences::ZColor));
                renderColoredAxis(context, cameraRotation * Mat4f::Rot90YCCW, prefs.getColor(Preferences::XColor));
                renderColoredAxis(context, cameraRotation * Mat4f::Rot90XCW, prefs.getColor(Preferences::YColor));
            }
        }
    }
}
