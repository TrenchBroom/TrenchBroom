/*
 Copyright (C) 2010 Kristian Duske

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

#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QtGlobal>

#include "PreferenceManager.h"
#include "Preferences.h"
#include "render/GL.h"
#include "ui/FormWithSectionsLayout.h"
#include "ui/QtUtils.h"
#include "ui/SliderWithLabel.h"
#include "ui/ViewConstants.h"

#include "kdl/range_utils.h"

#include "vm/scalar.h"

#include <array>
#include <string>

namespace tb::ui
{
namespace
{
struct FilterMode
{
  int minFilter;
  int magFilter;
  std::string name;
};

const auto FilterModes = std::array<FilterMode, 6>{
  FilterMode{GL_NEAREST, GL_NEAREST, "Nearest"},
  FilterMode{GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST, "Nearest (mipmapped)"},
  FilterMode{GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST, "Nearest (mipmapped, interpolated)"},
  FilterMode{GL_LINEAR, GL_LINEAR, "Linear"},
  FilterMode{GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR, "Linear (mipmapped)"},
  FilterMode{GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, "Linear (mipmapped, interpolated)"},
};

constexpr int brightnessToUI(const float value)
{
  return int(vm::round(100.0f * (value - 1.0f)));
}

constexpr float brightnessFromUI(const int value)
{
  return (float(value) / 100.0f) + 1.0f;
}

static_assert(0 == brightnessToUI(brightnessFromUI(0)));

} // namespace

ViewPreferencePane::ViewPreferencePane(QWidget* parent)
  : PreferencePane{parent}
{
  createGui();
  bindEvents();
}

void ViewPreferencePane::createGui()
{
  auto* viewPreferences = createViewPreferences();

  auto* layout = new QVBoxLayout{};
  layout->setContentsMargins(QMargins{});
  layout->setSpacing(0);

  layout->addSpacing(LayoutConstants::NarrowVMargin);
  layout->addWidget(viewPreferences, 1);
  layout->addSpacing(LayoutConstants::MediumVMargin);
  setLayout(layout);
}

QWidget* ViewPreferencePane::createViewPreferences()
{
  auto* viewBox = new QWidget{this};

  auto* viewPrefsHeader = new QLabel{"Map Views"};
  makeEmphasized(viewPrefsHeader);

  m_themeCombo = new QComboBox{};
  m_themeCombo->addItems({Preferences::systemTheme(), Preferences::darkTheme()});
  auto* themeInfo = new QLabel{};
  themeInfo->setText(tr("Requires restart after changing"));
  makeInfo(themeInfo);
  auto* themeLayout = new QHBoxLayout{};
  themeLayout->addWidget(m_themeCombo);
  themeLayout->addSpacing(LayoutConstants::NarrowHMargin);
  themeLayout->addWidget(themeInfo);
  themeLayout->setContentsMargins(0, 0, 0, 0);

  m_layoutCombo = new QComboBox{};
  m_layoutCombo->setToolTip("Sets the layout of the editing views.");
  m_layoutCombo->addItem("One Pane");
  m_layoutCombo->addItem("Two Panes");
  m_layoutCombo->addItem("Three Panes");
  m_layoutCombo->addItem("Four Panes");

  m_link2dCameras = new QCheckBox{"Sync 2D views"};
  m_link2dCameras->setToolTip("All 2D views pan and zoom together.");

  auto* viewLayoutLayout = new QHBoxLayout{};
  viewLayoutLayout->addWidget(m_layoutCombo);
  viewLayoutLayout->addSpacing(LayoutConstants::NarrowHMargin);
  viewLayoutLayout->addWidget(m_link2dCameras);
  viewLayoutLayout->setContentsMargins(0, 0, 0, 0);

  m_brightnessSlider = new SliderWithLabel{brightnessToUI(0.0f), brightnessToUI(2.0f)};
  m_brightnessSlider->setMaximumWidth(400);
  m_brightnessSlider->setToolTip(
    "Sets the brightness for materials and model skins in the 3D editing view.");
  m_gridAlphaSlider = new SliderWithLabel{0, 100};
  m_gridAlphaSlider->setMaximumWidth(400);
  m_gridAlphaSlider->setToolTip(
    "Sets the visibility of the grid lines in the 3D editing view.");
  m_fovSlider = new SliderWithLabel{50, 150};
  m_fovSlider->setMaximumWidth(400);
  m_fovSlider->setToolTip("Sets the field of vision in the 3D editing view.");

  m_showAxes = new QCheckBox{};
  m_showAxes->setToolTip(
    "Toggle showing the coordinate system axes in the 3D editing view.");

  m_filterModeCombo = new QComboBox{};
  m_filterModeCombo->setToolTip("Sets the texture filtering mode in the editing views.");
  for (const auto& filterMode : FilterModes)
  {
    m_filterModeCombo->addItem(QString::fromStdString(filterMode.name));
  }

  m_enableMsaa = new QCheckBox{};
  m_enableMsaa->setToolTip("Enable multisampling");

  m_materialBrowserIconSizeCombo = new QComboBox{};
  m_materialBrowserIconSizeCombo->addItem("25%");
  m_materialBrowserIconSizeCombo->addItem("50%");
  m_materialBrowserIconSizeCombo->addItem("100%");
  m_materialBrowserIconSizeCombo->addItem("150%");
  m_materialBrowserIconSizeCombo->addItem("200%");
  m_materialBrowserIconSizeCombo->addItem("250%");
  m_materialBrowserIconSizeCombo->addItem("300%");
  m_materialBrowserIconSizeCombo->setToolTip(
    "Sets the icon size in the material browser.");

  m_rendererFontSizeCombo = new QComboBox{};
  m_rendererFontSizeCombo->setEditable(true);
  m_rendererFontSizeCombo->setToolTip(
    "Sets the font size for various labels in the editing views.");
  m_rendererFontSizeCombo->addItems({"8",  "9",  "10", "11", "12", "13", "14", "15",
                                     "16", "17", "18", "19", "20", "22", "24", "26",
                                     "28", "32", "36", "40", "48", "56", "64", "72"});
  m_rendererFontSizeCombo->setValidator(new QIntValidator{1, 96});

  auto* layout = new FormWithSectionsLayout{};
  layout->setContentsMargins(0, LayoutConstants::MediumVMargin, 0, 0);
  layout->setVerticalSpacing(2);
  // override the default to make the sliders take up maximum width
  layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

  layout->addSection("User Interface");
  layout->addRow("Theme", themeLayout);

  layout->addSection("Map Views");
  layout->addRow("Layout", viewLayoutLayout);
  layout->addRow("Brightness", m_brightnessSlider);
  layout->addRow("Grid", m_gridAlphaSlider);
  layout->addRow("FOV", m_fovSlider);
  layout->addRow("Show axes", m_showAxes);
  layout->addRow("Filter mode", m_filterModeCombo);
  layout->addRow("Enable multisampling", m_enableMsaa);

  layout->addSection("Material Browser");
  layout->addRow("Icon size", m_materialBrowserIconSizeCombo);

  layout->addSection("Fonts");
  layout->addRow("Renderer Font Size", m_rendererFontSizeCombo);

  viewBox->setMinimumWidth(400);
  viewBox->setLayout(layout);

  return viewBox;
}

void ViewPreferencePane::bindEvents()
{
  connect(
    m_layoutCombo,
    QOverload<int>::of(&QComboBox::currentIndexChanged),
    this,
    &ViewPreferencePane::layoutChanged);
  connect(
    m_link2dCameras,
    &QCheckBox::checkStateChanged,
    this,
    &ViewPreferencePane::link2dCamerasChanged);
  connect(
    m_brightnessSlider,
    &SliderWithLabel::valueChanged,
    this,
    &ViewPreferencePane::brightnessChanged);
  connect(
    m_gridAlphaSlider,
    &SliderWithLabel::valueChanged,
    this,
    &ViewPreferencePane::gridAlphaChanged);
  connect(
    m_fovSlider, &SliderWithLabel::valueChanged, this, &ViewPreferencePane::fovChanged);
  connect(
    m_showAxes,
    &QCheckBox::checkStateChanged,
    this,
    &ViewPreferencePane::showAxesChanged);
  connect(
    m_enableMsaa,
    &QCheckBox::checkStateChanged,
    this,
    &ViewPreferencePane::enableMsaaChanged);
  connect(
    m_themeCombo,
    QOverload<int>::of(&QComboBox::activated),
    this,
    &ViewPreferencePane::themeChanged);
  connect(
    m_filterModeCombo,
    QOverload<int>::of(&QComboBox::currentIndexChanged),
    this,
    &ViewPreferencePane::filterModeChanged);
  connect(
    m_materialBrowserIconSizeCombo,
    QOverload<int>::of(&QComboBox::currentIndexChanged),
    this,
    &ViewPreferencePane::materialBrowserIconSizeChanged);
  connect(
    m_rendererFontSizeCombo,
    &QComboBox::currentTextChanged,
    this,
    &ViewPreferencePane::rendererFontSizeChanged);
}

bool ViewPreferencePane::canResetToDefaults()
{
  return true;
}

void ViewPreferencePane::doResetToDefaults()
{
  auto& prefs = PreferenceManager::instance();
  prefs.resetToDefault(Preferences::MapViewLayout);
  prefs.resetToDefault(Preferences::Link2DCameras);
  prefs.resetToDefault(Preferences::Brightness);
  prefs.resetToDefault(Preferences::GridAlpha);
  prefs.resetToDefault(Preferences::CameraFov);
  prefs.resetToDefault(Preferences::ShowAxes);
  prefs.resetToDefault(Preferences::EnableMSAA);
  prefs.resetToDefault(Preferences::TextureMinFilter);
  prefs.resetToDefault(Preferences::TextureMagFilter);
  prefs.resetToDefault(Preferences::Theme);
  prefs.resetToDefault(Preferences::MaterialBrowserIconSize);
  prefs.resetToDefault(Preferences::RendererFontSize);
}

void ViewPreferencePane::updateControls()
{
  m_layoutCombo->setCurrentIndex(pref(Preferences::MapViewLayout));
  m_link2dCameras->setChecked(pref(Preferences::Link2DCameras));
  m_brightnessSlider->setValue(brightnessToUI(pref(Preferences::Brightness)));
  m_gridAlphaSlider->setRatio(pref(Preferences::GridAlpha));
  m_fovSlider->setValue(int(pref(Preferences::CameraFov)));

  const auto filterModeIndex =
    findFilterMode(
      pref(Preferences::TextureMinFilter), pref(Preferences::TextureMagFilter))
      .value_or(-1);
  m_filterModeCombo->setCurrentIndex(int(filterModeIndex));

  m_showAxes->setChecked(pref(Preferences::ShowAxes));
  m_enableMsaa->setChecked(pref(Preferences::EnableMSAA));
  m_themeCombo->setCurrentIndex(findThemeIndex(pref(Preferences::Theme)));

  const auto materialBrowserIconSize = pref(Preferences::MaterialBrowserIconSize);
  if (materialBrowserIconSize == 0.25f)
  {
    m_materialBrowserIconSizeCombo->setCurrentIndex(0);
  }
  else if (materialBrowserIconSize == 0.5f)
  {
    m_materialBrowserIconSizeCombo->setCurrentIndex(1);
  }
  else if (materialBrowserIconSize == 1.5f)
  {
    m_materialBrowserIconSizeCombo->setCurrentIndex(3);
  }
  else if (materialBrowserIconSize == 2.0f)
  {
    m_materialBrowserIconSizeCombo->setCurrentIndex(4);
  }
  else if (materialBrowserIconSize == 2.5f)
  {
    m_materialBrowserIconSizeCombo->setCurrentIndex(5);
  }
  else if (materialBrowserIconSize == 3.0f)
  {
    m_materialBrowserIconSizeCombo->setCurrentIndex(6);
  }
  else
  {
    m_materialBrowserIconSizeCombo->setCurrentIndex(2);
  }

  m_rendererFontSizeCombo->setCurrentText(
    QString::asprintf("%i", pref(Preferences::RendererFontSize)));
}

bool ViewPreferencePane::validate()
{
  return true;
}

std::optional<size_t> ViewPreferencePane::findFilterMode(
  const int minFilter, const int magFilter) const
{
  return kdl::index_of(FilterModes, [&](const FilterMode& filterMode) {
    return filterMode.minFilter == minFilter && filterMode.magFilter == magFilter;
  });
}

int ViewPreferencePane::findThemeIndex(const QString& theme)
{
  return m_themeCombo->findText(theme);
}

void ViewPreferencePane::layoutChanged(const int index)
{
  assert(index >= 0 && index < 4);

  auto& prefs = PreferenceManager::instance();
  prefs.set(Preferences::MapViewLayout, index);
}

void ViewPreferencePane::link2dCamerasChanged(const int state)
{
  const auto value = state == Qt::Checked;
  auto& prefs = PreferenceManager::instance();
  prefs.set(Preferences::Link2DCameras, value);
}

void ViewPreferencePane::brightnessChanged(const int value)
{
  auto& prefs = PreferenceManager::instance();
  prefs.set(Preferences::Brightness, brightnessFromUI(value));
}

void ViewPreferencePane::gridAlphaChanged(const int /* value */)
{
  const auto ratio = m_gridAlphaSlider->ratio();
  auto& prefs = PreferenceManager::instance();
  prefs.set(Preferences::GridAlpha, ratio);
}

