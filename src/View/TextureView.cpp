/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "TextureView.h"

#include "Color.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "GL/GL.h"
#include "Renderer/RenderResources.h"

#include <wx/dcclient.h>

namespace TrenchBroom {
    namespace View {
        TextureView::TextureView(wxWindow* parent, wxWindowID windowId, Renderer::RenderResources& resources) :
        wxGLCanvas(parent, windowId, &resources.glAttribs().front(), wxDefaultPosition, wxDefaultSize),
        m_glContext(new wxGLContext(this, resources.sharedContext())) {
            Bind(wxEVT_PAINT, &TextureView::OnPaint, this);
        }
        
        TextureView::~TextureView() {
            if (m_glContext != NULL) {
                wxDELETE(m_glContext);
                m_glContext = NULL;
            }
        }

        void TextureView::OnPaint(wxPaintEvent& event) {
            if (!IsShownOnScreen())
                return;

            PreferenceManager& prefs = PreferenceManager::instance();
            const Color& backgroundColor = prefs.getColor(Preferences::BackgroundColor);
            
            if (SetCurrent(*m_glContext)) {
                wxPaintDC paintDC(this);

                glEnable(GL_MULTISAMPLE);
                glDisable(GL_DEPTH_TEST);
                glClearColor(backgroundColor.x(), backgroundColor.y(), backgroundColor.z(), backgroundColor.w());
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                
                SwapBuffers();
            }
        }
    }
}
