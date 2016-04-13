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

#ifndef CompilationProfileEditor_h
#define CompilationProfileEditor_h

#include <wx/panel.h>

class wxTextCtrl;

namespace TrenchBroom {
    namespace Model {
        class CompilationProfile;
    }

    namespace View {
        class CompilationTaskList;
        
        class CompilationProfileEditor : public wxPanel {
        private:
            Model::CompilationProfile* m_profile;
            wxTextCtrl* m_nameTxt;
            wxTextCtrl* m_workDirTxt;
            CompilationTaskList* m_taskList;
        public:
            CompilationProfileEditor(wxWindow* parent);
            ~CompilationProfileEditor();
        private:
            void OnNameChanged(wxCommandEvent& event);
            void OnWorkDirChanged(wxCommandEvent& event);
            void OnUpdateTxtUI(wxUpdateUIEvent& event);
            
            void OnAddTask(wxCommandEvent& event);
            void OnRemoveTask(wxCommandEvent& event);
            void OnMoveTaskUp(wxCommandEvent& event);
            void OnMoveTaskDown(wxCommandEvent& event);
            
            void OnUpdateAddTaskButtonUI(wxUpdateUIEvent& event);
            void OnUpdateRemoveTaskButtonUI(wxUpdateUIEvent& event);
            void OnUpdateMoveTaskUpButtonUI(wxUpdateUIEvent& event);
            void OnUpdateMoveTaskDownButtonUI(wxUpdateUIEvent& event);
        public:
            void setProfile(Model::CompilationProfile* profile);
        private:
            void profileDidChange();
            void refresh();
        };
    }
}

#endif /* CompilationProfileEditor_h */
