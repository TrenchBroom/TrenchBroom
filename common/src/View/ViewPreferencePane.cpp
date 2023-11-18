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

#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QtGlobal>

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/GL.h"
#include "View/ColorButton.h"
#include "View/FormWithSectionsLayout.h"
#include "View/MapViewLayout.h"
#include "View/QtUtils.h"
#include "View/SliderWithLabel.h"
#include "View/ViewConstants.h"

#include <vecmath/scalar.h>

#include <array>
#include <string>

namespace TrenchBroom::View
{
namespace
{
struct TextureMode
{
  int minFilter;
  int magFilter;
  std::string name;

  TextureMode(const int i_minFilter, const int i_magFilter, std::string i_name)
    : minFilter{i_minFilter}
    , magFilter{i_magFilter}
    , name{std::move(i_name)}
  {
  }
};

const auto TextureModes = std::array<TextureMode, 6>{
  TextureMode{GL_NEAREST, GL_NEAREST, "Nearest"},
  TextureMode{GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST, "Nearest (mipmapped)"},
  TextureMode{GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST, "Nearest (mipmapped, interpolated)"},
  TextureMode{GL_LINEAR, GL_LINEAR, "Linear"},
  TextureMode{GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR, "Linear (mipmapped)"},
  TextureMode{GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, "Linear (mipmapped, interpolated)"},
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

  m_viewCountCombo = new QComboBox{};
  m_viewCountCombo->setToolTip("Sets the number of displayed editing views.");
  m_viewCountCombo->addItem("One Pane");
  m_viewCountCombo->addItem("Two Panes");
  m_viewCountCombo->addItem("Three Panes");
  m_viewCountCombo->addItem("Four Panes");

  m_viewArrangementCombo = new QComboBox{};
  m_viewCountCombo->setToolTip("Sets the arrangement of the editing views.");

  m_link2dCameras = new QCheckBox{"Sync 2D views"};
  m_link2dCameras->setToolTip("All 2D views pan and zoom together.");

  auto* viewLayoutLayout = new QHBoxLayout{};
  viewLayoutLayout->addWidget(m_viewCountCombo);

  viewLayoutLayout->addSpacing(LayoutConstants::NarrowHMargin);
  viewLayoutLayout->addWidget(m_viewArrangementCombo);
  viewLayoutLayout->setContentsMargins(0, 0, 0, 0);
  viewLayoutLayout->addSpacing(LayoutConstants::NarrowHMargin);
  viewLayoutLayout->addWidget(m_link2dCameras);
  viewLayoutLayout->setContentsMargins(0, 0, 0, 0);

  m_brightnessSlider = new SliderWithLabel{brightnessToUI(0.0f), brightnessToUI(2.0f)};
  m_brightnessSlider->setMaximumWidth(400);
  m_brightnessSlider->setToolTip(
    "Sets the brightness for textures and model skins in the 3D editing view.");
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

  m_textureModeCombo = new QComboBox{};
  m_textureModeCombo->setToolTip("Sets the texture filtering mode in the editing views.");
  for (const auto& textureMode : TextureModes)
  {
    m_textureModeCombo->addItem(QString::fromStdString(textureMode.name));
  }

  m_enableMsaa = new QCheckBox{};
  m_enableMsaa->setToolTip("Enable multisampling");

  m_textureBrowserIconSizeCombo = new QComboBox{};
  m_textureBrowserIconSizeCombo->addItem("25%");
  m_textureBrowserIconSizeCombo->addItem("50%");
  m_textureBrowserIconSizeCombo->addItem("100%");
  m_textureBrowserIconSizeCombo->addItem("150%");
  m_textureBrowserIconSizeCombo->addItem("200%");
  m_textureBrowserIconSizeCombo->addItem("250%");
  m_textureBrowserIconSizeCombo->addItem("300%");
  m_textureBrowserIconSizeCombo->setToolTip("Sets the icon size in the texture browser.");

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
  layout->addRow("Texture mode", m_textureModeCombo);
  layout->addRow("Enable multisampling", m_enableMsaa);

  layout->addSection("Texture Browser");
  layout->addRow("Icon size", m_textureBrowserIconSizeCombo);

  layout->addSection("Fonts");
  layout->addRow("Renderer Font Size", m_rendererFontSizeCombo);

  viewBox->setMinimumWidth(400);
  viewBox->setLayout(layout);

  return viewBox;
}

void ViewPreferencePane::bindEvents()
{
  connect(
    m_viewCountCombo,
    QOverload<int>::of(&QComboBox::currentIndexChanged),
    this,
    &ViewPreferencePane::layoutChanged);
  connect(
    m_viewArrangementCombo,
    QOverload<int>::of(&QComboBox::currentIndexChanged),
    this,
    &ViewPreferencePane::layoutChanged);
  connect(
    m_link2dCameras,
    &QCheckBox::stateChanged,
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
    m_showAxes, &QCheckBox::stateChanged, this, &ViewPreferencePane::showAxesChanged);
  connect(
    m_enableMsaa, &QCheckBox::stateChanged, this, &ViewPreferencePane::enableMsaaChanged);
  connect(
    m_themeCombo,
    QOverload<int>::of(&QComboBox::activated),
    this,
    &ViewPreferencePane::themeChanged);
  connect(
    m_textureModeCombo,
    QOverload<int>::of(&QComboBox::currentIndexChanged),
    this,
    &ViewPreferencePane::textureModeChanged);
  connect(
    m_textureBrowserIconSizeCombo,
    QOverload<int>::of(&QComboBox::currentIndexChanged),
    this,
    &ViewPreferencePane::textureBrowserIconSizeChanged);
  connect(
    m_rendererFontSizeCombo,
    &QComboBox::currentTextChanged,
    this,
    &ViewPreferencePane::rendererFontSizeChanged);
}

bool ViewPreferencePane::doCanResetToDefaults()
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
  prefs.resetToDefault(Preferences::TextureBrowserIconSize);
  prefs.resetToDefault(Preferences::RendererFontSize);
}

void ViewPreferencePane::updateViewCombos()
{
  m_viewCountCombo->blockSignals(true);
  m_viewArrangementCombo->blockSignals(true);

  const auto viewLayout = static_cast<MapViewLayout>(pref(Preferences::MapViewLayout));
  m_viewArrangementCombo->clear();
  m_viewArrangementCombo->show();

  if (viewLayout == MapViewLayout::OnePane)
  {
    m_viewCountCombo->setCurrentIndex(0);
    m_viewArrangementCombo->hide();
  }
  else if (
    viewLayout == MapViewLayout::TwoPanesVertical
    || viewLayout == MapViewLayout::TwoPanesHorizontal)
  {
    m_viewCountCombo->setCurrentIndex(1);
    m_viewArrangementCombo->addItem("vertical");
    m_viewArrangementCombo->addItem("horizontal");
    m_viewArrangementCombo->setCurrentIndex(
      viewLayout == MapViewLayout::TwoPanesVertical ? 0 : 1);
  }
  else if (
    viewLayout == MapViewLayout::ThreePanesVertical
    || viewLayout == MapViewLayout::ThreePanesHorizontal)
  {
    m_viewCountCombo->setCurrentIndex(2);
    m_viewArrangementCombo->addItem("vertical");
    m_viewArrangementCombo->addItem("horizontal");
    m_viewArrangementCombo->setCurrentIndex(
      viewLayout == MapViewLayout::ThreePanesVertical ? 0 : 1);
  }
  else
  {
    m_viewCountCombo->setCurrentIndex(3);
    m_viewArrangementCombo->addItem("vertical");
    m_viewArrangementCombo->addItem("horizontal");
    m_viewArrangementCombo->addItem("grid");
    if (viewLayout == MapViewLayout::FourPanesVertical)
    {
      m_viewArrangementCombo->setCurrentIndex(0);
    }
    else if (viewLayout == MapViewLayout::FourPanesHorizontal)
    {
      m_viewArrangementCombo->setCurrentIndex(1);
    }
    else if (viewLayout == MapViewLayout::FourPanesGrid)
    {
      m_viewArrangementCombo->setCurrentIndex(2);
    }
  }

  m_viewCountCombo->blockSignals(false);
  m_viewArrangementCombo->blockSignals(false);
}

void ViewPreferencePane::doUpdateControls()
{
  updateViewCombos();

  m_link2dCameras->setChecked(pref(Preferences::Link2DCameras));
  m_brightnessSlider->setValue(brightnessToUI(pref(Preferences::Brightness)));
  m_gridAlphaSlider->setRatio(pref(Preferences::GridAlpha));
  m_fovSlider->setValue(int(pref(Preferences::CameraFov)));

  const auto textureModeIndex = findTextureMode(
    pref(Preferences::TextureMinFilter), pref(Preferences::TextureMagFilter));
  m_textureModeCombo->setCurrentIndex(int(textureModeIndex));

  m_showAxes->setChecked(pref(Preferences::ShowAxes));
  m_enableMsaa->setChecked(pref(Preferences::EnableMSAA));
  m_themeCombo->setCurrentIndex(findThemeIndex(pref(Preferences::Theme)));

  const auto textureBrowserIconSize = pref(Preferences::TextureBrowserIconSize);
  if (textureBrowserIconSize == 0.25f)
  {
    m_textureBrowserIconSizeCombo->setCurrentIndex(0);
  }
  else if (textureBrowserIconSize == 0.5f)
  {
    m_textureBrowserIconSizeCombo->setCurrentIndex(1);
  }
  else if (textureBrowserIconSize == 1.5f)
  {
    m_textureBrowserIconSizeCombo->setCurrentIndex(3);
  }
  else if (textureBrowserIconSize == 2.0f)
  {
    m_textureBrowserIconSizeCombo->setCurrentIndex(4);
  }
  else if (textureBrowserIconSize == 2.5f)
  {
    m_textureBrowserIconSizeCombo->setCurrentIndex(5);
  }
  else if (textureBrowserIconSize == 3.0f)
  {
    m_textureBrowserIconSizeCombo->setCurrentIndex(6);
  }
  else
  {
    m_textureBrowserIconSizeCombo->setCurrentIndex(2);
  }

  m_rendererFontSizeCombo->setCurrentText(
    QString::asprintf("%i", pref(Preferences::RendererFontSize)));
}

bool ViewPreferencePane::doValidate()
{
  return true;
}

size_t ViewPreferencePane::findTextureMode(const int minFilter, const int magFilter) const
{
  for (size_t i = 0; i < TextureModes.size(); ++i)
  {
    if (TextureModes[i].minFilter == minFilter && TextureModes[i].magFilter == magFilter)
    {
      return i;
    }
  }
  return TextureModes.size();
}

int ViewPreferencePane::findThemeIndex(const QString& theme)
{
  for (int i = 0; i < m_themeCombo->count(); ++i)
  {
    if (m_themeCombo->itemText(i) == theme)
    {
      return i;
    }
  }
  return 0;
}

void ViewPreferencePane::layoutChanged([[maybe_unused]] const int index)
{
  const auto countIndex = m_viewCountCombo->currentIndex();
  const auto arrangementIndex = m_viewArrangementCombo->currentIndex();

  auto& prefs = PreferenceManager::instance();
  if (countIndex == 0)
  {
    prefs.set(Preferences::MapViewLayout, static_cast<int>(MapViewLayout::OnePane));
  }
  else if (countIndex == 1 && arrangementIndex <= 0)
  {
    prefs.set(
      Preferences::MapViewLayout, static_cast<int>(MapViewLayout::TwoPanesVertical));
  }
  else if (countIndex == 1 && arrangementIndex == 1)
  {
    prefs.set(
      Preferences::MapViewLayout, static_cast<int>(MapViewLayout::TwoPanesHorizontal));
  }
  else if (countIndex == 2 && (arrangementIndex <= 0 || arrangementIndex > 1))
  {
    prefs.set(
      Preferences::MapViewLayout, static_cast<int>(MapViewLayout::ThreePanesVertical));
  }
  else if (countIndex == 2 && arrangementIndex == 1)
  {
    prefs.set(
      Preferences::MapViewLayout, static_cast<int>(MapViewLayout::ThreePanesHorizontal));
  }
  else if (countIndex == 3 && (arrangementIndex <= 0 || arrangementIndex > 2))
  {
    prefs.set(
      Preferences::MapViewLayout, static_cast<int>(MapViewLayout::FourPanesVertical));
  }
  else if (countIndex == 3 && arrangementIndex == 1)
  {
    prefs.set(
      Preferences::MapViewLayout, static_cast<int>(MapViewLayout::FourPanesHorizontal));
  }
  else if (countIndex == 3 && arrangementIndex == 2)
  {
    prefs.set(Preferences::MapViewLayout, static_cast<int>(MapViewLayout::FourPanesGrid));
  }
  else
  {
    assert(false);
  }

  updateViewCombos();
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

void ViewPreferencePane::textureModeChanged(const int value)
{
  const auto index = static_cast<size_t>(value);
  assert(index < TextureModes.size());
  const auto minFilter = TextureModes[index].minFilter;
  const auto magFilter = TextureModes[index].magFilter;

  auto& prefs = PreferenceManager::instance();
  prefs.set(Preferences::TextureMinFilter, minFilter);
  prefs.set(Preferences::TextureMagFilter, magFilter);
}

void ViewPreferencePane::themeChanged(int /*index*/)
{
  auto& prefs = PreferenceManager::instance();
  prefs.set(Preferences::Theme, m_themeCombo->currentText());
}

void ViewPreferencePane::textureBrowserIconSizeChanged(const int index)
{
  auto& prefs = PreferenceManager::instance();

  switch (index)
  {
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
} // namespace TrenchBroom::View
