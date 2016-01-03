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
#include "View/TitleBar.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include "Renderer/GL.h"

#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/clrpicker.h>
#include <wx/gbsizer.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {
        struct TextureMode {
            int minFilter;
            int magFilter;
            String name;
            
            TextureMode(const int i_minFilter, const int i_magFilter, const String& i_name) :
            minFilter(i_minFilter),
            magFilter(i_magFilter),
            name(i_name) {}
        };
        
        static const size_t NumTextureModes = 6;
        static const TextureMode TextureModes[] = {
            TextureMode(GL_NEAREST,                 GL_NEAREST, "Nearest"),
            TextureMode(GL_NEAREST_MIPMAP_NEAREST,  GL_NEAREST, "Nearest (mipmapped)"),
            TextureMode(GL_NEAREST_MIPMAP_LINEAR,   GL_NEAREST, "Nearest (mipmaps, interpolated)"),
            TextureMode(GL_LINEAR,                  GL_LINEAR,  "Linear"),
            TextureMode(GL_LINEAR_MIPMAP_NEAREST,   GL_LINEAR,  "Linear (mipmapped)"),
            TextureMode(GL_LINEAR_MIPMAP_LINEAR,    GL_LINEAR,  "Linear (mipmapped, interpolated")
        };
        
        static const size_t NumFrameLayouts = 4;
        
        ViewPreferencePane::ViewPreferencePane(wxWindow* parent) :
        PreferencePane(parent) {
            createGui();
            bindEvents();
        }


        void ViewPreferencePane::OnLayoutChanged(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            const int selection = m_layoutChoice->GetSelection();
            assert(selection >= 0 && selection < static_cast<int>(NumFrameLayouts));
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::MapViewLayout, selection);
        }

        void ViewPreferencePane::OnBrightnessChanged(wxScrollEvent& event) {
            if (IsBeingDeleted()) return;

            const int value = m_brightnessSlider->GetValue();
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::Brightness, value / 40.0f);
        }
        
        void ViewPreferencePane::OnGridAlphaChanged(wxScrollEvent& event) {
            if (IsBeingDeleted()) return;

            const int value = m_gridAlphaSlider->GetValue();
            
            PreferenceManager& prefs = PreferenceManager::instance();
            const int max = m_gridAlphaSlider->GetMax();
            const float floatValue = static_cast<float>(value) / static_cast<float>(max);
            prefs.set(Preferences::GridAlpha, floatValue);
        }

        void ViewPreferencePane::OnBackgroundColorChanged(wxColourPickerEvent& event) {
            if (IsBeingDeleted()) return;

            const Color value(fromWxColor(event.GetColour()), 1.0f);
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::BackgroundColor, value);
        }

        void ViewPreferencePane::OnShowAxesChanged(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            
            const bool value = event.IsChecked();
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::ShowAxes, value);
        }

        void ViewPreferencePane::OnTextureModeChanged(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            const int selection = m_textureModeChoice->GetSelection();
            assert(selection >= 0);
            
            const size_t index = static_cast<size_t>(selection);
            assert(index < NumTextureModes);
            const int minFilter = TextureModes[index].minFilter;
            const int magFilter = TextureModes[index].magFilter;
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(Preferences::TextureMinFilter, minFilter);
            prefs.set(Preferences::TextureMagFilter, magFilter);
        }

        void ViewPreferencePane::OnTextureBrowserIconSizeChanged(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

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
            
            TitleBar* viewPrefsTitle = new TitleBar(viewBox, "Map Views");
            
            wxString layoutNames[NumFrameLayouts];
            layoutNames[0] = "One Pane";
            layoutNames[1] = "Two Panes";
            layoutNames[2] = "Three Panes";
            layoutNames[3] = "Four Panes";
            
            wxStaticText* layoutLabel = new wxStaticText(viewBox, wxID_ANY, "Layout");
            m_layoutChoice = new wxChoice(viewBox, wxID_ANY, wxDefaultPosition, wxDefaultSize, NumFrameLayouts, layoutNames);
            
            wxStaticText* brightnessLabel = new wxStaticText(viewBox, wxID_ANY, "Brightness");
            m_brightnessSlider = new wxSlider(viewBox, wxID_ANY, 50, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_BOTTOM);
            wxStaticText* gridLabel = new wxStaticText(viewBox, wxID_ANY, "Grid");
            m_gridAlphaSlider = new wxSlider(viewBox, wxID_ANY, 50, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_BOTTOM);
            
            wxStaticText* backgroundColorLabel = new wxStaticText(viewBox, wxID_ANY, "Background Color");
            m_backgroundColorPicker = new wxColourPickerCtrl(viewBox, wxID_ANY);
            
            wxStaticText* axesLabel = new wxStaticText(viewBox, wxID_ANY, "Coordinate System");
            m_showAxes = new wxCheckBox(viewBox, wxID_ANY, "Show Axes");
            
            wxString textureModeNames[NumTextureModes];
            for (size_t i = 0; i < NumTextureModes; ++i)
                textureModeNames[i] = TextureModes[i].name;
            wxStaticText* textureModeLabel = new wxStaticText(viewBox, wxID_ANY, "Texture Mode");
            m_textureModeChoice = new wxChoice(viewBox, wxID_ANY, wxDefaultPosition, wxDefaultSize, NumTextureModes, textureModeNames);

            TitleBar* textureBrowserPrefsTitle = new TitleBar(viewBox, "Texture Browser");

            wxStaticText* textureBrowserIconSizeLabel = new wxStaticText(viewBox, wxID_ANY, "Icon Size");
            wxString iconSizes[7] = {"25%", "50%", "100%", "150%", "200%", "250%", "300%"};
            m_textureBrowserIconSizeChoice = new wxChoice(viewBox, wxID_ANY, wxDefaultPosition, wxDefaultSize, 7, iconSizes);
            m_textureBrowserIconSizeChoice->SetToolTip("Sets the icon size in the texture browser.");
            
            
            const int HMargin           = LayoutConstants::WideHMargin;
            const int LMargin           = LayoutConstants::WideVMargin;
            const int HeaderFlags       = wxLEFT;
            const int LabelFlags        = wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxLEFT;
            const int SliderFlags       = wxEXPAND | wxRIGHT;
            const int ChoiceFlags       = wxRIGHT;
            const int CheckBoxFlags     = wxLeft;
            const int ColorPickerFlags  = wxRIGHT;
            const int LineFlags         = wxEXPAND | wxTOP;
            
            int r = 0;
            
            wxGridBagSizer* sizer = new wxGridBagSizer(LayoutConstants::NarrowVMargin, LayoutConstants::WideHMargin);
            sizer->Add(viewPrefsTitle,                    wxGBPosition( r, 0), wxGBSpan(1,2), HeaderFlags, HMargin);
            ++r;
            
            sizer->Add(layoutLabel,                         wxGBPosition( r, 0), wxDefaultSpan, LabelFlags, HMargin);
            sizer->Add(m_layoutChoice,                      wxGBPosition( r, 1), wxDefaultSpan, ChoiceFlags, HMargin);
            ++r;
            
            sizer->Add(brightnessLabel,                     wxGBPosition( r, 0), wxDefaultSpan, LabelFlags, HMargin);
            sizer->Add(m_brightnessSlider,                  wxGBPosition( r, 1), wxDefaultSpan, SliderFlags, HMargin);
            ++r;

            sizer->Add(gridLabel,                           wxGBPosition( r, 0), wxDefaultSpan, LabelFlags, HMargin);
            sizer->Add(m_gridAlphaSlider,                   wxGBPosition( r, 1), wxDefaultSpan, SliderFlags, HMargin);
            ++r;
            
            sizer->Add(axesLabel,                           wxGBPosition( r, 0), wxDefaultSpan, LabelFlags, HMargin);
            sizer->Add(m_showAxes,                          wxGBPosition( r, 1), wxDefaultSpan, CheckBoxFlags, HMargin);
            ++r;
            
            sizer->Add(backgroundColorLabel,                wxGBPosition( r, 0), wxDefaultSpan, LabelFlags, HMargin);
            sizer->Add(m_backgroundColorPicker,             wxGBPosition( r, 1), wxDefaultSpan, ColorPickerFlags, HMargin);
            ++r;
            
            sizer->Add(textureModeLabel,                    wxGBPosition( r, 0), wxDefaultSpan, LabelFlags, HMargin);
            sizer->Add(m_textureModeChoice,                 wxGBPosition( r, 1), wxDefaultSpan, ChoiceFlags, HMargin);
            ++r;

            sizer->Add(0, LayoutConstants::ChoiceSizeDelta, wxGBPosition( r, 0), wxGBSpan(1,2));
            ++r;

            sizer->Add(new BorderLine(viewBox),             wxGBPosition( r, 0), wxGBSpan(1,2), LineFlags, LMargin);
            ++r;
            
            sizer->Add(textureBrowserPrefsTitle,            wxGBPosition( r, 0), wxGBSpan(1,2), HeaderFlags, HMargin);
            ++r;

            sizer->Add(textureBrowserIconSizeLabel,         wxGBPosition( r, 0), wxDefaultSpan, LabelFlags, HMargin);
            sizer->Add(m_textureBrowserIconSizeChoice,      wxGBPosition( r, 1), wxDefaultSpan, ChoiceFlags, HMargin);
            ++r;

            sizer->Add(0, LayoutConstants::ChoiceSizeDelta, wxGBPosition( r, 0), wxGBSpan(1,2));
            
            sizer->AddGrowableCol(1);
            sizer->SetMinSize(500, wxDefaultCoord);
            viewBox->SetSizer(sizer);
            return viewBox;
        }
        
        void ViewPreferencePane::bindEvents() {
            m_layoutChoice->Bind(wxEVT_CHOICE, &ViewPreferencePane::OnLayoutChanged, this);
            
            bindSliderEvents(m_brightnessSlider, &ViewPreferencePane::OnBrightnessChanged, this);
            bindSliderEvents(m_gridAlphaSlider, &ViewPreferencePane::OnGridAlphaChanged, this);
            
            m_backgroundColorPicker->Bind(wxEVT_COLOURPICKER_CHANGED, &ViewPreferencePane::OnBackgroundColorChanged, this);
            m_showAxes->Bind(wxEVT_CHECKBOX, &ViewPreferencePane::OnShowAxesChanged, this);
            
            m_textureModeChoice->Bind(wxEVT_CHOICE, &ViewPreferencePane::OnTextureModeChanged, this);
            m_textureBrowserIconSizeChoice->Bind(wxEVT_CHOICE, &ViewPreferencePane::OnTextureBrowserIconSizeChanged, this);
        }

        bool ViewPreferencePane::doCanResetToDefaults() {
            return true;
        }
        
        void ViewPreferencePane::doResetToDefaults() {
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.resetToDefault(Preferences::Brightness);
            prefs.resetToDefault(Preferences::GridAlpha);
            prefs.resetToDefault(Preferences::BackgroundColor);
            prefs.resetToDefault(Preferences::ShowAxes);
            prefs.resetToDefault(Preferences::TextureMinFilter);
            prefs.resetToDefault(Preferences::TextureMagFilter);
            prefs.resetToDefault(Preferences::TextureBrowserIconSize);
        }

        void ViewPreferencePane::doUpdateControls() {
            m_layoutChoice->SetSelection(pref(Preferences::MapViewLayout));
            
            m_brightnessSlider->SetValue(static_cast<int>(pref(Preferences::Brightness) * 40.0f));
            m_gridAlphaSlider->SetValue(static_cast<int>(pref(Preferences::GridAlpha) * m_gridAlphaSlider->GetMax()));
            
            m_backgroundColorPicker->SetColour(toWxColor(pref(Preferences::BackgroundColor)));
            m_showAxes->SetValue(pref(Preferences::ShowAxes));
            
            const size_t textureModeIndex = findTextureMode(pref(Preferences::TextureMinFilter), pref(Preferences::TextureMagFilter));
            assert(textureModeIndex < NumTextureModes);
            m_textureModeChoice->SetSelection(static_cast<int>(textureModeIndex));
            
            const float textureBrowserIconSize = pref(Preferences::TextureBrowserIconSize);
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

        size_t ViewPreferencePane::findTextureMode(const int minFilter, const int magFilter) const {
            for (size_t i = 0; i < NumTextureModes; ++i)
                if (TextureModes[i].minFilter == minFilter &&
                    TextureModes[i].magFilter == magFilter)
                    return i;
            return NumTextureModes;
        }
	}
}
