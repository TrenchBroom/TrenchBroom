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

#ifndef TrenchBroom_ViewPreferencePane
#define TrenchBroom_ViewPreferencePane

#include "View/PreferencePane.h"

class wxColourPickerCtrl;
class wxColourPickerEvent;
class wxCheckBox;
class wxChoice;
class wxComboBox;
class wxSlider;
class wxComboBox;

namespace TrenchBroom {
    namespace View {
        class ViewPreferencePane : public PreferencePane {
        private:
            wxChoice* m_layoutChoice;
            wxSlider* m_brightnessSlider;
            wxSlider* m_gridAlphaSlider;
            wxSlider* m_fovSlider;
            wxCheckBox* m_showAxes;
            wxChoice* m_textureModeChoice;
            wxColourPickerCtrl* m_backgroundColorPicker;
            wxColourPickerCtrl* m_gridColorPicker;
            wxColourPickerCtrl* m_edgeColorPicker;
            wxChoice* m_textureBrowserIconSizeChoice;
            wxComboBox* m_fontPrefsRendererFontSizeCombo;
        public:
            ViewPreferencePane(wxWindow* parent);

            void OnLayoutChanged(wxCommandEvent& event);
            void OnBrightnessChanged(wxScrollEvent& event);
            void OnGridAlphaChanged(wxScrollEvent& event);
            void OnFovChanged(wxScrollEvent& event);
            void OnShowAxesChanged(wxCommandEvent& event);
            void OnTextureModeChanged(wxCommandEvent& event);
            void OnBackgroundColorChanged(wxColourPickerEvent& event);
            void OnGridColorChanged(wxColourPickerEvent& event);
            void OnEdgeColorChanged(wxColourPickerEvent& event);
            void OnTextureBrowserIconSizeChanged(wxCommandEvent& event);
            void OnFontPrefsRendererFontSizeChanged(wxCommandEvent& event);
       private:
            void createGui();
            wxWindow* createViewPreferences();

            void bindEvents();

            bool doCanResetToDefaults() override;
            void doResetToDefaults() override;
            void doUpdateControls(MapDocumentWPtr document) override;
            bool doValidate() override;

            size_t findTextureMode(int minFilter, int magFilter) const;
        };
    }
}

#endif /* defined(TrenchBroom_ViewPreferencePane) */
