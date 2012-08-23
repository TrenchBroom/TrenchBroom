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

#include "Inspector.h"

#include <wx/panel.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        wxNotebookPage* Inspector::CreateMapInspector() {
            return new wxPanel(this);
        }
        
        wxNotebookPage* Inspector::CreateEntityInspector() {
            return new wxPanel(this);
        }
        
        wxNotebookPage* Inspector::CreateBrushInspector() {
            return new wxPanel(this);
        }
        
        wxNotebookPage* Inspector::CreateFaceInspector() {
            return new wxPanel(this);
        }

        Inspector::Inspector(wxWindow* parent) : wxNotebook(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_TOP | wxCLIP_CHILDREN) {
            AddPage(CreateMapInspector(), wxT("Map"));
            AddPage(CreateEntityInspector(), wxT("Entity"));
            AddPage(CreateBrushInspector(), wxT("Brush"));
            AddPage(CreateFaceInspector(), wxT("Face"));
        }
    }
}