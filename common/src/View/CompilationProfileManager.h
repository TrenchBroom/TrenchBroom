/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#ifndef CompilationProfileManager_h
#define CompilationProfileManager_h

#include "View/ViewTypes.h"

#include <wx/panel.h>

namespace TrenchBroom {
    namespace Model {
        class CompilationConfig;
        class CompilationProfile;
    }
    
    namespace View {
        class CompilationProfileListBox;
        class CompilationProfileEditor;
        
        class CompilationProfileManager : public QWidget {
        private:
            Model::CompilationConfig& m_config;
            CompilationProfileListBox* m_profileList;
            CompilationProfileEditor* m_profileEditor;
        public:
            CompilationProfileManager(QWidget* parent, MapDocumentWPtr document, Model::CompilationConfig& config);
            
            const Model::CompilationProfile* selectedProfile() const;
        private:
            void OnAddProfile(wxCommandEvent& event);
            void OnRemoveProfile(wxCommandEvent& event);
            void OnUpdateAddProfileButtonUI(wxUpdateUIEvent& event);
            void OnUpdateRemoveProfileButtonUI(wxUpdateUIEvent& event);
            
            void OnProfileSelectionChanged(wxCommandEvent& event);
        };
    }
}

#endif /* CompilationProfileManager_h */
