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
#include "View/wxUtils.h"

#include <wx/panel.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statbmp.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        ImageListBox::ImageListBox(wxWindow* parent, const wxString& emptyText) :
        ControlListBox(parent, true, emptyText) {}

        class ImageListBox::ImageListBoxItem : public Item {
        private:
            wxStaticText* m_titleText;
            wxStaticText* m_subtitleText;
            wxStaticBitmap* m_imageBmp;
        public:
            ImageListBoxItem(wxWindow* parent, const wxSize& margins, const wxString& title, const wxString& subtitle) :
            Item(parent),
            m_titleText(NULL),
            m_subtitleText(NULL),
            m_imageBmp(NULL) {
                createGui(margins, title, subtitle, NULL);
            }
            
            ImageListBoxItem(wxWindow* parent, const wxSize& margins, const wxString& title, const wxString& subtitle, const wxBitmap& image)  :
            Item(parent),
            m_titleText(NULL),
            m_subtitleText(NULL),
            m_imageBmp(NULL) {
                createGui(margins, title, subtitle, &image);
            }

            void setDefaultColours(const wxColour& foreground, const wxColour& background) {
                Item::setDefaultColours(foreground, background);
                m_subtitleText->SetForegroundColour(makeLighter(m_subtitleText->GetForegroundColour()));
            }
        private:
            void createGui(const wxSize& margins, const wxString& title, const wxString& subtitle, const wxBitmap* image) {
                m_titleText = new wxStaticText(this, wxID_ANY, title, wxDefaultPosition, wxDefaultSize,  wxST_ELLIPSIZE_END);
                m_subtitleText = new wxStaticText(this, wxID_ANY, subtitle, wxDefaultPosition, wxDefaultSize,  wxST_ELLIPSIZE_MIDDLE);
                
                m_titleText->SetFont(m_titleText->GetFont().Bold());
                m_subtitleText->SetForegroundColour(makeLighter(m_subtitleText->GetForegroundColour()));
#ifndef _WIN32
                m_subtitleText->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif
                
                wxSizer* vSizer = new wxBoxSizer(wxVERTICAL);
                vSizer->Add(m_titleText, 0);
                vSizer->Add(m_subtitleText, 0);
                
                wxSizer* hSizer = new wxBoxSizer(wxHORIZONTAL);
                hSizer->AddSpacer(margins.x);
                
                if (image != NULL) {
                    m_imageBmp = new wxStaticBitmap(this, wxID_ANY, *image);
                    hSizer->Add(m_imageBmp, 0, wxALIGN_BOTTOM | wxTOP | wxBOTTOM, margins.y);
                }
                hSizer->Add(vSizer, 0, wxTOP | wxBOTTOM, margins.y);
                hSizer->AddSpacer(margins.x);
                
                SetSizer(hSizer);
            }
        };

        ControlListBox::Item* ImageListBox::createItem(wxWindow* parent, const wxSize& margins, const size_t index) {
            wxBitmap bitmap;
            if (image(index, bitmap))
                return new ImageListBoxItem(parent, margins, title(index), subtitle(index), bitmap);
            else
                return new ImageListBoxItem(parent, margins, title(index), subtitle(index));
        }

        bool ImageListBox::image(const size_t n, wxBitmap& result) const {
            result = wxNullBitmap;
            return false;
        }
    }
}
