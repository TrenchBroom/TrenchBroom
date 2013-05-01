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

        void CompassRenderer::renderAxis(Vbo& vbo, const Mat4f& rotation) {
            VertexArray strip(vbo, GL_TRIANGLE_STRIP, m_shaftVertices.size(),
                              Attribute::position3f(),
                              Attribute::normal3f(),
                              0);
            VertexArray set(vbo, GL_TRIANGLES, m_headVertices.size(),
                            Attribute::position3f(),
                            Attribute::normal3f(),
                            0);
            IndexedVertexArray fans(vbo, GL_TRIANGLE_FAN, m_headCapVertices.size() + m_shaftCapVertices.size(),
                                    Attribute::position3f(),
                                    Attribute::normal3f(),
                                    0);

            SetVboState mapVbo(vbo, Vbo::VboMapped);
            strip.addAttributes(rotation * m_shaftVertices, rotation * m_shaftNormals);
            set.addAttributes(rotation * m_headVertices, rotation * m_headNormals);
            fans.addAttributes(rotation * m_headCapVertices, rotation * m_headCapNormals);
            fans.endPrimitive();
            fans.addAttributes(rotation * m_shaftCapVertices, rotation * m_shaftCapNormals);
            fans.endPrimitive();
            
            SetVboState activateVbo(vbo, Vbo::VboActive);
            strip.render();
            set.render();
            fans.render();
        }
        
        CompassRenderer::CompassRenderer() {
            const Vec3f offset(0.0f, 0.0f, m_shaftLength / 2.0f);
            
            cylinder(m_shaftLength, m_shaftRadius, m_segments, m_shaftVertices, m_shaftNormals);
            for (size_t i = 0; i < m_shaftVertices.size(); i++)
                m_shaftVertices[i] -= offset;
            
            cone(m_headLength, m_headRadius, m_segments, m_headVertices, m_headNormals);
            for (size_t i = 0; i < m_headVertices.size(); i++)
                m_headVertices[i] += offset;
            
            circle(m_headRadius, m_segments, m_headCapVertices, m_headCapNormals);
            m_headCapVertices = Mat4f::Rot180X * m_headCapVertices;
            m_headCapNormals = Mat4f::Rot180X * m_headCapNormals;
            for (size_t i = 0; i < m_headCapVertices.size(); i++)
                m_headCapVertices[i] += offset;
            
            circle(m_shaftRadius, m_segments, m_shaftCapVertices, m_shaftCapNormals);
            m_shaftCapVertices = Mat4f::Rot180X * m_shaftCapVertices;
            m_shaftCapNormals = Mat4f::Rot180X * m_shaftCapNormals;
            for (size_t i = 0; i < m_shaftCapVertices.size(); i++)
                m_shaftCapVertices[i] -= offset;
        }
        
        void CompassRenderer::render(Vbo& vbo, RenderContext& context) {
            glFrontFace(GL_CCW);
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            const Controller::AxisRestriction restriction = context.inputState().axisRestriction();
            
            const Mat4f cameraRotation = cameraRotationMatrix(context.camera());
            
            ActivateShader compassShader(context.shaderManager(), Shaders::CompassShader);
            compassShader.setUniformVariable("CameraPosition", Vec3f(0.0f, 500.0f, 0.0f));
            compassShader.setUniformVariable("LightDirection", Vec3f(0.0f, 0.5f, 1.0f).normalized());
            compassShader.setUniformVariable("LightDiffuse", Color(1.0f, 1.0f, 1.0f, 1.0f));
            compassShader.setUniformVariable("LightSpecular", Color(0.3f, 0.3f, 0.3f, 1.0f));
            compassShader.setUniformVariable("GlobalAmbient", Color(0.2f, 0.2f, 0.2f, 1.0f));
            compassShader.setUniformVariable("MaterialShininess", 32.0f);
            
            compassShader.setUniformVariable("MaterialDiffuse", prefs.getColor(Preferences::ZColor));
            compassShader.setUniformVariable("MaterialAmbient", prefs.getColor(Preferences::ZColor));
            compassShader.setUniformVariable("MaterialSpecular", prefs.getColor(Preferences::ZColor));
            renderAxis(vbo, cameraRotation);
            
            compassShader.setUniformVariable("MaterialDiffuse", prefs.getColor(Preferences::XColor));
            compassShader.setUniformVariable("MaterialAmbient", prefs.getColor(Preferences::XColor));
            compassShader.setUniformVariable("MaterialSpecular", prefs.getColor(Preferences::XColor));
            renderAxis(vbo, cameraRotation * Mat4f::Rot90YCCW);

            compassShader.setUniformVariable("MaterialDiffuse", prefs.getColor(Preferences::YColor));
            compassShader.setUniformVariable("MaterialAmbient", prefs.getColor(Preferences::YColor));
            compassShader.setUniformVariable("MaterialSpecular", prefs.getColor(Preferences::YColor));
            renderAxis(vbo, cameraRotation * Mat4f::Rot90XCW);
        }
    }
}
