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

#include "ImageListBox.h"

#include "View/ViewConstants.h"

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statbmp.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        ImageListBox::ImageListBox(wxWindow* parent, const wxString& emptyText) :
        ControlListBox(parent, emptyText) {}

        ControlListBox::Item* ImageListBox::createItem(wxWindow* parent, const wxSize& margins, const size_t index) {
            Item* container = new Item(parent);
            wxStaticBitmap* imagePanel = new wxStaticBitmap(container, wxID_ANY, image(index));
            wxStaticText* titleText = new wxStaticText(container, wxID_ANY, title(index), wxDefaultPosition, wxDefaultSize,  wxST_ELLIPSIZE_END);
            wxStaticText* subtitleText = new wxStaticText(container, wxID_ANY, subtitle(index), wxDefaultPosition, wxDefaultSize,  wxST_ELLIPSIZE_MIDDLE);
            
            titleText->SetFont(titleText->GetFont().Bold());
#ifndef _WIN32
            subtitleText->SetFont(subtitleText->GetFont().Smaller());
#endif
            
            wxSizer* vSizer = new wxBoxSizer(wxVERTICAL);
            vSizer->Add(titleText, 0);
            vSizer->Add(subtitleText, 0);
            
            wxSizer* hSizer = new wxBoxSizer(wxHORIZONTAL);
            hSizer->AddSpacer(margins.x);
            hSizer->Add(imagePanel, 0, wxALIGN_BOTTOM | wxTOP | wxBOTTOM, margins.y);
            hSizer->Add(vSizer, 0, wxTOP | wxBOTTOM, margins.y);
            hSizer->AddSpacer(margins.x);
            
            container->SetSizer(hSizer);
            return container;
        }

        const wxBitmap& ImageListBox::image(const size_t n) const {
            return wxNullBitmap;
        }
    }
}
