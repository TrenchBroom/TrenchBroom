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

#ifndef LaunchGameEngineDialog_h
#define LaunchGameEngineDialog_h

#include "IO/Path.h"
#include "View/CompilationVariables.h"
#include "View/ViewTypes.h"

#include <wx/dialog.h>

namespace TrenchBroom {
    namespace Model {
        class GameEngineProfile;
    }

    namespace View {
        class AutoCompleteTextControl;
        class GameEngineProfileListBox;

        class LaunchGameEngineDialog : public wxDialog {
        private:
            MapDocumentWPtr m_document;
            GameEngineProfileListBox* m_gameEngineList;
            AutoCompleteTextControl* m_parameterText;
            Model::GameEngineProfile* m_lastProfile;
        public:
            LaunchGameEngineDialog(wxWindow* parent, MapDocumentWPtr document);
        private:
            void createGui();
            LaunchGameEngineVariables variables() const;

            void OnSelectGameEngineProfile(wxCommandEvent& event);

            void OnUpdateParameterTextUI(wxUpdateUIEvent& event);
            void OnParameterTextChanged(wxCommandEvent& event);

            void OnEditGameEnginesButton(wxCommandEvent& event);

            void OnCloseButton(wxCommandEvent& event);
            void OnUpdateCloseButtonUI(wxUpdateUIEvent& event);

            void OnLaunch(wxCommandEvent& event);
            void OnUpdateLaunchButtonUI(wxUpdateUIEvent& event);

            void OnClose(wxCloseEvent& event);
        };
    }
}

#endif /* LaunchGameEngineDialog_h */