void ViewPreferencePane::fovChanged(const int value)
{
  auto& prefs = PreferenceManager::instance();
  prefs.set(Preferences::CameraFov, float(value));
}

void ViewPreferencePane::showAxesChanged(const int state)
{
  const auto value = state == Qt::Checked;
  auto& prefs = PreferenceManager::instance();
  prefs.set(Preferences::ShowAxes, value);
}

void ViewPreferencePane::enableMsaaChanged(const int state)
{
  const auto value = state == Qt::Checked;
  auto& prefs = PreferenceManager::instance();
  prefs.set(Preferences::EnableMSAA, value);
}

void ViewPreferencePane::filterModeChanged(const int value)
{
  const auto index = static_cast<size_t>(value);
  assert(index < FilterModes.size());
  const auto minFilter = FilterModes[index].minFilter;
  const auto magFilter = FilterModes[index].magFilter;

  auto& prefs = PreferenceManager::instance();
  prefs.set(Preferences::TextureMinFilter, minFilter);
  prefs.set(Preferences::TextureMagFilter, magFilter);
}

void ViewPreferencePane::themeChanged(int /*index*/)
{
  auto& prefs = PreferenceManager::instance();
  prefs.set(Preferences::Theme, m_themeCombo->currentText());
}

void ViewPreferencePane::materialBrowserIconSizeChanged(const int index)
{
  auto& prefs = PreferenceManager::instance();

  switch (index)
  {
  case 0:
    prefs.set(Preferences::MaterialBrowserIconSize, 0.25f);
    break;
  case 1:
    prefs.set(Preferences::MaterialBrowserIconSize, 0.5f);
    break;
  case 2:
    prefs.set(Preferences::MaterialBrowserIconSize, 1.0f);
    break;
  case 3:
    prefs.set(Preferences::MaterialBrowserIconSize, 1.5f);
    break;
  case 4:
    prefs.set(Preferences::MaterialBrowserIconSize, 2.0f);
    break;
  case 5:
    prefs.set(Preferences::MaterialBrowserIconSize, 2.5f);
    break;
  case 6:
    prefs.set(Preferences::MaterialBrowserIconSize, 3.0f);
    break;
  }
}

void ViewPreferencePane::rendererFontSizeChanged(const QString& str)
{
  bool ok;
  const auto value = str.toInt(&ok);
  if (ok)
  {
    auto& prefs = PreferenceManager::instance();
    prefs.set(Preferences::RendererFontSize, value);
  }
}

} // namespace tb::ui
