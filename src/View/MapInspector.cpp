/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "MapInspector.h"

#include "View/LayoutConstants.h"
#include "View/MapTreeView.h"

#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        MapInspector::MapInspector(wxWindow* parent, MapDocumentPtr document) :
        wxPanel(parent),
        m_treeView(NULL) {
            m_treeView = new MapTreeView(this, document);
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_treeView, 1, wxEXPAND | wxLEFT | wxTOP | wxRIGHT | wxBOTTOM, LayoutConstants::NotebookPageInnerMargin);

            SetSizerAndFit(sizer);
        }
    }
}
