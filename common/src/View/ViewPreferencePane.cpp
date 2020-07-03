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

#include "PreferenceManager.h"
#include "Preferences.h"
#include "View/ColorButton.h"
#include "View/FormWithSectionsLayout.h"
#include "View/SliderWithLabel.h"
#include "View/ViewConstants.h"
#include "View/QtUtils.h"

#include "Renderer/GL.h"

#include <QtGlobal>
#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>

#include <array>
#include <string>

namespace TrenchBroom {
    namespace View {
        struct TextureMode {
            int minFilter;
            int magFilter;
            std::string name;

            TextureMode(const int i_minFilter, const int i_magFilter, const std::string& i_name) :
            minFilter(i_minFilter),
            magFilter(i_magFilter),
            name(i_name) {}
        };

        static const std::array<TextureMode, 6> TextureModes = {
            TextureMode(GL_NEAREST,                 GL_NEAREST, "Nearest"),
            TextureMode(GL_NEAREST_MIPMAP_NEAREST,  GL_NEAREST, "Nearest (mipmapped)"),
            TextureMode(GL_NEAREST_MIPMAP_LINEAR,   GL_NEAREST, "Nearest (mipmapped, interpolated)"),
            TextureMode(GL_LINEAR,                  GL_LINEAR,  "Linear"),
            TextureMode(GL_LINEAR_MIPMAP_NEAREST,   GL_LINEAR,  "Linear (mipmapped)"),
            TextureMode(GL_LINEAR_MIPMAP_LINEAR,    GL_LINEAR,  "Linear (mipmapped, interpolated")
        };

        ViewPreferencePane::ViewPreferencePane(QWidget* parent) :
        PreferencePane(parent) {
            createGui();
            bindEvents();
        }

        void ViewPreferencePane::createGui() {
            auto* viewPreferences = createViewPreferences();

            auto* layout = new QVBoxLayout();
            layout->setContentsMargins(QMargins());
            layout->setSpacing(0);

            layout->addSpacing(LayoutConstants::NarrowVMargin);
            layout->addWidget(viewPreferences, 1);
            layout->addSpacing(LayoutConstants::MediumVMargin);
            setLayout(layout);
        }

