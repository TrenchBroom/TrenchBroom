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

#include "Renderer/ApplyMatrix.h"
#include "Renderer/IndexedVertexArray.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/Shader/Shader.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"
#include "Utility/Preferences.h"
#include "Utility/VecMath.h"

#include <algorithm>
#include <cassert>

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Renderer {
        const float CompassRenderer::m_shaftLength = 28.0f;
        const float CompassRenderer::m_shaftRadius = 1.2f;
        const float CompassRenderer::m_headLength = 7.0f;
        const float CompassRenderer::m_headRadius = 3.5f;

        CompassRenderer::CompassRenderer() :
        m_strip(NULL),
        m_set(NULL),
        m_fans(NULL) {}

        CompassRenderer::~CompassRenderer() {
            delete m_fans;
            m_fans = NULL;
            delete m_set;
            m_set = NULL;
            delete m_strip;
            m_strip = NULL;
        }

        void CompassRenderer::render(Vbo& vbo, RenderContext& context) {
            SetVboState activateVbo(vbo, Vbo::VboActive);
            if (m_fans == NULL) {
                assert(m_strip == NULL);

                Vec3f::List headVertices, headNormals;
                Vec3f::List shaftVertices, shaftNormals;
                Vec3f::List topCapVertices, topCapNormals;
                Vec3f::List bottomCapVertices, bottomCapNormals;
                const Vec3f offset(0.0f, 0.0f, m_shaftLength / 2.0f);

                cylinder(m_shaftLength, m_shaftRadius, m_segments, shaftVertices, shaftNormals);
                for (size_t i = 0; i < shaftVertices.size(); i++)
                    shaftVertices[i] -= offset;

                cone(m_headLength, m_headRadius, m_segments, headVertices, headNormals);
                for (size_t i = 0; i < headVertices.size(); i++)
                    headVertices[i] += offset;

                circle(m_headRadius, m_segments, topCapVertices, topCapNormals);
                topCapVertices = Mat4f::Rot180X * topCapVertices;
                topCapNormals = Mat4f::Rot180X * topCapNormals;
                for (size_t i = 0; i < topCapVertices.size(); i++)
                    topCapVertices[i] += offset;

                circle(m_shaftRadius, m_segments, bottomCapVertices, bottomCapNormals);
                bottomCapVertices = Mat4f::Rot180X * bottomCapVertices;
                bottomCapNormals = Mat4f::Rot180X * bottomCapNormals;
                for (size_t i = 0; i < bottomCapVertices.size(); i++)
                    bottomCapVertices[i] -= offset;

                m_strip = new VertexArray(vbo, GL_TRIANGLE_STRIP, static_cast<unsigned int>(shaftVertices.size()),
                                          Attribute::position3f(),
                                          Attribute::normal3f(),
                                          0);

                m_set = new VertexArray(vbo, GL_TRIANGLES, static_cast<unsigned int>(headVertices.size()),
                                        Attribute::position3f(),
                                        Attribute::normal3f(),
                                        0);

                m_fans = new IndexedVertexArray(vbo, GL_TRIANGLE_FAN, static_cast<unsigned int>(topCapVertices.size() + bottomCapVertices.size()),
                                                Attribute::position3f(),
                                                Attribute::normal3f(),
                                                0);

                SetVboState mapVbo(vbo, Vbo::VboMapped);
                m_strip->addAttributes(shaftVertices, shaftNormals);
                m_set->addAttributes(headVertices, headNormals);
                m_fans->addAttributes(topCapVertices, topCapNormals);
                m_fans->endPrimitive();
                m_fans->addAttributes(bottomCapVertices, bottomCapNormals);
                m_fans->endPrimitive();
            }

            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            glFrontFace(GL_CCW);

            Mat4f rotation = Mat4f::Identity;
            rotation[0] = context.camera().right();
            rotation[1] = context.camera().direction();
            rotation[2] = context.camera().up();

            bool invertible;
            invertMatrix(rotation, invertible);
            assert(invertible);

            ApplyModelMatrix applyRotation(context.transformation(), rotation);

            ActivateShader compassShader(context.shaderManager(), Shaders::CompassShader);
            compassShader.currentShader().setUniformVariable("CameraPosition", Vec3f(0.0f, 500.0f, 0.0f));
            compassShader.currentShader().setUniformVariable("LightDirection", Vec3f(0.0f, 0.5f, 1.0f).normalized());
            compassShader.currentShader().setUniformVariable("LightDiffuse", Color(1.0f, 1.0f, 1.0f, 1.0f));
            compassShader.currentShader().setUniformVariable("LightSpecular", Color(0.3f, 0.3f, 0.3f, 1.0f));
            compassShader.currentShader().setUniformVariable("GlobalAmbient", Color(0.2f, 0.2f, 0.2f, 1.0f));
            compassShader.currentShader().setUniformVariable("MaterialShininess", 32.0f);

            // render Z axis
            compassShader.currentShader().setUniformVariable("MaterialDiffuse", prefs.getColor(Preferences::ZColor));
            compassShader.currentShader().setUniformVariable("MaterialAmbient", prefs.getColor(Preferences::ZColor));
            compassShader.currentShader().setUniformVariable("MaterialSpecular", prefs.getColor(Preferences::ZColor));
            m_strip->render();
            m_set->render();
            m_fans->render();

            { // render X axis
                ApplyModelMatrix xRotation(context.transformation(), Mat4f::Rot90YCCW);
                compassShader.currentShader().setUniformVariable("MaterialDiffuse", prefs.getColor(Preferences::XColor));
                compassShader.currentShader().setUniformVariable("MaterialAmbient", prefs.getColor(Preferences::XColor));
                compassShader.currentShader().setUniformVariable("MaterialSpecular", prefs.getColor(Preferences::XColor));
                m_strip->render();
                m_set->render();
                m_fans->render();
            }

            { // render Y axis
                ApplyModelMatrix yRotation(context.transformation(), Mat4f::Rot90XCW);
                compassShader.currentShader().setUniformVariable("MaterialDiffuse", prefs.getColor(Preferences::YColor));
                compassShader.currentShader().setUniformVariable("MaterialAmbient", prefs.getColor(Preferences::YColor));
                compassShader.currentShader().setUniformVariable("MaterialSpecular", prefs.getColor(Preferences::YColor));
                m_strip->render();
                m_set->render();
                m_fans->render();
            }
        }
    }
}
