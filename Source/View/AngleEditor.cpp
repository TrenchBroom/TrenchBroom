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

#include "AngleEditor.h"

#include "Model/EditStateManager.h"
#include "Model/Entity.h"
#include "Model/MapDocument.h"
#include "Renderer/ApplyMatrix.h"
#include "Renderer/AxisFigure.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Shader/ShaderProgram.h"
#include "Renderer/SharedResources.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"
#include "Utility/Color.h"
#include "Utility/Preferences.h"
#include "View/EditorView.h"

namespace TrenchBroom {
    namespace View {
        BEGIN_EVENT_TABLE(AngleEditorCanvas, wxGLCanvas)
        EVT_PAINT(AngleEditorCanvas::OnPaint)
        END_EVENT_TABLE()

        AngleEditorCanvas::AngleEditorCanvas(wxWindow* parent, Renderer::SharedResources& sharedResources, Renderer::Camera& mapCamera) :
        wxGLCanvas(parent, wxID_ANY, sharedResources.attribs()),
        m_sharedResources(sharedResources),
        m_mapCamera(mapCamera),
        m_glContext(new wxGLContext(this, sharedResources.sharedContext())) {
        }

        void AngleEditorCanvas::OnPaint(wxPaintEvent& event) {
            wxPaintDC(this);
			if (SetCurrent(*m_glContext)) {
                Vec3f mapCamDir = m_mapCamera.direction();
                mapCamDir[2] = 0.0f;
                mapCamDir.normalize();
                const float camAngle = angleFrom(mapCamDir, Vec3f::PosX, Vec3f::PosZ);
                const float size = 48.0f;

                Vec3f dir(1.0f, 0.0f, -1.0f);
                dir.normalize();
                Vec3f pos = -50.0f * dir;

                Renderer::Camera camera(75.0f, 1.0f, 128.0f, pos, dir);
                camera.setOrtho(true);

                Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
                const Color& backgroundColor = prefs.getColor(Preferences::BackgroundColor);
                glClearColor(backgroundColor.x(), backgroundColor.y(), backgroundColor.z(), backgroundColor.w());
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                camera.update(0, 0, GetClientSize().x, GetClientSize().y);

                Renderer::Vbo vbo(GL_ARRAY_BUFFER, 0xFFF);
                Renderer::SetVboState mapVbo(vbo, Renderer::Vbo::VboMapped);

                Vec2f::List vertices;
                Renderer::circle(size, CircleSegments, vertices);

                Renderer::VertexArray circleArray(vbo, GL_LINE_LOOP, CircleSegments, Renderer::Attribute::position2f(), 0);
                circleArray.addAttributes(vertices);

                Renderer::VertexArray linesArray(vbo, GL_LINES, 48 + 4 + 2 * static_cast<unsigned int>(m_angles.size()),
                                                 Renderer::Attribute::position3f(),
                                                 Renderer::Attribute::color4f());

                float a = 0.0f;
                for (unsigned int i = 0; i < 24; i++) {
                    float s = std::sin(a);
                    float c = std::cos(a);

                    linesArray.addAttribute(Vec3f(0.85f * size * s, 0.85f * size * c, 0.0f));
                    linesArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 1.0f));
                    linesArray.addAttribute(Vec3f(1.0f * size * s, 1.0f * size * c, 0.0f));
                    linesArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 1.0f));

