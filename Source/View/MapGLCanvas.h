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

#ifndef __TrenchBroom__MapGLCanvas__
#define __TrenchBroom__MapGLCanvas__

#include <wx/glcanvas.h>

namespace TrenchBroom {
    namespace View {
        class MapGLCanvas : public wxGLCanvas {
        protected:
            int* m_attribs;
            wxGLContext* m_glContext;
            
            int* Attribs();
        public:
            MapGLCanvas(wxWindow* parent);
            ~MapGLCanvas();
            
            void OnPaint(wxPaintEvent& event);
        protected:
            DECLARE_EVENT_TABLE()
        };
    }
}

#endif /* defined(__TrenchBroom__MapGLCanvas__) */