        QWidget* ViewPreferencePane::createViewPreferences() {
            auto* viewBox = new QWidget(this);

            auto* viewPrefsHeader = new QLabel("Map Views");
            makeEmphasized(viewPrefsHeader);

            m_themeCombo = new QComboBox();
            m_themeCombo->addItems({Preferences::systemTheme(), Preferences::darkTheme()});
            auto* themeInfo = new QLabel();
            themeInfo->setText(tr("Requires restart after changing"));
            makeInfo(themeInfo);
            auto* themeLayout = new QHBoxLayout();
            themeLayout->addWidget(m_themeCombo);
            themeLayout->addSpacing(LayoutConstants::NarrowHMargin);
            themeLayout->addWidget(themeInfo);
            themeLayout->setContentsMargins(0, 0, 0, 0);

            m_layoutCombo = new QComboBox();
            m_layoutCombo->setToolTip("Sets the layout of the editing views.");
            m_layoutCombo->addItem("One Pane");
            m_layoutCombo->addItem("Two Panes");
            m_layoutCombo->addItem("Three Panes");
            m_layoutCombo->addItem("Four Panes");

            m_brightnessSlider = new SliderWithLabel(0, 200);
            m_brightnessSlider->setMaximumWidth(400);
            m_brightnessSlider->setToolTip("Sets the brightness for textures and model skins in the 3D editing view.");
            m_gridAlphaSlider = new SliderWithLabel(0, 100);
            m_gridAlphaSlider->setMaximumWidth(400);
            m_gridAlphaSlider->setToolTip("Sets the visibility of the grid lines in the 3D editing view.");
            m_fovSlider = new SliderWithLabel(50, 150);
            m_fovSlider->setMaximumWidth(400);
            m_fovSlider->setToolTip("Sets the field of vision in the 3D editing view.");

            m_showAxes = new QCheckBox();
            m_showAxes->setToolTip("Toggle showing the coordinate system axes in the 3D editing view.");

            m_textureModeCombo = new QComboBox();
            m_textureModeCombo->setToolTip("Sets the texture filtering mode in the editing views.");
            for (const auto& textureMode : TextureModes) {
                m_textureModeCombo->addItem(QString::fromStdString(textureMode.name));
            }

            m_backgroundColorButton = new ColorButton();
            m_backgroundColorButton->setToolTip("Sets the background color of the editing views.");
            m_gridColorButton = new ColorButton();
            m_gridColorButton->setToolTip("Sets the color of the grid lines in the editing views.");
            m_edgeColorButton = new ColorButton();
            m_edgeColorButton->setToolTip("Sets the color of brush edges in the editing views.");

            m_textureBrowserIconSizeCombo = new QComboBox();
            m_textureBrowserIconSizeCombo->addItem("25%");
            m_textureBrowserIconSizeCombo->addItem("50%");
            m_textureBrowserIconSizeCombo->addItem("100%");
            m_textureBrowserIconSizeCombo->addItem("150%");
            m_textureBrowserIconSizeCombo->addItem("200%");
            m_textureBrowserIconSizeCombo->addItem("250%");
            m_textureBrowserIconSizeCombo->addItem("300%");
            m_textureBrowserIconSizeCombo->setToolTip("Sets the icon size in the texture browser.");

            m_rendererFontSizeCombo = new QComboBox();
            m_rendererFontSizeCombo->setEditable(true);
            m_rendererFontSizeCombo->setToolTip("Sets the font size for various labels in the editing views.");
            m_rendererFontSizeCombo->addItems({ "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "22", "24", "26", "28", "32", "36", "40", "48", "56", "64", "72" });
            m_rendererFontSizeCombo->setValidator(new QIntValidator(1, 96));

            auto* layout = new FormWithSectionsLayout();
            layout->setContentsMargins(0, LayoutConstants::MediumVMargin, 0, 0);
            layout->setVerticalSpacing(2);
            // override the default to make the sliders take up maximum width
            layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

            layout->addSection("User Interface");
            layout->addRow("Theme", themeLayout);

            layout->addSection("Map Views");
            layout->addRow("Layout", m_layoutCombo);
            layout->addRow("Brightness", m_brightnessSlider);
            layout->addRow("Grid", m_gridAlphaSlider);
            layout->addRow("FOV", m_fovSlider);
            layout->addRow("Show axes", m_showAxes);
            layout->addRow("Texture mode", m_textureModeCombo);

            layout->addSection("Colors");
            layout->addRow("Background", m_backgroundColorButton);
            layout->addRow("Grid", m_gridColorButton);
            layout->addRow("Edges", m_edgeColorButton);

            layout->addSection("Texture Browser");
            layout->addRow("Icon size", m_textureBrowserIconSizeCombo);

            layout->addSection("Fonts");
            layout->addRow("Renderer Font Size", m_rendererFontSizeCombo);

            viewBox->setMinimumWidth(400);
            viewBox->setLayout(layout);

            return viewBox;
        }

        void ViewPreferencePane::bindEvents() {
            connect(m_layoutCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ViewPreferencePane::layoutChanged);
            connect(m_brightnessSlider, &SliderWithLabel::valueChanged, this, &ViewPreferencePane::brightnessChanged);
            connect(m_gridAlphaSlider, &SliderWithLabel::valueChanged, this, &ViewPreferencePane::gridAlphaChanged);
            connect(m_fovSlider, &SliderWithLabel::valueChanged, this, &ViewPreferencePane::fovChanged);
            connect(m_showAxes, &QCheckBox::stateChanged, this, &ViewPreferencePane::showAxesChanged);
            connect(m_backgroundColorButton, &ColorButton::colorChanged, this, &ViewPreferencePane::backgroundColorChanged);
            connect(m_gridColorButton, &ColorButton::colorChanged, this, &ViewPreferencePane::gridColorChanged);
            connect(m_edgeColorButton, &ColorButton::colorChanged, this, &ViewPreferencePane::edgeColorChanged);
            connect(m_themeCombo, QOverload<int>::of(&QComboBox::activated), this, &ViewPreferencePane::themeChanged);
            connect(m_textureModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ViewPreferencePane::textureModeChanged);
            connect(m_textureBrowserIconSizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ViewPreferencePane::textureBrowserIconSizeChanged);
            connect(m_rendererFontSizeCombo, &QComboBox::currentTextChanged, this, &ViewPreferencePane::rendererFontSizeChanged);
        }

        bool ViewPreferencePane::doCanResetToDefaults() {
            return true;
        }

        void ViewPreferencePane::doResetToDefaults() {
            auto& prefs = PreferenceManager::instance();
            prefs.resetToDefault(Preferences::MapViewLayout);
            prefs.resetToDefault(Preferences::Brightness);
            prefs.resetToDefault(Preferences::GridAlpha);
            prefs.resetToDefault(Preferences::CameraFov);
            prefs.resetToDefault(Preferences::ShowAxes);
            prefs.resetToDefault(Preferences::TextureMinFilter);
            prefs.resetToDefault(Preferences::TextureMagFilter);
            prefs.resetToDefault(Preferences::BackgroundColor);
            prefs.resetToDefault(Preferences::GridColor2D);
            prefs.resetToDefault(Preferences::EdgeColor);
            prefs.resetToDefault(Preferences::Theme);
            prefs.resetToDefault(Preferences::TextureBrowserIconSize);
            prefs.resetToDefault(Preferences::RendererFontSize);
        }

        void ViewPreferencePane::doUpdateControls() {
            m_layoutCombo->setCurrentIndex(pref(Preferences::MapViewLayout));
            m_brightnessSlider->setValue(int(pref(Preferences::Brightness) * 100.0f));
            m_gridAlphaSlider->setRatio(pref(Preferences::GridAlpha));
            m_fovSlider->setValue(int(pref(Preferences::CameraFov)));

            const auto textureModeIndex = findTextureMode(pref(Preferences::TextureMinFilter), pref(Preferences::TextureMagFilter));
            m_textureModeCombo->setCurrentIndex(int(textureModeIndex));

            m_showAxes->setChecked(pref(Preferences::ShowAxes));

            m_backgroundColorButton->setColor(toQColor(pref(Preferences::BackgroundColor)));
            m_gridColorButton->setColor(toQColor(pref(Preferences::GridColor2D)));
            m_edgeColorButton->setColor(toQColor(pref(Preferences::EdgeColor)));
            m_themeCombo->setCurrentIndex(findThemeIndex(pref(Preferences::Theme)));

            const auto textureBrowserIconSize = pref(Preferences::TextureBrowserIconSize);
            if (textureBrowserIconSize == 0.25f) {
                m_textureBrowserIconSizeCombo->setCurrentIndex(0);
            } else if (textureBrowserIconSize == 0.5f) {
                m_textureBrowserIconSizeCombo->setCurrentIndex(1);
            } else if (textureBrowserIconSize == 1.5f) {
                m_textureBrowserIconSizeCombo->setCurrentIndex(3);
            } else if (textureBrowserIconSize == 2.0f) {
                m_textureBrowserIconSizeCombo->setCurrentIndex(4);
            } else if (textureBrowserIconSize == 2.5f) {
                m_textureBrowserIconSizeCombo->setCurrentIndex(5);
            } else if (textureBrowserIconSize == 3.0f) {
                m_textureBrowserIconSizeCombo->setCurrentIndex(6);
            } else {
                m_textureBrowserIconSizeCombo->setCurrentIndex(2);
            }

            m_rendererFontSizeCombo->setCurrentText(QString::asprintf("%i", pref(Preferences::RendererFontSize)));
        }

        bool ViewPreferencePane::doValidate() {
            return true;
        }

        size_t ViewPreferencePane::findTextureMode(const int minFilter, const int magFilter) const {
            for (size_t i = 0; i < TextureModes.size(); ++i) {
                if (TextureModes[i].minFilter == minFilter &&
                    TextureModes[i].magFilter == magFilter) {
                    return i;
                }
            }
            return TextureModes.size();
        }

        int ViewPreferencePane::findThemeIndex(const QString& theme) {
            for (int i = 0; i < m_themeCombo->count(); ++i) {
                if (m_themeCombo->itemText(i) == theme) {
                    return i;
                }
            }
            return 0;
        }

        void ViewPreferencePane::layoutChanged(const int index) {
            assert(index >= 0 && index < 4);

            auto& prefs = PreferenceManager::instance();
            prefs.set(Preferences::MapViewLayout, index);
        }

        void ViewPreferencePane::brightnessChanged(const int value) {
            auto& prefs = PreferenceManager::instance();
            prefs.set(Preferences::Brightness, static_cast<float>(value) / 100.0f);
        }

        void ViewPreferencePane::gridAlphaChanged(const int /* value */) {
            const auto ratio = m_gridAlphaSlider->ratio();
            auto& prefs = PreferenceManager::instance();
            prefs.set(Preferences::GridAlpha, ratio);
        }

        void ViewPreferencePane::fovChanged(const int value) {
            auto& prefs = PreferenceManager::instance();
            prefs.set(Preferences::CameraFov, float(value));
        }

        void ViewPreferencePane::showAxesChanged(const int state) {
            const auto value = state == Qt::Checked;
            auto& prefs = PreferenceManager::instance();
            prefs.set(Preferences::ShowAxes, value);
        }

        void ViewPreferencePane::textureModeChanged(const int value) {
            const auto index = static_cast<size_t>(value);
            assert(index < TextureModes.size());
            const auto minFilter = TextureModes[index].minFilter;
            const auto magFilter = TextureModes[index].magFilter;

            auto& prefs = PreferenceManager::instance();
            prefs.set(Preferences::TextureMinFilter, minFilter);
            prefs.set(Preferences::TextureMagFilter, magFilter);
        }

        void ViewPreferencePane::backgroundColorChanged(const QColor& color) {
            const auto value = Color(fromQColor(color), 1.0f);
            auto& prefs = PreferenceManager::instance();
            prefs.set(Preferences::BackgroundColor, value);
        }

        void ViewPreferencePane::gridColorChanged(const QColor& color) {
            const auto value = Color(fromQColor(color), 1.0f);
            auto& prefs = PreferenceManager::instance();
            prefs.set(Preferences::GridColor2D, value);
        }

        void ViewPreferencePane::edgeColorChanged(const QColor& color) {
            const auto value = Color(fromQColor(color), 1.0f);
            auto& prefs = PreferenceManager::instance();
            prefs.set(Preferences::EdgeColor, value);
        }

        void ViewPreferencePane::themeChanged(int /*index*/) {
            auto& prefs = PreferenceManager::instance();
            prefs.set(Preferences::Theme, m_themeCombo->currentText());
        }

        void ViewPreferencePane::textureBrowserIconSizeChanged(const int index) {
            auto& prefs = PreferenceManager::instance();

            switch (index) {
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

        void ViewPreferencePane::rendererFontSizeChanged(const QString& str) {
            bool ok;
            const auto value = str.toInt(&ok);
            if (ok) {
                auto& prefs = PreferenceManager::instance();
                prefs.set(Preferences::RendererFontSize, value);
            }
        }
    }
}
