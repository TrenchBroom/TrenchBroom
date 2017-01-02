/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#ifndef GameEngineProfileEditor_h
#define GameEngineProfileEditor_h

#include <wx/panel.h>

class wxSimplebook;
class wxTextCtrl;

namespace TrenchBroom {
    namespace Model {
        class GameEngineProfile;
    }
    
    namespace View {
        class GameEngineProfileEditor : public wxPanel {
        private:
            Model::GameEngineProfile* m_profile;
            wxSimplebook* m_book;
            wxTextCtrl* m_nameText;
            wxTextCtrl* m_pathText;
        public:
            GameEngineProfileEditor(wxWindow* parent);
            ~GameEngineProfileEditor();
        private:
            wxWindow* createEditorPage(wxWindow* parent);
            
            void OnNameChanged(wxCommandEvent& event);
            void OnPathChanged(wxCommandEvent& event);
            void OnChangePathClicked(wxCommandEvent& event);
            void OnUpdatePathTextUI(wxIdleEvent& event);
            
            void updatePath(const wxString& str);
        public:
            void setProfile(Model::GameEngineProfile* profile);
        private:
            void profileWillBeRemoved();
            void profileDidChange();
            void refresh();
            
            bool isValidEnginePath(const wxString& str) const;
        };
    }
}

#endif /* GameEngineProfileEditor_h */
