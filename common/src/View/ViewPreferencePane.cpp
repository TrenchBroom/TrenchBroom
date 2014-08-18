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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ViewPreferencePane.h"

#include "StringUtils.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "View/BorderLine.h"
#include "View/ViewConstants.h"

#include <wx/choice.h>
#include <wx/gbsizer.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {
        static const size_t NumTextureModes = 6;
        static const std::pair<String,int> TextureModes[] = {
            std::make_pair("GL_NEAREST",                0x2600),
            std::make_pair("GL_LINEAR",                 0x2601),
            std::make_pair("GL_NEAREST_MIPMAP_NEAREST", 0x2700),
            std::make_pair("GL_LINEAR_MIPMAP_NEAREST",  0x2701),
            std::make_pair("GL_NEAREST_MIPMAP_LINEAR",  0x2702),
            std::make_pair("GL_LINEAR_MIPMAP_LINEAR",   0x2703)
        };
        
        ViewPreferencePane::ViewPreferencePane(wxWindow* parent) :
        PreferencePane(parent) {
            createGui();
            bindEvents();
        }


        void ViewPreferencePane::OnBrightnessChanged(wxScrollEvent& event) {
            const int value = m_brightnessSlider->GetValue();
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::Brightness, value / 40.0f);
        }
        
        void ViewPreferencePane::OnGridAlphaChanged(wxScrollEvent& event) {
            const int value = m_gridAlphaSlider->GetValue();
            
            PreferenceManager& prefs = PreferenceManager::instance();
            const int max = m_gridAlphaSlider->GetMax();
            const float floatValue = static_cast<float>(value) / static_cast<float>(max);
            prefs.set(Preferences::GridAlpha, floatValue);
        }

        void ViewPreferencePane::OnTextureModeChanged(wxCommandEvent& event) {
            const int selection = m_textureModeChoice->GetSelection();
            assert(selection >= 0);
            
            const size_t index = static_cast<size_t>(selection);
            assert(index < NumTextureModes);
            const int mode = TextureModes[index].second;
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::TextureMode, mode);
        }

        void ViewPreferencePane::OnTextureBrowserIconSizeChanged(wxCommandEvent& event) {
            PreferenceManager& prefs = PreferenceManager::instance();

            const int selection = m_textureBrowserIconSizeChoice->GetSelection();
            switch (selection) {
                case 0:
                    prefs.set(Preferences::TextureBrowserIconSize, 0.25f);
                    break;
                case 1:
                    prefs.set(Preferences::TextureBrowserIconSize, 0.5f);
                    break;
                case 2:
                    prefs.set(Preferences::TextureBrowserIconSize, 1.0f);
                    break;
                case 3:
                    prefs.set(Preferences::TextureBrowserIconSize, 1.5f);
                    break;
                case 4:
                    prefs.set(Preferences::TextureBrowserIconSize, 2.0f);
                    break;
                case 5:
                    prefs.set(Preferences::TextureBrowserIconSize, 2.5f);
                    break;
                case 6:
                    prefs.set(Preferences::TextureBrowserIconSize, 3.0f);
                    break;
            }
        }

        void ViewPreferencePane::createGui() {
            wxWindow* viewPreferences = createViewPreferences();
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->AddSpacer(LayoutConstants::NarrowVMargin);
            sizer->Add(viewPreferences, 1, wxEXPAND);
            sizer->AddSpacer(LayoutConstants::WideVMargin);
            
            SetMinSize(sizer->GetMinSize());
            SetSizer(sizer);

            SetBackgroundColour(*wxWHITE);
        }
        
        wxWindow* ViewPreferencePane::createViewPreferences() {
            wxPanel* viewBox = new wxPanel(this);
            viewBox->SetBackgroundColour(*wxWHITE);
            
            wxStaticText* _3dViewPrefsHeader = new wxStaticText(viewBox, wxID_ANY, "3D View");
            _3dViewPrefsHeader->SetFont(_3dViewPrefsHeader->GetFont().Bold());
            wxStaticText* brightnessLabel = new wxStaticText(viewBox, wxID_ANY, "Brightness");
            m_brightnessSlider = new wxSlider(viewBox, wxID_ANY, 50, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_BOTTOM);
            wxStaticText* gridLabel = new wxStaticText(viewBox, wxID_ANY, "Grid");
            m_gridAlphaSlider = new wxSlider(viewBox, wxID_ANY, 50, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_BOTTOM);
            
            wxString textureModeNames[NumTextureModes];
            for (size_t i = 0; i < NumTextureModes; ++i)
                textureModeNames[i] = TextureModes[i].first;
            wxStaticText* textureModeLabel = new wxStaticText(viewBox, wxID_ANY, "Texture Mode");
            m_textureModeChoice = new wxChoice(viewBox, wxID_ANY, wxDefaultPosition, wxDefaultSize, NumTextureModes, textureModeNames);

            wxStaticText* textureBrowserPrefsHeader = new wxStaticText(viewBox, wxID_ANY, "Texture Browser");
            textureBrowserPrefsHeader->SetFont(textureBrowserPrefsHeader->GetFont().Bold());
            wxStaticText* textureBrowserIconSizeLabel = new wxStaticText(viewBox, wxID_ANY, "Icon Size");
            wxString iconSizes[7] = {"25%", "50%", "100%", "150%", "200%", "250%", "300%"};
            m_textureBrowserIconSizeChoice = new wxChoice(viewBox, wxID_ANY, wxDefaultPosition, wxDefaultSize, 7, iconSizes);
            m_textureBrowserIconSizeChoice->SetToolTip("Sets the icon size in the texture browser.");
            
            
            const int HMargin       = LayoutConstants::WideHMargin;
            const int LMargin       = LayoutConstants::WideVMargin;
            const int HeaderFlags   = wxLEFT;
            const int LabelFlags    = wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxLEFT;
            const int SliderFlags   = wxEXPAND | wxRIGHT;
            const int ChoiceFlags   = wxRIGHT;
            const int LineFlags     = wxEXPAND | wxTOP;
            
            wxGridBagSizer* sizer = new wxGridBagSizer(LayoutConstants::NarrowVMargin, LayoutConstants::WideHMargin);
            sizer->Add(_3dViewPrefsHeader,                  wxGBPosition( 0, 0), wxGBSpan(1,2), HeaderFlags, HMargin);

            sizer->Add(brightnessLabel,                     wxGBPosition( 1, 0), wxDefaultSpan, LabelFlags, HMargin);
            sizer->Add(m_brightnessSlider,                  wxGBPosition( 1, 1), wxDefaultSpan, SliderFlags, HMargin);

            sizer->Add(gridLabel,                           wxGBPosition( 2, 0), wxDefaultSpan, LabelFlags, HMargin);
            sizer->Add(m_gridAlphaSlider,                   wxGBPosition( 2, 1), wxDefaultSpan, SliderFlags, HMargin);
            
            sizer->Add(textureModeLabel,                    wxGBPosition( 3, 0), wxDefaultSpan, LabelFlags, HMargin);
            sizer->Add(m_textureModeChoice,                 wxGBPosition( 3, 1), wxDefaultSpan, ChoiceFlags, HMargin);
            sizer->Add(0, LayoutConstants::ChoiceSizeDelta, wxGBPosition( 4, 0), wxGBSpan(1,2));

            sizer->Add(new BorderLine(viewBox),             wxGBPosition( 5, 0), wxGBSpan(1,2), LineFlags, LMargin);
            
            sizer->Add(textureBrowserPrefsHeader,           wxGBPosition( 6, 0), wxGBSpan(1,2), HeaderFlags, HMargin);
            sizer->Add(textureBrowserIconSizeLabel,         wxGBPosition( 7, 0), wxDefaultSpan, LabelFlags, HMargin);
            sizer->Add(m_textureBrowserIconSizeChoice,      wxGBPosition( 7, 1), wxDefaultSpan, ChoiceFlags, HMargin);
            sizer->Add(0, LayoutConstants::ChoiceSizeDelta, wxGBPosition( 8, 0), wxGBSpan(1,2));
            
            sizer->AddGrowableCol(1);
            sizer->SetMinSize(500, wxDefaultCoord);
            viewBox->SetSizer(sizer);
            return viewBox;
        }
        
        void ViewPreferencePane::bindEvents() {
            m_textureModeChoice->Bind(wxEVT_CHOICE, &ViewPreferencePane::OnTextureModeChanged, this);
            m_textureBrowserIconSizeChoice->Bind(wxEVT_CHOICE, &ViewPreferencePane::OnTextureBrowserIconSizeChanged, this);
            
            bindSliderEvents(m_brightnessSlider, &ViewPreferencePane::OnBrightnessChanged, this);
            bindSliderEvents(m_gridAlphaSlider, &ViewPreferencePane::OnGridAlphaChanged, this);
        }

        void ViewPreferencePane::doUpdateControls() {
            PreferenceManager& prefs = PreferenceManager::instance();
            
            m_brightnessSlider->SetValue(static_cast<int>(prefs.get(Preferences::Brightness) * 40.0f));
            m_gridAlphaSlider->SetValue(static_cast<int>(prefs.get(Preferences::GridAlpha) * m_gridAlphaSlider->GetMax()));
            
            const size_t textureModeIndex = findTextureMode(prefs.get(Preferences::TextureMode));
            assert(textureModeIndex < NumTextureModes);
            m_textureModeChoice->SetSelection(static_cast<int>(textureModeIndex));
            
            const float textureBrowserIconSize = prefs.get(Preferences::TextureBrowserIconSize);
            if (textureBrowserIconSize == 0.25f)
                m_textureBrowserIconSizeChoice->SetSelection(0);
            else if (textureBrowserIconSize == 0.5f)
                m_textureBrowserIconSizeChoice->SetSelection(1);
            else if (textureBrowserIconSize == 1.5f)
                m_textureBrowserIconSizeChoice->SetSelection(3);
            else if (textureBrowserIconSize == 2.0f)
                m_textureBrowserIconSizeChoice->SetSelection(4);
            else if (textureBrowserIconSize == 2.5f)
                m_textureBrowserIconSizeChoice->SetSelection(5);
            else if (textureBrowserIconSize == 3.0f)
                m_textureBrowserIconSizeChoice->SetSelection(6);
            else
                m_textureBrowserIconSizeChoice->SetSelection(2);
        }

        bool ViewPreferencePane::doValidate() {
            return true;
        }

        size_t ViewPreferencePane::findTextureMode(const int value) const {
            for (size_t i = 0; i < NumTextureModes; ++i)
                if (TextureModes[i].second == value)
                    return i;
            return NumTextureModes;
        }
	}
}
