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

#pragma once

#include "View/PreferencePane.h"

class QCheckBox;
class QComboBox;

namespace TrenchBroom {
    namespace View {
        class ColorButton;
        class SliderWithLabel;

        class ViewPreferencePane : public PreferencePane {
            Q_OBJECT
        private:
            QComboBox* m_layoutCombo;
            SliderWithLabel* m_brightnessSlider;
            SliderWithLabel* m_gridAlphaSlider;
            SliderWithLabel* m_fovSlider;
            QCheckBox* m_showAxes;
            QComboBox* m_textureModeCombo;
            ColorButton* m_backgroundColorButton;
            ColorButton* m_gridColorButton;
            ColorButton* m_edgeColorButton;
            QComboBox* m_themeCombo;
            QComboBox* m_textureBrowserIconSizeCombo;
            QComboBox* m_rendererFontSizeCombo;
        public:
            explicit ViewPreferencePane(QWidget* parent = nullptr);
       private:
            void createGui();
            QWidget* createViewPreferences();

            void bindEvents();

            bool doCanResetToDefaults() override;
            void doResetToDefaults() override;
            void doUpdateControls() override;
            bool doValidate() override;

            size_t findTextureMode(int minFilter, int magFilter) const;
            int findThemeIndex(const QString& theme);
        private slots:
            void layoutChanged(int index);
            void brightnessChanged(int value);
            void gridAlphaChanged(int value);
            void fovChanged(int value);
            void showAxesChanged(int state);
            void textureModeChanged(int index);
            void backgroundColorChanged(const QColor& color);
            void gridColorChanged(const QColor& color);
            void edgeColorChanged(const QColor& color);
            void themeChanged(int index);
            void textureBrowserIconSizeChanged(int index);
            void rendererFontSizeChanged(const QString& text);
        };
    }
}

#endif /* defined(TrenchBroom_ViewPreferencePane) */
