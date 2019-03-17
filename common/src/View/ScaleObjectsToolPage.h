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

#ifndef TrenchBroom_ScaleObjectsToolPage
#define TrenchBroom_ScaleObjectsToolPage

#include "TrenchBroom.h"
#include "View/ViewTypes.h"

#include <wx/panel.h>

class wxButton;
class wxChoice;
class wxSimplebook;
class wxTextCtrl;
class wxSimplebook;
class wxChoice;

namespace TrenchBroom {
    namespace View {
        class ScaleObjectsTool;

        class ScaleObjectsToolPage : public wxPanel {
        private:
            MapDocumentWPtr m_document;

            wxSimplebook* m_book;

            wxTextCtrl* m_sizeTextBox;
            wxTextCtrl* m_factorsTextBox;

            wxChoice* m_scaleFactorsOrSize;
            wxButton* m_button;
        public:
            ScaleObjectsToolPage(wxWindow* parent, MapDocumentWPtr document);
            void activate();
        private:
            void createGui();

            void OnUpdateButton(wxUpdateUIEvent& event);
            void OnApply(wxCommandEvent& event);

            bool canScale() const;
            vm::vec3 getScaleFactors() const;
        };
    }
}

#endif /* defined(TrenchBroom_ScaleObjectsToolPage) */
