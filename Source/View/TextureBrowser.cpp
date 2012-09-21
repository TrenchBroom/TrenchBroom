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

#include "TextureBrowser.h"

#include "View/TextureBrowserCanvas.h"

#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        TextureBrowser::TextureBrowser(wxWindow* parent, wxGLContext* sharedContext, Utility::Console& console, Model::TextureManager& textureManager) :
        wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN) {
            m_scrollBar = new wxScrollBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL);
            m_canvas = new TextureBrowserCanvas(this, sharedContext, console, textureManager, m_scrollBar);
            
            wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
            sizer->Add(m_canvas, 1, wxEXPAND);
            sizer->Add(m_scrollBar, 0, wxEXPAND);
            
            SetSizerAndFit(sizer);
        }

        void TextureBrowser::reload() {
            m_canvas->reload();
        }
    }
}
