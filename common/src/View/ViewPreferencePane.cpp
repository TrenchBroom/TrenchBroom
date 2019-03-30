/*
 Copyright (C) 2010-2017 Kristian Duske

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
#include <wx/combobox.h>
#include <wx/gbsizer.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <QLabel>
#include <wx/layout.h>
#include <wx/valnum.h>

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
            TextureMode(GL_NEAREST_MIPMAP_LINEAR,   GL_NEAREST, "Nearest (mipmapped, interpolated)"),
            TextureMode(GL_LINEAR,                  GL_LINEAR,  "Linear"),
            TextureMode(GL_LINEAR_MIPMAP_NEAREST,   GL_LINEAR,  "Linear (mipmapped)"),
            TextureMode(GL_LINEAR_MIPMAP_LINEAR,    GL_LINEAR,  "Linear (mipmapped, interpolated")
        };

        static const size_t NumFrameLayouts = 4;

        ViewPreferencePane::ViewPreferencePane(QWidget* parent) :
        PreferencePane(parent) {
            createGui();
            bindEvents();
        }


        void ViewPreferencePane::OnLayoutChanged() {


            const auto selection = m_layoutChoice->GetSelection();
            assert(selection >= 0 && selection < static_cast<int>(NumFrameLayouts));

            auto& prefs = PreferenceManager::instance();
            prefs.set(Preferences::MapViewLayout, selection);
        }

        void ViewPreferencePane::OnBrightnessChanged(wxScrollEvent& event) {


            const auto value = m_brightnessSlider->GetValue();

            auto& prefs = PreferenceManager::instance();
            prefs.set(Preferences::Brightness, value / 40.0f);
        }

        void ViewPreferencePane::OnGridAlphaChanged(wxScrollEvent& event) {


            const auto value = m_gridAlphaSlider->GetValue();

            auto& prefs = PreferenceManager::instance();
            const auto max = m_gridAlphaSlider->GetMax();
            const auto floatValue = float(value) / float(max);
            prefs.set(Preferences::GridAlpha, floatValue);
        }

        void ViewPreferencePane::OnFovChanged(wxScrollEvent& event) {


            const auto value = m_fovSlider->GetValue();

            auto& prefs = PreferenceManager::instance();
            prefs.set(Preferences::CameraFov, float(value));
        }

        void ViewPreferencePane::OnShowAxesChanged() {


            const auto value = event.IsChecked();

            auto& prefs = PreferenceManager::instance();
            prefs.set(Preferences::ShowAxes, value);
        }

        void ViewPreferencePane::OnTextureModeChanged() {


            const auto selection = m_textureModeChoice->GetSelection();
            assert(selection >= 0);

            const auto index = static_cast<size_t>(selection);
            assert(index < NumTextureModes);
            const auto minFilter = TextureModes[index].minFilter;
            const auto magFilter = TextureModes[index].magFilter;

            auto& prefs = PreferenceManager::instance();
            prefs.set(Preferences::TextureMinFilter, minFilter);
            prefs.set(Preferences::TextureMagFilter, magFilter);
        }

        void ViewPreferencePane::OnBackgroundColorChanged(wxColourPickerEvent& event) {


            const auto value = Color(fromWxColor(event.GetColour()), 1.0f);

            auto& prefs = PreferenceManager::instance();
            prefs.set(Preferences::BackgroundColor, value);
        }

        void ViewPreferencePane::OnGridColorChanged(wxColourPickerEvent& event) {


            const auto value = Color(fromWxColor(event.GetColour()), 1.0f);

            auto& prefs = PreferenceManager::instance();
            prefs.set(Preferences::GridColor2D, value);
        }

        void ViewPreferencePane::OnEdgeColorChanged(wxColourPickerEvent& event) {


            const auto value = Color(fromWxColor(event.GetColour()), 1.0f);

            auto& prefs = PreferenceManager::instance();
            prefs.set(Preferences::EdgeColor, value);
        }

        void ViewPreferencePane::OnTextureBrowserIconSizeChanged() {


            auto& prefs = PreferenceManager::instance();

            const auto selection = m_textureBrowserIconSizeChoice->GetSelection();
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

        void ViewPreferencePane::OnFontPrefsRendererFontSizeChanged() {
            if(IsBeingDeleted()) {
                return;
            }

            auto& prefs = PreferenceManager::instance();
            auto str = m_fontPrefsRendererFontSizeCombo->GetValue();
            prefs.set(Preferences::RendererFontSize, wxAtoi(str));
        }

        void ViewPreferencePane::createGui() {
            auto* viewPreferences = createViewPreferences();

            auto* sizer = new QVBoxLayout();
            sizer->addSpacing(LayoutConstants::NarrowVMargin);
            sizer->addWidget(viewPreferences, 1, wxEXPAND);
            sizer->addSpacing(LayoutConstants::WideVMargin);

            SetMinSize(sizer->GetMinSize());
            setLayout(sizer);
        }

        QWidget* ViewPreferencePane::createViewPreferences() {
            auto* viewBox = new QWidget(this);

            auto* viewPrefsHeader = new QLabel(viewBox, wxID_ANY, "Map Views");
            viewPrefsHeader->SetFont(viewPrefsHeader->GetFont().Bold());

            QString layoutNames[NumFrameLayouts];
            layoutNames[0] = "One Pane";
            layoutNames[1] = "Two Panes";
            layoutNames[2] = "Three Panes";
            layoutNames[3] = "Four Panes";

            auto* layoutLabel = new QLabel(viewBox, wxID_ANY, "Layout");
            m_layoutChoice = new wxChoice(viewBox, wxID_ANY, wxDefaultPosition, wxDefaultSize, NumFrameLayouts, layoutNames);

            auto* brightnessLabel = new QLabel(viewBox, wxID_ANY, "Brightness");
            m_brightnessSlider = new wxSlider(viewBox, wxID_ANY, 50, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_BOTTOM | wxSL_VALUE_LABEL);
            auto* gridLabel = new QLabel(viewBox, wxID_ANY, "Grid");
            m_gridAlphaSlider = new wxSlider(viewBox, wxID_ANY, 50, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_BOTTOM | wxSL_VALUE_LABEL);
            auto* fovLabel = new QLabel(viewBox, wxID_ANY, "Field of Vision");
            m_fovSlider = new wxSlider(viewBox, wxID_ANY, 90, 50, 150, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_BOTTOM | wxSL_VALUE_LABEL);

            auto* axesLabel = new QLabel(viewBox, wxID_ANY, "Coordinate System");
            m_showAxes = new wxCheckBox(viewBox, wxID_ANY, "Show Axes");

            QString textureModeNames[NumTextureModes];
            for (size_t i = 0; i < NumTextureModes; ++i) {
                textureModeNames[i] = TextureModes[i].name;
            }
            auto* textureModeLabel = new QLabel(viewBox, wxID_ANY, "Texture Mode");
            m_textureModeChoice = new wxChoice(viewBox, wxID_ANY, wxDefaultPosition, wxDefaultSize, NumTextureModes, textureModeNames);



            auto* colorPrefsHeader = new QLabel(viewBox, wxID_ANY, "Colors");
            colorPrefsHeader->SetFont(colorPrefsHeader->GetFont().Bold());

            auto* backgroundColorLabel = new QLabel(viewBox, wxID_ANY, "Background");
            m_backgroundColorPicker = new wxColourPickerCtrl(viewBox, wxID_ANY);

            auto* gridColorLabel = new QLabel(viewBox, wxID_ANY, "Grid");
            m_gridColorPicker = new wxColourPickerCtrl(viewBox, wxID_ANY);

            auto* edgeColorLabel = new QLabel(viewBox, wxID_ANY, "Edges");
            m_edgeColorPicker = new wxColourPickerCtrl(viewBox, wxID_ANY);



            auto* textureBrowserPrefsHeader = new QLabel(viewBox, wxID_ANY, "Texture Browser");
            textureBrowserPrefsHeader->SetFont(textureBrowserPrefsHeader->GetFont().Bold());

            auto* textureBrowserIconSizeLabel = new QLabel(viewBox, wxID_ANY, "Icon Size");
            QString iconSizes[7] = {"25%", "50%", "100%", "150%", "200%", "250%", "300%"};
            m_textureBrowserIconSizeChoice = new wxChoice(viewBox, wxID_ANY, wxDefaultPosition, wxDefaultSize, 7, iconSizes);
            m_textureBrowserIconSizeChoice->setToolTip("Sets the icon size in the texture browser.");



            auto* fontPrefsHeader = new QLabel(viewBox, wxID_ANY, "Fonts");
            fontPrefsHeader->SetFont(fontPrefsHeader->GetFont().Bold());

            wxIntegerValidator<unsigned int> ValidIntP;
            ValidIntP.SetRange(1, 96);
            auto* fontPrefsRendererFontSizeLabel = new QLabel(viewBox, wxID_ANY, "Renderer Font Size");
            std::vector<QString> rendererFontSizes {  "8",  "9", "10", "11", "12", "13", "14", "15", "16", "17", "18", "20", "22",
                                                      "24", "26", "28", "32", "36", "40", "48", "56", "64", "72" };
            m_fontPrefsRendererFontSizeCombo = new wxComboBox(viewBox, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,
                                                              static_cast<int>(rendererFontSizes.size()), rendererFontSizes.data(),
                                                              0, ValidIntP);
            m_fontPrefsRendererFontSizeCombo->setToolTip("Sets the font size for various labels in the 2D and 3D views.");



            const auto HMargin           = LayoutConstants::WideHMargin;
            const auto LMargin           = LayoutConstants::WideVMargin;
            const auto HeaderFlags       = wxLEFT;
            const auto LabelFlags        = wxALIGN_RIGHT | Qt::AlignVCenter | wxLEFT;
            const auto SliderFlags       = wxEXPAND | wxRIGHT;
            const auto ChoiceFlags       = wxRIGHT;
            const auto CheckBoxFlags     = wxLEFT;
            const auto ColorPickerFlags  = wxRIGHT;
            const auto LineFlags         = wxEXPAND | wxTOP;


            int r = 0;

            auto* sizer = new wxGridBagSizer(LayoutConstants::NarrowVMargin, LayoutConstants::WideHMargin);
            sizer->addWidget(viewPrefsHeader,                     wxGBPosition( r, 0), wxGBSpan(1,2), HeaderFlags, HMargin);
            ++r;

            sizer->addWidget(layoutLabel,                         wxGBPosition( r, 0), wxDefaultSpan, LabelFlags, HMargin);
            sizer->addWidget(m_layoutChoice,                      wxGBPosition( r, 1), wxDefaultSpan, ChoiceFlags, HMargin);
            ++r;

            sizer->addWidget(brightnessLabel,                     wxGBPosition( r, 0), wxDefaultSpan, LabelFlags, HMargin);
            sizer->addWidget(m_brightnessSlider,                  wxGBPosition( r, 1), wxDefaultSpan, SliderFlags, HMargin);
            ++r;
            sizer->addWidget(0, LMargin,                          wxGBPosition( r, 0), wxGBSpan(1,2));
            ++r;

            sizer->addWidget(gridLabel,                           wxGBPosition( r, 0), wxDefaultSpan, LabelFlags, HMargin);
            sizer->addWidget(m_gridAlphaSlider,                   wxGBPosition( r, 1), wxDefaultSpan, SliderFlags, HMargin);
            ++r;
            sizer->addWidget(0, LMargin,                          wxGBPosition( r, 0), wxGBSpan(1,2));
            ++r;

            sizer->addWidget(fovLabel,                            wxGBPosition( r, 0), wxDefaultSpan, LabelFlags, HMargin);
            sizer->addWidget(m_fovSlider,                         wxGBPosition( r, 1), wxDefaultSpan, SliderFlags, HMargin);
            ++r;
            sizer->addWidget(0, LMargin,                          wxGBPosition( r, 0), wxGBSpan(1,2));
            ++r;

            sizer->addWidget(axesLabel,                           wxGBPosition( r, 0), wxDefaultSpan, LabelFlags, HMargin);
            sizer->addWidget(m_showAxes,                          wxGBPosition( r, 1), wxDefaultSpan, CheckBoxFlags, HMargin);
            ++r;

            sizer->addWidget(textureModeLabel,                    wxGBPosition( r, 0), wxDefaultSpan, LabelFlags, HMargin);
            sizer->addWidget(m_textureModeChoice,                 wxGBPosition( r, 1), wxDefaultSpan, ChoiceFlags, HMargin);
            ++r;

            sizer->addWidget(0, LayoutConstants::ChoiceSizeDelta, wxGBPosition( r, 0), wxGBSpan(1,2));
            ++r;



            sizer->addWidget(new BorderLine(viewBox),             wxGBPosition( r, 0), wxGBSpan(1,2), LineFlags, LMargin);
            ++r;

            sizer->addWidget(colorPrefsHeader,                    wxGBPosition( r, 0), wxGBSpan(1,2), HeaderFlags, HMargin);
            ++r;

            sizer->addWidget(backgroundColorLabel,                wxGBPosition( r, 0), wxDefaultSpan, LabelFlags, HMargin);
            sizer->addWidget(m_backgroundColorPicker,             wxGBPosition( r, 1), wxDefaultSpan, ColorPickerFlags, HMargin);
            ++r;

            sizer->addWidget(gridColorLabel,                      wxGBPosition( r, 0), wxDefaultSpan, LabelFlags, HMargin);
            sizer->addWidget(m_gridColorPicker,                   wxGBPosition( r, 1), wxDefaultSpan, ColorPickerFlags, HMargin);
            ++r;

            sizer->addWidget(edgeColorLabel,                      wxGBPosition( r, 0), wxDefaultSpan, LabelFlags, HMargin);
            sizer->addWidget(m_edgeColorPicker,                   wxGBPosition( r, 1), wxDefaultSpan, ColorPickerFlags, HMargin);
            ++r;



            sizer->addWidget(new BorderLine(viewBox),             wxGBPosition( r, 0), wxGBSpan(1,2), LineFlags, LMargin);
            ++r;

            sizer->addWidget(textureBrowserPrefsHeader,           wxGBPosition( r, 0), wxGBSpan(1,2), HeaderFlags, HMargin);
            ++r;

            sizer->addWidget(textureBrowserIconSizeLabel,         wxGBPosition( r, 0), wxDefaultSpan, LabelFlags, HMargin);
            sizer->addWidget(m_textureBrowserIconSizeChoice,      wxGBPosition( r, 1), wxDefaultSpan, ChoiceFlags, HMargin);
            ++r;



            sizer->addWidget(new BorderLine(viewBox),             wxGBPosition( r, 0), wxGBSpan(1,2), LineFlags, LMargin);
            ++r;

            sizer->addWidget(fontPrefsHeader,                     wxGBPosition( r, 0), wxGBSpan(1,2), HeaderFlags, HMargin);
            ++r;

            sizer->addWidget(fontPrefsRendererFontSizeLabel,      wxGBPosition( r, 0), wxDefaultSpan, LabelFlags,  HMargin);
            sizer->addWidget(m_fontPrefsRendererFontSizeCombo,    wxGBPosition( r, 1), wxDefaultSpan, ChoiceFlags, HMargin);
            ++r;

            sizer->addWidget(0, LayoutConstants::ChoiceSizeDelta, wxGBPosition( r, 0), wxGBSpan(1,2));

            sizer->AddGrowableCol(1);
            sizer->SetMinSize(500, wxDefaultCoord);
            viewBox->setLayout(sizer);
            return viewBox;
        }

        void ViewPreferencePane::bindEvents() {
            m_layoutChoice->Bind(wxEVT_CHOICE, &ViewPreferencePane::OnLayoutChanged, this);

            bindSliderEvents(m_brightnessSlider, &ViewPreferencePane::OnBrightnessChanged, this);
            bindSliderEvents(m_gridAlphaSlider, &ViewPreferencePane::OnGridAlphaChanged, this);
            bindSliderEvents(m_fovSlider, &ViewPreferencePane::OnFovChanged, this);

            m_showAxes->Bind(wxEVT_CHECKBOX, &ViewPreferencePane::OnShowAxesChanged, this);

            m_backgroundColorPicker->Bind(wxEVT_COLOURPICKER_CHANGED, &ViewPreferencePane::OnBackgroundColorChanged, this);
            m_gridColorPicker->Bind(wxEVT_COLOURPICKER_CHANGED, &ViewPreferencePane::OnGridColorChanged, this);
            m_edgeColorPicker->Bind(wxEVT_COLOURPICKER_CHANGED, &ViewPreferencePane::OnEdgeColorChanged, this);

            m_textureModeChoice->Bind(wxEVT_CHOICE, &ViewPreferencePane::OnTextureModeChanged, this);
            m_textureBrowserIconSizeChoice->Bind(wxEVT_CHOICE, &ViewPreferencePane::OnTextureBrowserIconSizeChanged, this);

            m_fontPrefsRendererFontSizeCombo->Bind(wxEVT_COMBOBOX, &ViewPreferencePane::OnFontPrefsRendererFontSizeChanged, this);
            m_fontPrefsRendererFontSizeCombo->Bind(wxEVT_TEXT, &ViewPreferencePane::OnFontPrefsRendererFontSizeChanged, this);
        }

        bool ViewPreferencePane::doCanResetToDefaults() {
            return true;
        }

        void ViewPreferencePane::doResetToDefaults() {
            auto& prefs = PreferenceManager::instance();
            prefs.resetToDefault(Preferences::Brightness);
            prefs.resetToDefault(Preferences::GridAlpha);
            prefs.resetToDefault(Preferences::CameraFov);
            prefs.resetToDefault(Preferences::ShowAxes);
            prefs.resetToDefault(Preferences::TextureMinFilter);
            prefs.resetToDefault(Preferences::TextureMagFilter);
            prefs.resetToDefault(Preferences::BackgroundColor);
            prefs.resetToDefault(Preferences::GridColor2D);
            prefs.resetToDefault(Preferences::EdgeColor);
            prefs.resetToDefault(Preferences::TextureBrowserIconSize);
            prefs.resetToDefault(Preferences::RendererFontSize);
        }

        void ViewPreferencePane::doUpdateControls() {
            m_layoutChoice->SetSelection(pref(Preferences::MapViewLayout));

            m_brightnessSlider->SetValue(int(pref(Preferences::Brightness) * 40.0f));
            m_gridAlphaSlider->SetValue(int(pref(Preferences::GridAlpha) * m_gridAlphaSlider->GetMax()));
            m_fovSlider->SetValue(int(pref(Preferences::CameraFov)));

            const auto textureModeIndex = findTextureMode(pref(Preferences::TextureMinFilter), pref(Preferences::TextureMagFilter));
            assert(textureModeIndex < NumTextureModes);
            m_textureModeChoice->SetSelection(static_cast<int>(textureModeIndex));

            m_showAxes->SetValue(pref(Preferences::ShowAxes));

            m_backgroundColorPicker->SetColour(toWxColor(pref(Preferences::BackgroundColor)));
            m_gridColorPicker->SetColour(toWxColor(pref(Preferences::GridColor2D)));
            m_edgeColorPicker->SetColour(toWxColor(pref(Preferences::EdgeColor)));

            const auto textureBrowserIconSize = pref(Preferences::TextureBrowserIconSize);
            if (textureBrowserIconSize == 0.25f) {
                m_textureBrowserIconSizeChoice->SetSelection(0);
            } else if (textureBrowserIconSize == 0.5f) {
                m_textureBrowserIconSizeChoice->SetSelection(1);
            } else if (textureBrowserIconSize == 1.5f) {
                m_textureBrowserIconSizeChoice->SetSelection(3);
            } else if (textureBrowserIconSize == 2.0f) {
                m_textureBrowserIconSizeChoice->SetSelection(4);
            } else if (textureBrowserIconSize == 2.5f) {
                m_textureBrowserIconSizeChoice->SetSelection(5);
            } else if (textureBrowserIconSize == 3.0f) {
                m_textureBrowserIconSizeChoice->SetSelection(6);
            } else {
                m_textureBrowserIconSizeChoice->SetSelection(2);
            }

            m_fontPrefsRendererFontSizeCombo->SetValue(QString::Format(wxT("%i"), pref(Preferences::RendererFontSize)));
        }

        bool ViewPreferencePane::doValidate() {
            return true;
        }

        size_t ViewPreferencePane::findTextureMode(const int minFilter, const int magFilter) const {
            for (size_t i = 0; i < NumTextureModes; ++i) {
                if (TextureModes[i].minFilter == minFilter &&
                    TextureModes[i].magFilter == magFilter) {
                    return i;
                }
            }
            return NumTextureModes;
        }
	}
}
