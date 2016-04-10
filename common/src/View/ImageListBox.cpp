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

#include "View/ImagePanel.h"
#include "View/ViewConstants.h"

#include <wx/panel.h>
#include <wx/gbsizer.h>
#include <wx/stattext.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        ImageListBox::ImageListBox(wxWindow* parent, const wxString& emptyText) :
        ControlListBox(parent, emptyText) {}

        wxWindow* ImageListBox::createItem(wxWindow* parent, const size_t index) {
            wxWindow* container = new wxWindow(parent, wxID_ANY);
            ImagePanel* imagePanel = new ImagePanel(container, image(index));
            wxStaticText* titleText = new wxStaticText(container, wxID_ANY, title(index), wxDefaultPosition, wxDefaultSize,  wxST_ELLIPSIZE_END);
            wxStaticText* subtitleText = new wxStaticText(container, wxID_ANY, subtitle(index), wxDefaultPosition, wxDefaultSize,  wxST_ELLIPSIZE_MIDDLE);
            
            titleText->SetFont(titleText->GetFont().Bold());
            subtitleText->SetFont(subtitleText->GetFont().Smaller());
            
            wxGridBagSizer* sizer = new wxGridBagSizer(0, 0);
            sizer->Add(imagePanel,      wxGBPosition(0, 0), wxGBSpan(2, 1), wxALIGN_BOTTOM);
            sizer->Add(titleText,       wxGBPosition(0, 1), wxDefaultSpan, wxALIGN_BOTTOM);
            sizer->Add(subtitleText,    wxGBPosition(1, 1), wxDefaultSpan, wxALIGN_TOP);
            sizer->AddGrowableCol(1);
            
            container->SetSizer(sizer);
            return container;
        }

        const wxBitmap& ImageListBox::image(const size_t n) const {
            return wxNullBitmap;
        }
    }
}
