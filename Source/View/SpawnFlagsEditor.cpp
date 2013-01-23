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

#include "SpawnFlagsEditor.h"

#include <wx/checkbox.h>
#include <wx/panel.h>
#include <wx/sizer.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        wxWindow* SpawnFlagsEditor::createVisual(wxWindow* parent) {
            assert(m_panel == NULL);
            
            m_panel = new wxPanel(parent);
            
            for (unsigned int i = 0; i < 24; i++)
                m_flags[i] = new wxCheckBox(m_panel, wxID_ANY, wxT(""));
            
            wxFlexGridSizer* sizer = new wxFlexGridSizer(8, 3, 0, 10);
            for (unsigned int row = 0; row < 8; row++)
                for (unsigned int col = 0; col < 3; col++)
                    sizer->Add(m_flags[col * 8 + row]);
            m_panel->SetSizerAndFit(sizer);
            return m_panel;
        }
        
        void SpawnFlagsEditor::destroyVisual() {
            assert(m_panel != NULL);
            m_panel->Destroy();
            m_panel = NULL;
        }
        
        void SpawnFlagsEditor::updateVisual(const Model::EntityList& entities) {
            
            for (unsigned int i = 0; i < 24; i++) {
                wxString label;
                label << (i + 1);
                if (i == 8)
                    label << " !Easy";
                else if (i == 9)
                    label << " !Normal";
                else if (i == 10)
                    label << " !Hard";
                else if (i == 11)
                    label << " !DM";
                m_flags[i]->SetLabel(label);
            }
            m_panel->Layout();
            m_panel->Fit();
        }

        SpawnFlagsEditor::SpawnFlagsEditor(SmartPropertyEditorManager& manager) :
        SmartPropertyEditor(manager),
        m_panel(NULL) {}
    }
}