                    a += Math<float>::Pi / 12.0f;
                }

                linesArray.addAttribute(Vec3f(0.0f, 0.0f, 0.0f));
                linesArray.addAttribute(Color(1.0f, 0.0f, 0.0f, 1.0f));
                linesArray.addAttribute(Vec3f(size, 0.0f, 0.0f));
                linesArray.addAttribute(Color(1.0f, 0.0f, 0.0f, 1.0f));
                linesArray.addAttribute(Vec3f(0.0f, 0.0f, 0.0f));
                linesArray.addAttribute(Color(0.0f, 1.0f, 0.0f, 1.0f));
                linesArray.addAttribute(Vec3f(0.0f, size, 0.0f));
                linesArray.addAttribute(Color(0.0f, 1.0f, 0.0f, 1.0f));
                /*
                linesArray.addAttribute(Vec3f(0.0f, 0.0f, -size));
                linesArray.addAttribute(Color(0.0f, 0.0f, 1.0f, 1.0f));
                linesArray.addAttribute(Vec3f(0.0f, 0.0f, size));
                linesArray.addAttribute(Color(0.0f, 0.0f, 1.0f, 1.0f));
                 */

                AngleList::const_iterator angleIt, angleEnd;
                for (angleIt = m_angles.begin(), angleEnd = m_angles.end(); angleIt != angleEnd; ++angleIt) {
                    float angle = Math<float>::radians(*angleIt);
                    linesArray.addAttribute(Vec3f::Null);
                    linesArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 1.0f));
                    linesArray.addAttribute(Vec3f(size * std::cos(angle), size * std::sin(angle), 0.0f));
                    linesArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 1.0f));
                }

                Renderer::SetVboState activateVbo(vbo, Renderer::Vbo::VboActive);

                Renderer::Transformation transformation(camera.projectionMatrix(), camera.viewMatrix());
                Mat4f matrix;
                rotateCCW(matrix, camAngle, Vec3f::PosZ);
                Renderer::ApplyModelMatrix apply(transformation, matrix);

                Renderer::ActivateShader handleShader(m_sharedResources.shaderManager(), Renderer::Shaders::HandleShader);
                handleShader.currentShader().setUniformVariable("Color", Color(1.0f, 1.0f, 1.0f, 1.0f));
                circleArray.render();

                Renderer::ActivateShader coloredShader(m_sharedResources.shaderManager(), Renderer::Shaders::ColoredHandleShader);
                linesArray.render();

                SwapBuffers();
            }
        }

        wxWindow* AngleEditor::createVisual(wxWindow* parent) {
            assert(m_panel == NULL);
            assert(m_canvas == NULL);

            wxSize size = wxSize(parent->GetClientSize().y, parent->GetClientSize().y);
            m_panel = new wxPanel(parent, wxID_ANY, wxDefaultPosition, size, wxBORDER_SUNKEN);
            m_canvas = new AngleEditorCanvas(m_panel, document().sharedResources(), view().camera());

            wxSizer* innerSizer = new wxBoxSizer(wxHORIZONTAL);
            innerSizer->Add(m_canvas, 1, wxEXPAND);
            m_panel->SetSizer(innerSizer);

            wxSizer* outerSizer = new wxBoxSizer(wxHORIZONTAL);
            outerSizer->AddStretchSpacer();
            outerSizer->Add(m_panel, 0, wxALIGN_CENTER_VERTICAL);
            outerSizer->AddStretchSpacer();
            parent->SetSizer(outerSizer);

            return m_panel;
        }

        void AngleEditor::destroyVisual() {
            assert(m_panel != NULL);

            m_panel->Destroy();
            m_panel = NULL;
            m_canvas = NULL;
        }

        void AngleEditor::updateVisual() {
            assert(m_panel != NULL);

            AngleEditorCanvas::AngleList angles;

            const Model::EntityList& entities = document().editStateManager().allSelectedEntities();
            Model::EntityList::const_iterator entityIt, entityEnd;
            for (entityIt = entities.begin(), entityEnd = entities.end(); entityIt != entityEnd; ++entityIt) {
                const Model::Entity& entity = **entityIt;
                const Model::PropertyValue* angleValue = entity.propertyForKey(property());
                if (angleValue != NULL)
                    angles.push_back(static_cast<float>(std::atof(angleValue->c_str())));
            }

            m_canvas->setAngles(angles);
            m_canvas->Refresh();
        }

        AngleEditor::AngleEditor(SmartPropertyEditorManager& manager) :
        SmartPropertyEditor(manager),
        m_panel(NULL),
        m_canvas(NULL) {}
    }
}
