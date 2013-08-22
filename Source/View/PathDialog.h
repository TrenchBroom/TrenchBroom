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

#ifndef __TrenchBroom__PathDialog__
#define __TrenchBroom__PathDialog__

#include "Utility/String.h"

#include <wx/dialog.h>

class wxRadioButton;

namespace TrenchBroom {
    namespace View {
        class PathDialog : public wxDialog {
        private:
            String m_absolutePath;
            String m_mapRelativePath;
            String m_appRelativePath;
            String m_quakeRelativePath;
            
            wxRadioButton* m_absolute;
            wxRadioButton* m_relativeToMap;
            wxRadioButton* m_relativeToApp;
            wxRadioButton* m_relativeToQuake;
        public:
            PathDialog(wxWindow* parent, const String& path, const String& mapPath);
            
            const String& path() const;
            
            void OnOkClicked(wxCommandEvent& event);
            void OnCancelClicked(wxCommandEvent& event);

            DECLARE_EVENT_TABLE();
        };
    }
}

#endif /* defined(__TrenchBroom__PathDialog__) */
