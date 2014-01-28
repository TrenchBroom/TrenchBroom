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

#include "Inspector.h"

#include "View/EntityInspector.h"
#include "View/FaceInspector.h"
#include "View/MapInspector.h"
#include "View/ViewInspector.h"

#include <wx/notebook.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        Inspector::Inspector(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller, Renderer::RenderResources& resources) :
        wxPanel(parent) {
            m_notebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_TOP | wxCLIP_CHILDREN);
            m_mapInspector = new MapInspector(m_notebook, document, controller);
            m_entityInspector = new EntityInspector(m_notebook, document, controller, resources);
            m_faceInspector = new FaceInspector(m_notebook, document, controller, resources);
            m_viewInspector = new ViewInspector(m_notebook);
            
            m_notebook->AddPage(m_mapInspector, "Map");
            m_notebook->AddPage(m_entityInspector, "Entity");
            m_notebook->AddPage(m_faceInspector, "Face");
            m_notebook->AddPage(m_viewInspector, "View");
            
            wxSizer* notebookSizer = new wxBoxSizer(wxVERTICAL);
            notebookSizer->Add(m_notebook, 1, wxEXPAND);
            SetSizer(notebookSizer);
        }

        void Inspector::switchToPage(const InspectorPage page) {
            m_notebook->SetSelection(static_cast<size_t>(page));
        }
    }
}
