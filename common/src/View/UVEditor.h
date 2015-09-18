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

#ifndef TrenchBroom_UVEditor
#define TrenchBroom_UVEditor

#include "View/ViewTypes.h"

#include <wx/panel.h>

class wxButton;
class wxSpinCtrl;
class wxSpinEvent;
class wxWindow;

namespace TrenchBroom {
    namespace View {
        class GLContextManager;
        class UVView;
        
        class UVEditor : public wxPanel {
        private:
            MapDocumentWPtr m_document;

            UVView* m_uvView;
            wxSpinCtrl* m_xSubDivisionEditor;
            wxSpinCtrl* m_ySubDivisionEditor;
        public:
            UVEditor(wxWindow* parent, MapDocumentWPtr document, GLContextManager& contextManager);

            void OnResetTexture(wxCommandEvent& event);
            void OnFlipTextureH(wxCommandEvent& event);
            void OnFlipTextureV(wxCommandEvent& event);
            void OnRotateTextureCCW(wxCommandEvent& event);
            void OnRotateTextureCW(wxCommandEvent& event);
            void OnUpdateButtonUI(wxUpdateUIEvent& event);
            
            void OnSubDivisionChanged(wxSpinEvent& event);
        private:
            void createGui(GLContextManager& contextManager);
        };
    }
}

#endif /* defined(TrenchBroom_UVEditor) */
