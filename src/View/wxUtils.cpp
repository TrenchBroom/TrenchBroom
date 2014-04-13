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

#include "wxUtils.h"

#include <wx/frame.h>
#include <wx/window.h>

namespace TrenchBroom {
    namespace View {
        wxFrame* findFrame(wxWindow* window) {
            if (window == NULL)
                return NULL;
            
            wxFrame* frame = wxDynamicCast(window, wxFrame);
            wxWindow* parent = window->GetParent();
            while (frame == NULL && parent != NULL) {
                frame = wxDynamicCast(parent, wxFrame);
                parent = parent->GetParent();
            }
            return frame;
        }

        Color fromWxColor(const wxColor& color) {
            const float r = static_cast<float>(color.Red())   / 255.0f;
            const float g = static_cast<float>(color.Green()) / 255.0f;
            const float b = static_cast<float>(color.Blue())  / 255.0f;
            const float a = static_cast<float>(color.Alpha()) / 255.0f;
            return Color(r, g, b, a);
        }
        
        wxColor toWxColor(const Color& color) {
            const unsigned char r = static_cast<unsigned char>(color.r() * 255.0f);
            const unsigned char g = static_cast<unsigned char>(color.g() * 255.0f);
            const unsigned char b = static_cast<unsigned char>(color.b() * 255.0f);
            const unsigned char a = static_cast<unsigned char>(color.a() * 255.0f);
            return wxColor(r, g, b, a);
        }
    }
}
