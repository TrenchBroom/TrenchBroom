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

#include "Model/MapDocument.h"
#include "Renderer/SharedResources.h"
#include "Utility/Color.h"
#include "Utility/Preferences.h"

namespace TrenchBroom {
    namespace View {
        BEGIN_EVENT_TABLE(AngleEditorCanvas, wxGLCanvas)
        EVT_PAINT(AngleEditorCanvas::OnPaint)
        END_EVENT_TABLE()

        AngleEditorCanvas::AngleEditorCanvas(wxWindow* parent, const int* attribs) :
        wxGLCanvas(parent, wxID_ANY, attribs),
        m_glContext(new wxGLContext(this)) {
        }
        
        void AngleEditorCanvas::OnPaint(wxPaintEvent& event) {
            wxPaintDC(this);
			if (SetCurrent(*m_glContext)) {
                Mat4f matrix;
                matrix.setOrtho(-1.0f, 1.0f, 0.0f, 0.0f, GetClientSize().x, GetClientSize().y);
                
                Mat4f view;
                view.setView(Vec3f(0.0f, 1.0f, -1.0f).normalized(), Vec3f(0.0f, 1.0f, 1.0f).normalized());
                view.translate(0.0f, -1.0f, 1.0f);
                matrix *= view;

                Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
                const Color& backgroundColor = prefs.getColor(Preferences::BackgroundColor);
                glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, backgroundColor.w);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                
                glViewport(0.0f, 0.0f, GetClientSize().x, GetClientSize().y);
                glMatrixMode(GL_PROJECTION);
                glLoadMatrixf(matrix.v);
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();

                SwapBuffers();
            }
        }

        wxWindow* AngleEditor::createVisual(wxWindow* parent) {
            assert(m_panel == NULL);
            assert(m_canvas == NULL);
            
            wxSize size = wxSize(parent->GetClientSize().y, parent->GetClientSize().y);
            m_panel = new wxPanel(parent, wxID_ANY, wxDefaultPosition, size, wxBORDER_SUNKEN);
            m_canvas = new AngleEditorCanvas(m_panel, document().sharedResources().attribs());

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
        }
        
        AngleEditor::AngleEditor(SmartPropertyEditorManager& manager) :
        SmartPropertyEditor(manager),
        m_panel(NULL),
        m_canvas(NULL) {}
    }
}
