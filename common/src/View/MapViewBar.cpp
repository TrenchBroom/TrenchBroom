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

#include "MapViewBar.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "View/MapDocument.h"
#include "View/ViewConstants.h"
#include "View/ViewEditor.h"
#include "View/wxUtils.h"

#include <wx/dcclient.h>
#include <wx/simplebook.h>
#include <wx/sizer.h>
#include <wx/srchctrl.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {
        MapViewBar::MapViewBar(wxWindow* parent, MapDocumentWPtr document) :
        ContainerBar(parent, wxBOTTOM),
        m_document(document),
        m_toolBook(NULL),
        m_viewEditor(NULL) {
#if defined __APPLE__
            SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif
            createGui(document);
        }
        
        wxBookCtrlBase* MapViewBar::toolBook() {
            return m_toolBook;
        }

        void MapViewBar::OnSearchPatternChanged(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
        }

        void MapViewBar::createGui(MapDocumentWPtr document) {
            m_toolBook = new wxSimplebook(this);
            m_viewEditor = new ViewPopupEditor(this, document);
            
            wxSizer* hSizer = new wxBoxSizer(wxHORIZONTAL);
            hSizer->AddSpacer(LayoutConstants::NarrowHMargin);
            hSizer->Add(m_toolBook, 1, wxEXPAND | wxALIGN_CENTRE_VERTICAL);
            hSizer->AddSpacer(LayoutConstants::MediumHMargin);
            hSizer->Add(m_viewEditor, 0, wxALIGN_CENTRE_VERTICAL);
            hSizer->AddSpacer(LayoutConstants::NarrowHMargin);
            
            wxSizer* vSizer = new wxBoxSizer(wxVERTICAL);
            vSizer->AddSpacer(LayoutConstants::NarrowVMargin);
            vSizer->Add(hSizer, 1, wxEXPAND);
            vSizer->AddSpacer(LayoutConstants::NarrowVMargin);
            
            SetSizer(vSizer);
        }
    }
}
