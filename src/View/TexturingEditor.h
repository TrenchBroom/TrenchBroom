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

#ifndef __TrenchBroom__TexturingEditor__
#define __TrenchBroom__TexturingEditor__

#include "View/GLContextHolder.h"
#include "View/ViewTypes.h"

#include <wx/panel.h>

class wxSpinCtrl;
class wxSpinEvent;
class wxWindow;

namespace TrenchBroom {
    namespace View {
        class TexturingView;
        
        class TexturingEditor : public wxPanel {
        private:
            MapDocumentWPtr m_document;
            ControllerWPtr m_controller;

            TexturingView* m_texturingView;
            wxSpinCtrl* m_xSubDivisionEditor;
            wxSpinCtrl* m_ySubDivisionEditor;
        public:
            TexturingEditor(wxWindow* parent, GLContextHolder::Ptr sharedContext, MapDocumentWPtr document, ControllerWPtr controller);

            void OnSubDivisionChanged(wxSpinEvent& event);
        private:
            void createGui(GLContextHolder::Ptr sharedContext);
            void bindEvents();
        };
    }
}

#endif /* defined(__TrenchBroom__TexturingEditor__) */
