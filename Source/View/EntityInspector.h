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

#ifndef __TrenchBroom__EntityInspector__
#define __TrenchBroom__EntityInspector__

#include <wx/panel.h>

class wxWindow;
class wxGLContext;

namespace TrenchBroom {
    namespace View {
        class DocumentViewHolder;
        
        class EntityInspector : public wxPanel {
        protected:
            DocumentViewHolder& m_documentViewHolder;

            wxWindow* createPropertyEditor();
            wxWindow* createEntityBrowser(wxGLContext* sharedContext);
        public:
            EntityInspector(wxWindow* parent, DocumentViewHolder& documentViewHolder, wxGLContext* sharedContext);
        };
    }
}

#endif /* defined(__TrenchBroom__EntityInspector__) */
