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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ui/FaceAttribsEditor.h"

#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QVBoxLayout>
#include <QtGlobal>

#include "gl/Material.h"
#include "gl/Texture.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceAttributes.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/GameConfig.h"
#include "mdl/GameInfo.h"
#include "mdl/Grid.h"
#include "mdl/Map.h"
#include "mdl/MapFormat.h"
#include "mdl/Map_Brushes.h"
#include "mdl/UpdateBrushFaceAttributes.h"
#include "mdl/WorldNode.h"
#include "ui/BitmapButton.h"
#include "ui/BorderLine.h"
#include "ui/FlagsPopupEditor.h"
#include "ui/MapDocument.h"
#include "ui/QStyleUtils.h"
#include "ui/SignalDelayer.h"
#include "ui/SpinControl.h"
#include "ui/UVEditor.h"
#include "ui/ViewConstants.h"
#include "ui/ViewUtils.h"

#include "kd/string_format.h"
#include "kd/string_utils.h"

#include "vm/vec_io.h" // IWYU pragma: keep

#include <string>

namespace tb::ui
{

FaceAttribsEditor::FaceAttribsEditor(
  MapDocument& document, gl::ContextManager& contextManager, QWidget* parent)
  : QWidget{parent}
  , m_document{document}
  , m_updateControlsSignalDelayer{new SignalDelayer{this}}
{
  createGui(contextManager);
  bindEvents();
  connectObservers();
  updateIncrements();
}

bool FaceAttribsEditor::cancelMouseDrag()
{
  return m_uvEditor->cancelMouseDrag();
}

void FaceAttribsEditor::xOffsetChanged(const double value)
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(map, {.xOffset = mdl::SetValue{float(value)}}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::yOffsetChanged(const double value)
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(map, {.yOffset = mdl::SetValue{float(value)}}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::rotationChanged(const double value)
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(map, {.rotation = mdl::SetValue{float(value)}}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::xScaleChanged(const double value)
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(map, {.xScale = mdl::SetValue{float(value)}}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::yScaleChanged(const double value)
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(map, {.yScale = mdl::SetValue{float(value)}}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::surfaceFlagChanged(
  const size_t /* index */, const int value, const int setFlag, const int /* mixedFlag */)
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(
        map,
        {.surfaceFlags = setFlag & value ? mdl::FlagOp{mdl::SetFlagBits{value}}
                                         : mdl::FlagOp{mdl::ClearFlagBits{value}}}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::contentFlagChanged(
  const size_t /* index */, const int value, const int setFlag, const int /* mixedFlag */)
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(
        map,
        {.surfaceContents = setFlag & value ? mdl::FlagOp{mdl::SetFlagBits{value}}
                                            : mdl::FlagOp{mdl::ClearFlagBits{value}}}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::surfaceValueChanged(const double value)
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(map, {.surfaceValue = mdl::SetValue{float(value)}}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::colorValueChanged(const QString& /* text */)
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  const std::string str = m_colorEditor->text().toStdString();
  if (!kdl::str_is_blank(str))
  {
    Color::parse(str) | kdl::transform([&](const auto& color) {
      if (!setBrushFaceAttributes(map, {.color = {color}}))
      {
        updateControls();
      }
    }) | kdl::ignore();
  }
  else
  {
    if (!setBrushFaceAttributes(map, {.color = {std::nullopt}}))
    {
      updateControls();
    }
  }
}

void FaceAttribsEditor::surfaceFlagsUnset()
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(map, {.surfaceFlags = mdl::SetFlags{std::nullopt}}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::contentFlagsUnset()
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(map, {.surfaceContents = mdl::SetFlags{std::nullopt}}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::surfaceValueUnset()
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(map, {.surfaceValue = mdl::SetValue{std::nullopt}}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::colorValueUnset()
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(map, {.color = std::optional<Color>(std::nullopt)}))
  {
    updateControls();
  }
}

// SiN stuff
void FaceAttribsEditor::sinNonlitValueChanged(const double value)
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(map, {.sinNonlitValue = float(value)}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::sinNonlitValueUnset()
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(
        map, {.sinNonlitValue = std::optional<float>{std::nullopt}}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::sinTransAngleChanged(const double value)
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(map, {.sinTransAngle = float(value)}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::sinTransAngleUnset()
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(map, {.sinTransAngle = std::optional<int>{std::nullopt}}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::sinTransMagChanged(const double value)
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(map, {.sinTransMag = float(value)}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::sinTransMagUnset()
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(map, {.sinTransMag = std::optional<float>{std::nullopt}}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::sinTranslucenceChanged(const double value)
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(map, {.sinTranslucence = float(value)}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::sinTranslucenceUnset()
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(
        map, {.sinTranslucence = std::optional<float>{std::nullopt}}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::sinRestitutionChanged(const double value)
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(map, {.sinRestitution = float(value)}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::sinRestitutionUnset()
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(
        map, {.sinRestitution = std::optional<float>{std::nullopt}}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::sinFrictionChanged(const double value)
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(map, {.sinFriction = float(value)}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::sinFrictionUnset()
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(map, {.sinFriction = std::optional<float>{std::nullopt}}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::sinAnimTimeChanged(const double value)
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(map, {.sinAnimTime = float(value)}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::sinAnimTimeUnset()
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(map, {.sinAnimTime = std::optional<float>{std::nullopt}}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::sinDirectStyleValueChanged(const QString& /* text */)
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  const std::string str = m_surfaceSiNDirectStyleEditor->text().toStdString();
  if (!kdl::str_is_blank(str))
  {
    if (!setBrushFaceAttributes(map, {.sinDirectStyle = {str}}))
    {
      updateControls();
    }
  }
  else
  {
    if (!setBrushFaceAttributes(
          map, {.sinDirectStyle = std::optional<std::string>{std::nullopt}}))
    {
      updateControls();
    }
  }
}

void FaceAttribsEditor::sinDirectStyleValueUnset()
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(
        map, {.sinDirectStyle = std::optional<std::string>(std::nullopt)}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::sinDirectChanged(const double value)
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(map, {.sinDirect = float(value)}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::sinDirectUnset()
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(map, {.sinDirect = std::optional<float>(std::nullopt)}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::sinDirectAngleChanged(const double value)
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(map, {.sinDirectAngle = float(value)}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::sinDirectAngleUnset()
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(
        map, {.sinDirectAngle = std::optional<float>{std::nullopt}}))
  {
    updateControls();
  }
}

// SiN Extended
void FaceAttribsEditor::sinExtDirectScaleChanged(const double value)
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(map, {.sinExtDirectScale = float(value)}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::sinExtDirectScaleUnset()
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(
        map, {.sinExtDirectScale = std::optional<float>(std::nullopt)}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::sinExtPatchScaleChanged(const double value)
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(map, {.sinExtPatchScale = float(value)}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::sinExtPatchScaleUnset()
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(
        map, {.sinExtPatchScale = std::optional<float>(std::nullopt)}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::sinExtMinLightChanged(const double value)
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(map, {.sinExtMinLight = float(value)}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::sinExtMinLightUnset()
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(
        map, {.sinExtMinLight = std::optional<float>(std::nullopt)}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::sinExtMaxLightChanged(const double value)
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(map, {.sinExtMaxLight = float(value)}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::sinExtMaxLightUnset()
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(
        map, {.sinExtMaxLight = std::optional<float>(std::nullopt)}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::sinExtLuxelScaleChanged(const double value)
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(map, {.sinExtLuxelScale = float(value)}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::sinExtLuxelScaleUnset()
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(
        map, {.sinExtLuxelScale = std::optional<float>(std::nullopt)}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::sinExtMottleChanged(const double value)
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(map, {.sinExtMottle = float(value)}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::sinExtMottleUnset()
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(map, {.sinExtMottle = std::optional<float>(std::nullopt)}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::sinExtFlagsUnset()
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(map, {.extendedFlags = mdl::SetFlags{std::nullopt}}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::sinExtFlagChanged(
  const size_t /* index */, const int value, const int setFlag, const int /* mixedFlag */)
{
  auto& map = m_document.map();
  if (!map.selection().hasAnyBrushFaces())
  {
    return;
  }

  if (!setBrushFaceAttributes(
        map,
        {.extendedFlags = setFlag & value ? mdl::FlagOp{mdl::SetFlagBits{value}}
                                          : mdl::FlagOp{mdl::ClearFlagBits{value}}}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::updateIncrements()
{
  auto& map = m_document.map();
  const auto& grid = map.grid();

  m_xOffsetEditor->setIncrements(grid.actualSize(), 2.0 * grid.actualSize(), 1.0);
  m_yOffsetEditor->setIncrements(grid.actualSize(), 2.0 * grid.actualSize(), 1.0);
  m_rotationEditor->setIncrements(vm::to_degrees(grid.angle()), 90.0, 1.0);
}

static QWidget* createUnsetButtonLayout(QWidget* expandWidget, QWidget* button)
{
  auto* wrapper = new QWidget();
  auto* rowLayout = new QHBoxLayout();
  rowLayout->setContentsMargins(0, 0, 0, 0);
  rowLayout->setSpacing(LayoutConstants::NarrowHMargin);
  rowLayout->addWidget(expandWidget, 1);
  rowLayout->addWidget(button);
  wrapper->setLayout(rowLayout);
  return wrapper;
}

void FaceAttribsEditor::createGui(gl::ContextManager& contextManager)
{
  m_uvEditor = new UVEditor{m_document, contextManager};

  auto* materialNameLabel = new QLabel{"Material"};
  setEmphasizedStyle(materialNameLabel);
  m_materialName = new QLabel{"none"};
  m_materialName->setTextInteractionFlags(Qt::TextSelectableByMouse);

  auto* textureSizeLabel = new QLabel{"Size"};
  setEmphasizedStyle(textureSizeLabel);
  m_textureSize = new QLabel{""};

  const auto max = std::numeric_limits<double>::max();
  const auto min = -max;

  auto* xOffsetLabel = new QLabel{"X Offset"};
  setEmphasizedStyle(xOffsetLabel);
  m_xOffsetEditor = new SpinControl{};
  m_xOffsetEditor->setRange(min, max);
  m_xOffsetEditor->setDigits(0, 6);

  auto* yOffsetLabel = new QLabel{"Y Offset"};
  setEmphasizedStyle(yOffsetLabel);
  m_yOffsetEditor = new SpinControl{};
  m_yOffsetEditor->setRange(min, max);
  m_yOffsetEditor->setDigits(0, 6);

  auto* xScaleLabel = new QLabel{"X Scale"};
  setEmphasizedStyle(xScaleLabel);
  m_xScaleEditor = new SpinControl{};
  m_xScaleEditor->setRange(min, max);
  m_xScaleEditor->setIncrements(0.1, 0.25, 0.01);
  m_xScaleEditor->setDigits(0, 6);

  auto* yScaleLabel = new QLabel{"Y Scale"};
  setEmphasizedStyle(yScaleLabel);
  m_yScaleEditor = new SpinControl{};
  m_yScaleEditor->setRange(min, max);
  m_yScaleEditor->setIncrements(0.1, 0.25, 0.01);
  m_yScaleEditor->setDigits(0, 6);

  auto* rotationLabel = new QLabel{"Angle"};
  setEmphasizedStyle(rotationLabel);
  m_rotationEditor = new SpinControl{};
  m_rotationEditor->setRange(min, max);
  m_rotationEditor->setDigits(0, 6);

  m_surfaceValueLabel = new QLabel{"Value"};
  setEmphasizedStyle(m_surfaceValueLabel);
  m_surfaceValueEditor = new SpinControl{};
  m_surfaceValueEditor->setRange(min, max);
  m_surfaceValueEditor->setIncrements(1.0, 10.0, 100.0);
  m_surfaceValueEditor->setDigits(0, 6);
  m_surfaceValueUnsetButton =
    createBitmapButton("ResetUV.svg", tr("Unset surface value"));
  m_surfaceValueEditorLayout =
    createUnsetButtonLayout(m_surfaceValueEditor, m_surfaceValueUnsetButton);

  m_surfaceFlagsLabel = new QLabel{"Surface"};
  setEmphasizedStyle(m_surfaceFlagsLabel);
  m_surfaceFlagsEditor = new FlagsPopupEditor{2};
  m_surfaceFlagsUnsetButton =
    createBitmapButton("ResetUV.svg", tr("Unset surface flags"));
  m_surfaceFlagsEditorLayout =
    createUnsetButtonLayout(m_surfaceFlagsEditor, m_surfaceFlagsUnsetButton);

  m_contentFlagsLabel = new QLabel{"Content"};
  setEmphasizedStyle(m_contentFlagsLabel);
  m_contentFlagsEditor = new FlagsPopupEditor{2};
  m_contentFlagsUnsetButton =
    createBitmapButton("ResetUV.svg", tr("Unset content flags"));
  m_contentFlagsEditorLayout =
    createUnsetButtonLayout(m_contentFlagsEditor, m_contentFlagsUnsetButton);

  m_colorLabel = new QLabel{"Color"};
  setEmphasizedStyle(m_colorLabel);
  m_colorEditor = new QLineEdit{};
  m_colorUnsetButton = createBitmapButton("ResetUV.svg", tr("Unset color"));
  m_colorEditorLayout = createUnsetButtonLayout(m_colorEditor, m_colorUnsetButton);

  // SiN stuff
  auto setupSiNNumericControl = [](
                                  const char* labelStr,
                                  QLabel*& label,
                                  SpinControl*& spin,
                                  QAbstractButton*& unset,
                                  QWidget*& layout,
                                  double min,
                                  double max,
                                  double sinc,
                                  double minc,
                                  double linc) {
    label = new QLabel{labelStr};
    setEmphasizedStyle(label);
    spin = new SpinControl{};
    spin->setRange(min, max);
    spin->setIncrements(sinc, minc, linc);
    spin->setDigits(0, 6);
    unset = createBitmapButton("ResetUV.svg", tr("Unset value"));
    layout = createUnsetButtonLayout(spin, unset);
  };

  setupSiNNumericControl(
    "Non-lit Value",
    m_surfaceSiNNonlitValueLabel,
    m_surfaceSiNNonlitValueEditor,
    m_surfaceSiNNonlitValueUnsetButton,
    m_surfaceSiNNonlitValueEditorLayout,
    0.0f,
    1.0f,
    0.01f,
    0.05f,
    0.1f);

  setupSiNNumericControl(
    "Translate Angle",
    m_surfaceSiNTransAngleLabel,
    m_surfaceSiNTransAngleEditor,
    m_surfaceSiNTransAngleUnsetButton,
    m_surfaceSiNTransAngleEditorLayout,
    0.0f,
    360.0f,
    1,
    5,
    30);

  setupSiNNumericControl(
    "Translate Magnitude",
    m_surfaceSiNTransMagLabel,
    m_surfaceSiNTransMagEditor,
    m_surfaceSiNTransMagUnsetButton,
    m_surfaceSiNTransMagEditorLayout,
    min,
    max,
    1.0f,
    10.0f,
    100.0f);

  setupSiNNumericControl(
    "Translucence",
    m_surfaceSiNTranslucenceLabel,
    m_surfaceSiNTranslucenceEditor,
    m_surfaceSiNTranslucenceUnsetButton,
    m_surfaceSiNTranslucenceEditorLayout,
    0.0f,
    1.0f,
    0.01f,
    0.05f,
    0.1f);

  setupSiNNumericControl(
    "Restitution",
    m_surfaceSiNRestitutionLabel,
    m_surfaceSiNRestitutionEditor,
    m_surfaceSiNRestitutionUnsetButton,
    m_surfaceSiNRestitutionEditorLayout,
    0.0f,
    1.0f,
    0.01f,
    0.05f,
    0.1f);

  setupSiNNumericControl(
    "Friction",
    m_surfaceSiNFrictionLabel,
    m_surfaceSiNFrictionEditor,
    m_surfaceSiNFrictionUnsetButton,
    m_surfaceSiNFrictionEditorLayout,
    0.0f,
    max,
    0.01f,
    0.05f,
    0.1f);

  setupSiNNumericControl(
    "Anim Time",
    m_surfaceSiNAnimTimeLabel,
    m_surfaceSiNAnimTimeEditor,
    m_surfaceSiNAnimTimeUnsetButton,
    m_surfaceSiNAnimTimeEditorLayout,
    0.0f,
    max,
    0.1f,
    0.25f,
    0.5f);

  m_surfaceSiNDirectStyleLabel = new QLabel{"Direct Style"};
  setEmphasizedStyle(m_surfaceSiNDirectStyleLabel);
  m_surfaceSiNDirectStyleEditor = new QLineEdit{};
  m_surfaceSiNDirectStyleUnsetButton =
    createBitmapButton("ResetUV.svg", tr("Unset surface direct style"));
  m_surfaceSiNDirectStyleEditorLayout = createUnsetButtonLayout(
    m_surfaceSiNDirectStyleEditor, m_surfaceSiNDirectStyleUnsetButton);

  setupSiNNumericControl(
    "Direct",
    m_surfaceSiNDirectLabel,
    m_surfaceSiNDirectEditor,
    m_surfaceSiNDirectUnsetButton,
    m_surfaceSiNDirectEditorLayout,
    min,
    max,
    1,
    10,
    100);

  setupSiNNumericControl(
    "Direct Angle",
    m_surfaceSiNDirectAngleLabel,
    m_surfaceSiNDirectAngleEditor,
    m_surfaceSiNDirectAngleUnsetButton,
    m_surfaceSiNDirectAngleEditorLayout,
    0.0f,
    360.0f,
    1,
    5,
    30);

  // Extended
  setupSiNNumericControl(
    "(Ext) Direct Scale",
    m_surfaceSiNExtDirectScaleLabel,
    m_surfaceSiNExtDirectScaleEditor,
    m_surfaceSiNExtDirectScaleUnsetButton,
    m_surfaceSiNExtDirectScaleEditorLayout,
    0.0f,
    max,
    0.01f,
    0.1f,
    0.5f);

  setupSiNNumericControl(
    "(Ext) Patch Scale",
    m_surfaceSiNExtPatchScaleLabel,
    m_surfaceSiNExtPatchScaleEditor,
    m_surfaceSiNExtPatchScaleUnsetButton,
    m_surfaceSiNExtPatchScaleEditorLayout,
    0.0f,
    max,
    0.01f,
    0.1f,
    0.5f);

  setupSiNNumericControl(
    "(Ext) Minlight",
    m_surfaceSiNExtMinLightLabel,
    m_surfaceSiNExtMinLightEditor,
    m_surfaceSiNExtMinLightUnsetButton,
    m_surfaceSiNExtMinLightEditorLayout,
    0.0f,
    1.0f,
    0.01f,
    0.1f,
    0.5f);

  setupSiNNumericControl(
    "(Ext) Maxlight",
    m_surfaceSiNExtMaxLightLabel,
    m_surfaceSiNExtMaxLightEditor,
    m_surfaceSiNExtMaxLightUnsetButton,
    m_surfaceSiNExtMaxLightEditorLayout,
    0.0f,
    1.0f,
    0.01f,
    0.1f,
    0.5f);

  setupSiNNumericControl(
    "(Ext) Luxel Scale",
    m_surfaceSiNExtLuxelScaleLabel,
    m_surfaceSiNExtLuxelScaleEditor,
    m_surfaceSiNExtLuxelScaleUnsetButton,
    m_surfaceSiNExtLuxelScaleEditorLayout,
    0.0f,
    max,
    0.01f,
    0.1f,
    0.5f);

  setupSiNNumericControl(
    "(Ext) Mottle",
    m_surfaceSiNExtMottleLabel,
    m_surfaceSiNExtMottleEditor,
    m_surfaceSiNExtMottleUnsetButton,
    m_surfaceSiNExtMottleEditorLayout,
    0.0f,
    1.0f,
    0.01f,
    0.1f,
    0.5f);

  m_sinExtFlagsLabel = new QLabel{"Extended"};
  setEmphasizedStyle(m_sinExtFlagsLabel);
  m_sinExtFlagsEditor = new FlagsPopupEditor{2};
  m_sinExtFlagsUnsetButton =
    createBitmapButton("ResetUV.svg", tr("Unset extended flags"));
  m_sinExtFlagsEditorLayout =
    createUnsetButtonLayout(m_sinExtFlagsEditor, m_sinExtFlagsUnsetButton);


  const Qt::Alignment LabelFlags = Qt::AlignVCenter | Qt::AlignRight;
  const Qt::Alignment ValueFlags = Qt::AlignVCenter;

  auto* faceAttribsLayout = new QGridLayout{};
  faceAttribsLayout->setContentsMargins(
    LayoutConstants::NarrowHMargin,
    LayoutConstants::MediumVMargin,
    LayoutConstants::NarrowHMargin,
    LayoutConstants::MediumVMargin);
  faceAttribsLayout->setHorizontalSpacing(LayoutConstants::MediumHMargin);
  faceAttribsLayout->setVerticalSpacing(LayoutConstants::MediumVMargin);

  int r = 0;
  int c = 0;

  faceAttribsLayout->addWidget(materialNameLabel, r, c++, LabelFlags);
  faceAttribsLayout->addWidget(m_materialName, r, c++, ValueFlags);
  faceAttribsLayout->addWidget(textureSizeLabel, r, c++, LabelFlags);
  faceAttribsLayout->addWidget(m_textureSize, r, c++, ValueFlags);
  ++r;
  c = 0;

  faceAttribsLayout->addWidget(xOffsetLabel, r, c++, LabelFlags);
  faceAttribsLayout->addWidget(m_xOffsetEditor, r, c++);
  faceAttribsLayout->addWidget(yOffsetLabel, r, c++, LabelFlags);
  faceAttribsLayout->addWidget(m_yOffsetEditor, r, c++);
  ++r;
  c = 0;

  faceAttribsLayout->addWidget(xScaleLabel, r, c++, LabelFlags);
  faceAttribsLayout->addWidget(m_xScaleEditor, r, c++);
  faceAttribsLayout->addWidget(yScaleLabel, r, c++, LabelFlags);
  faceAttribsLayout->addWidget(m_yScaleEditor, r, c++);
  ++r;
  c = 0;

  faceAttribsLayout->addWidget(rotationLabel, r, c++, LabelFlags);
  faceAttribsLayout->addWidget(m_rotationEditor, r, c++);
  faceAttribsLayout->addWidget(m_surfaceValueLabel, r, c++, LabelFlags);
  faceAttribsLayout->addWidget(m_surfaceValueEditorLayout, r, c++);
  ++r;
  c = 0;

  faceAttribsLayout->addWidget(m_surfaceFlagsLabel, r, c++, LabelFlags);
  faceAttribsLayout->addWidget(m_surfaceFlagsEditorLayout, r, c++, 1, 3);
  ++r;
  c = 0;

  faceAttribsLayout->addWidget(m_contentFlagsLabel, r, c++, LabelFlags);
  faceAttribsLayout->addWidget(m_contentFlagsEditorLayout, r, c++, 1, 3);
  ++r;
  c = 0;

  faceAttribsLayout->addWidget(m_colorLabel, r, c++, LabelFlags);
  faceAttribsLayout->addWidget(m_colorEditorLayout, r, c++, 1, 3);
  ++r;
  c = 0;

  // SiN
  faceAttribsLayout->addWidget(m_surfaceSiNNonlitValueLabel, r, c++, LabelFlags);
  faceAttribsLayout->addWidget(m_surfaceSiNNonlitValueEditorLayout, r, c++, 1, 3);
  ++r;
  c = 0;

  faceAttribsLayout->addWidget(m_surfaceSiNTransAngleLabel, r, c++, LabelFlags);
  faceAttribsLayout->addWidget(m_surfaceSiNTransAngleEditorLayout, r, c++);
  faceAttribsLayout->addWidget(m_surfaceSiNTransMagLabel, r, c++, LabelFlags);
  faceAttribsLayout->addWidget(m_surfaceSiNTransMagEditorLayout, r, c++);
  ++r;
  c = 0;

  faceAttribsLayout->addWidget(m_surfaceSiNTranslucenceLabel, r, c++, LabelFlags);
  faceAttribsLayout->addWidget(m_surfaceSiNTranslucenceEditorLayout, r, c++);
  faceAttribsLayout->addWidget(m_surfaceSiNRestitutionLabel, r, c++, LabelFlags);
  faceAttribsLayout->addWidget(m_surfaceSiNRestitutionEditorLayout, r, c++);
  ++r;
  c = 0;

  faceAttribsLayout->addWidget(m_surfaceSiNFrictionLabel, r, c++, LabelFlags);
  faceAttribsLayout->addWidget(m_surfaceSiNFrictionEditorLayout, r, c++);
  faceAttribsLayout->addWidget(m_surfaceSiNAnimTimeLabel, r, c++, LabelFlags);
  faceAttribsLayout->addWidget(m_surfaceSiNAnimTimeEditorLayout, r, c++);
  ++r;
  c = 0;

  faceAttribsLayout->addWidget(m_surfaceSiNDirectStyleLabel, r, c++, LabelFlags);
  faceAttribsLayout->addWidget(m_surfaceSiNDirectStyleEditorLayout, r, c++, 1, 3);
  ++r;
  c = 0;

  faceAttribsLayout->addWidget(m_surfaceSiNDirectLabel, r, c++, LabelFlags);
  faceAttribsLayout->addWidget(m_surfaceSiNDirectEditorLayout, r, c++);
  faceAttribsLayout->addWidget(m_surfaceSiNDirectAngleLabel, r, c++, LabelFlags);
  faceAttribsLayout->addWidget(m_surfaceSiNDirectAngleEditorLayout, r, c++);
  ++r;
  c = 0;

  faceAttribsLayout->addWidget(m_surfaceSiNExtDirectScaleLabel, r, c++, LabelFlags);
  faceAttribsLayout->addWidget(m_surfaceSiNExtDirectScaleEditorLayout, r, c++);
  faceAttribsLayout->addWidget(m_surfaceSiNExtPatchScaleLabel, r, c++, LabelFlags);
  faceAttribsLayout->addWidget(m_surfaceSiNExtPatchScaleEditorLayout, r, c++);
  ++r;
  c = 0;

  faceAttribsLayout->addWidget(m_surfaceSiNExtMinLightLabel, r, c++, LabelFlags);
  faceAttribsLayout->addWidget(m_surfaceSiNExtMinLightEditorLayout, r, c++);
  faceAttribsLayout->addWidget(m_surfaceSiNExtMaxLightLabel, r, c++, LabelFlags);
  faceAttribsLayout->addWidget(m_surfaceSiNExtMaxLightEditorLayout, r, c++);
  ++r;
  c = 0;

  faceAttribsLayout->addWidget(m_surfaceSiNExtLuxelScaleLabel, r, c++, LabelFlags);
  faceAttribsLayout->addWidget(m_surfaceSiNExtLuxelScaleEditorLayout, r, c++);
  faceAttribsLayout->addWidget(m_surfaceSiNExtMottleLabel, r, c++, LabelFlags);
  faceAttribsLayout->addWidget(m_surfaceSiNExtMottleEditorLayout, r, c++);
  ++r;
  c = 0;

  faceAttribsLayout->addWidget(m_sinExtFlagsLabel, r, c++, LabelFlags);
  faceAttribsLayout->addWidget(m_sinExtFlagsEditorLayout, r, c++, 1, 3);
  ++r;
  c = 0;


  faceAttribsLayout->setColumnStretch(1, 1);
  faceAttribsLayout->setColumnStretch(3, 1);

  auto* outerLayout = new QVBoxLayout{};
  outerLayout->setContentsMargins(0, 0, 0, 0);
  outerLayout->setSpacing(LayoutConstants::NarrowVMargin);
  outerLayout->addWidget(m_uvEditor, 1);
  outerLayout->addWidget(new BorderLine{});
  outerLayout->addLayout(faceAttribsLayout);

  setLayout(outerLayout);
}

void FaceAttribsEditor::bindEvents()
{
  connect(
    m_xOffsetEditor,
    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    this,
    &FaceAttribsEditor::xOffsetChanged);
  connect(
    m_yOffsetEditor,
    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    this,
    &FaceAttribsEditor::yOffsetChanged);
  connect(
    m_xScaleEditor,
    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    this,
    &FaceAttribsEditor::xScaleChanged);
  connect(
    m_yScaleEditor,
    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    this,
    &FaceAttribsEditor::yScaleChanged);
  connect(
    m_rotationEditor,
    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    this,
    &FaceAttribsEditor::rotationChanged);
  connect(
    m_surfaceValueEditor,
    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    this,
    &FaceAttribsEditor::surfaceValueChanged);
  connect(
    m_surfaceFlagsEditor,
    &FlagsPopupEditor::flagChanged,
    this,
    &FaceAttribsEditor::surfaceFlagChanged);
  connect(
    m_contentFlagsEditor,
    &FlagsPopupEditor::flagChanged,
    this,
    &FaceAttribsEditor::contentFlagChanged);
  connect(
    m_colorEditor, &QLineEdit::textEdited, this, &FaceAttribsEditor::colorValueChanged);
  connect(
    m_surfaceValueUnsetButton,
    &QAbstractButton::clicked,
    this,
    &FaceAttribsEditor::surfaceValueUnset);
  connect(
    m_surfaceFlagsUnsetButton,
    &QAbstractButton::clicked,
    this,
    &FaceAttribsEditor::surfaceFlagsUnset);
  connect(
    m_contentFlagsUnsetButton,
    &QAbstractButton::clicked,
    this,
    &FaceAttribsEditor::contentFlagsUnset);
  connect(
    m_colorUnsetButton,
    &QAbstractButton::clicked,
    this,
    &FaceAttribsEditor::colorValueUnset);
  connect(
    m_updateControlsSignalDelayer,
    &SignalDelayer::processSignal,
    this,
    &FaceAttribsEditor::updateControls);

  // SIN stuff
  connect(
    m_surfaceSiNNonlitValueEditor,
    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    this,
    &FaceAttribsEditor::sinNonlitValueChanged);
  connect(
    m_surfaceSiNNonlitValueUnsetButton,
    &QAbstractButton::clicked,
    this,
    &FaceAttribsEditor::sinNonlitValueUnset);
  connect(
    m_surfaceSiNTransAngleEditor,
    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    this,
    &FaceAttribsEditor::sinTransAngleChanged);
  connect(
    m_surfaceSiNTransAngleUnsetButton,
    &QAbstractButton::clicked,
    this,
    &FaceAttribsEditor::sinTransAngleUnset);
  connect(
    m_surfaceSiNTransMagEditor,
    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    this,
    &FaceAttribsEditor::sinTransMagChanged);
  connect(
    m_surfaceSiNTransMagUnsetButton,
    &QAbstractButton::clicked,
    this,
    &FaceAttribsEditor::sinTransMagUnset);
  connect(
    m_surfaceSiNTranslucenceEditor,
    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    this,
    &FaceAttribsEditor::sinTranslucenceChanged);
  connect(
    m_surfaceSiNTranslucenceUnsetButton,
    &QAbstractButton::clicked,
    this,
    &FaceAttribsEditor::sinTranslucenceUnset);
  connect(
    m_surfaceSiNRestitutionEditor,
    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    this,
    &FaceAttribsEditor::sinRestitutionChanged);
  connect(
    m_surfaceSiNRestitutionUnsetButton,
    &QAbstractButton::clicked,
    this,
    &FaceAttribsEditor::sinRestitutionUnset);
  connect(
    m_surfaceSiNFrictionEditor,
    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    this,
    &FaceAttribsEditor::sinFrictionChanged);
  connect(
    m_surfaceSiNFrictionUnsetButton,
    &QAbstractButton::clicked,
    this,
    &FaceAttribsEditor::sinFrictionUnset);
  connect(
    m_surfaceSiNAnimTimeEditor,
    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    this,
    &FaceAttribsEditor::sinAnimTimeChanged);
  connect(
    m_surfaceSiNAnimTimeUnsetButton,
    &QAbstractButton::clicked,
    this,
    &FaceAttribsEditor::sinAnimTimeUnset);
  connect(
    m_surfaceSiNDirectStyleEditor,
    &QLineEdit::textEdited,
    this,
    &FaceAttribsEditor::sinDirectStyleValueChanged);
  connect(
    m_surfaceSiNDirectStyleUnsetButton,
    &QAbstractButton::clicked,
    this,
    &FaceAttribsEditor::sinDirectStyleValueUnset);
  connect(
    m_surfaceSiNDirectEditor,
    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    this,
    &FaceAttribsEditor::sinDirectChanged);
  connect(
    m_surfaceSiNDirectUnsetButton,
    &QAbstractButton::clicked,
    this,
    &FaceAttribsEditor::sinDirectUnset);
  connect(
    m_surfaceSiNDirectAngleEditor,
    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    this,
    &FaceAttribsEditor::sinDirectAngleChanged);
  connect(
    m_surfaceSiNDirectAngleUnsetButton,
    &QAbstractButton::clicked,
    this,
    &FaceAttribsEditor::sinDirectAngleUnset);

  // SiN extended
  connect(
    m_surfaceSiNExtDirectScaleEditor,
    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    this,
    &FaceAttribsEditor::sinExtDirectScaleChanged);
  connect(
    m_surfaceSiNExtPatchScaleEditor,
    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    this,
    &FaceAttribsEditor::sinExtPatchScaleChanged);
  connect(
    m_surfaceSiNExtMinLightEditor,
    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    this,
    &FaceAttribsEditor::sinExtMinLightChanged);
  connect(
    m_surfaceSiNExtMaxLightEditor,
    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    this,
    &FaceAttribsEditor::sinExtMaxLightChanged);
  connect(
    m_surfaceSiNExtLuxelScaleEditor,
    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    this,
    &FaceAttribsEditor::sinExtLuxelScaleChanged);
  connect(
    m_surfaceSiNExtMottleEditor,
    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    this,
    &FaceAttribsEditor::sinExtMottleChanged);
  connect(
    m_sinExtFlagsUnsetButton,
    &QAbstractButton::clicked,
    this,
    &FaceAttribsEditor::sinExtFlagsUnset);
  connect(
    m_sinExtFlagsEditor,
    &FlagsPopupEditor::flagChanged,
    this,
    &FaceAttribsEditor::sinExtFlagChanged);
}

void FaceAttribsEditor::connectObservers()
{
  auto& map = m_document.map();

  m_notifierConnection += m_document.documentWasLoadedNotifier.connect(
    this, &FaceAttribsEditor::documentDidChange);
  m_notifierConnection += m_document.documentDidChangeNotifier.connect(
    this, &FaceAttribsEditor::documentDidChange);
  m_notifierConnection +=
    map.grid().gridDidChangeNotifier.connect(this, &FaceAttribsEditor::updateIncrements);
}

void FaceAttribsEditor::documentDidChange()
{
  updateControlsDelayed();
}

static void disableAndSetPlaceholder(QDoubleSpinBox* box, const QString& text)
{
  box->setSpecialValueText(text);
  box->setValue(box->minimum());
  box->setEnabled(false);
}

static void setValueOrMulti(QDoubleSpinBox* box, const bool multi, const double value)
{
  if (multi)
  {
    box->setSpecialValueText("multi");
    box->setValue(box->minimum());
  }
  else
  {
    box->setSpecialValueText("");
    box->setValue(value);
  }
}

void FaceAttribsEditor::updateControls()
{
  // block signals emitted when updating the editor values
  const auto blockXOffsetEditor = QSignalBlocker{m_xOffsetEditor};
  const auto blockYOffsetEditor = QSignalBlocker{m_yOffsetEditor};
  const auto blockRotationEditor = QSignalBlocker{m_rotationEditor};
  const auto blockXScaleEditor = QSignalBlocker{m_xScaleEditor};
  const auto blockYScaleEditor = QSignalBlocker{m_yScaleEditor};
  const auto blockSurfaceValueEditor = QSignalBlocker{m_surfaceValueEditor};
  const auto blockSurfaceFlagsEditor = QSignalBlocker{m_surfaceFlagsEditor};
  const auto blockContentFlagsEditor = QSignalBlocker{m_contentFlagsEditor};
  const auto blockColorEditor = QSignalBlocker{m_colorEditor};

  // SiN
  const auto blockSiNNonlitValueEditor = QSignalBlocker{m_surfaceSiNNonlitValueEditor};
  const auto blockSiNTransAngleEditor = QSignalBlocker{m_surfaceSiNTransAngleEditor};
  const auto blockSiNTransMagEditor = QSignalBlocker{m_surfaceSiNTransMagEditor};
  const auto blockSiNTranslucenceEditor = QSignalBlocker{m_surfaceSiNTranslucenceEditor};
  const auto blockSiNRestitutionEditor = QSignalBlocker{m_surfaceSiNRestitutionEditor};
  const auto blockSiNFrictionEditor = QSignalBlocker{m_surfaceSiNFrictionEditor};
  const auto blockSiNAnimTimeEditor = QSignalBlocker{m_surfaceSiNAnimTimeEditor};
  const auto blockSiNDirectStyleEditor = QSignalBlocker{m_surfaceSiNDirectStyleEditor};
  const auto blockSiNDirectEditor = QSignalBlocker{m_surfaceSiNDirectEditor};
  const auto blockSiNDirectAngleEditor = QSignalBlocker{m_surfaceSiNDirectAngleEditor};

  // Extended
  const auto blockSiNExtDirectScaleEditor =
    QSignalBlocker{m_surfaceSiNExtDirectScaleEditor};
  const auto blockSiNExtPatchScaleEditor =
    QSignalBlocker{m_surfaceSiNExtPatchScaleEditor};
  const auto blockSiNExtMinLightEditor = QSignalBlocker{m_surfaceSiNExtMinLightEditor};
  const auto blockSiNExtMaxLightEditor = QSignalBlocker{m_surfaceSiNExtMaxLightEditor};
  const auto blockSiNExtLuxelScaleEditor =
    QSignalBlocker{m_surfaceSiNExtLuxelScaleEditor};
  const auto blockSiNExtMottleEditor = QSignalBlocker{m_surfaceSiNExtMottleEditor};
  const auto blockSiNExtFlagsEditor = QSignalBlocker{m_sinExtFlagsEditor};

  if (hasSurfaceFlags())
  {
    showSurfaceFlagsEditor();
    const auto [values, labels, tooltips] = getSurfaceFlags();
    m_surfaceFlagsEditor->setFlags(values, labels, tooltips);
  }
  else
  {
    hideSurfaceFlagsEditor();
  }

  if (hasContentFlags())
  {
    showContentFlagsEditor();
    const auto [values, labels, tooltips] = getContentFlags();
    m_contentFlagsEditor->setFlags(values, labels, tooltips);
  }
  else
  {
    hideContentFlagsEditor();
  }

  if (hasColorAttribs())
  {
    showColorAttribEditor();
  }
  else
  {
    hideColorAttribEditor();
  }

  // SiN
  if (hasSiNAttributes())
  {
    const auto [values, labels, tooltips] = getExtendedFlags();
    m_sinExtFlagsEditor->setFlags(values, labels, tooltips);

    showSiNAttribEditor();
  }
  else
  {
    hideSiNAttribEditor();
  }

  const auto faceHandles = m_document.map().selection().allBrushFaces();
  if (!faceHandles.empty())
  {
    auto materialMulti = false;
    auto xOffsetMulti = false;
    auto yOffsetMulti = false;
    auto rotationMulti = false;
    auto xScaleMulti = false;
    auto yScaleMulti = false;
    auto surfaceValueMulti = false;
    auto colorValueMulti = false;

    const auto& firstFace = faceHandles[0].face();
    const auto& materialName = firstFace.attributes().materialName();
    const auto xOffset = firstFace.attributes().xOffset();
    const auto yOffset = firstFace.attributes().yOffset();
    const auto rotation = firstFace.attributes().rotation();
    const auto xScale = firstFace.attributes().xScale();
    const auto yScale = firstFace.attributes().yScale();
    auto setSurfaceFlags = firstFace.resolvedSurfaceFlags();
    auto setSurfaceContents = firstFace.resolvedSurfaceContents();
    auto mixedSurfaceFlags = 0;
    auto mixedSurfaceContents = 0;
    const auto surfaceValue = firstFace.resolvedSurfaceValue();
    const auto& colorValue = firstFace.attributes().color();
    auto hasSurfaceValue = firstFace.attributes().surfaceValue().has_value();
    auto hasSurfaceFlags = firstFace.attributes().surfaceFlags().has_value();
    auto hasSurfaceContents = firstFace.attributes().surfaceContents().has_value();
    auto hasColorValue = firstFace.attributes().hasColor();

    // SiN
    auto sinNonlitValueMulti = false;
    const auto& sinNonlitValue = firstFace.attributes().sinNonlitValue();
    auto hasSiNNonlitValue = firstFace.attributes().hasSiNNonlitValue();

    auto sinTransAngleMulti = false;
    const auto& sinTransAngle = firstFace.attributes().sinTransAngle();
    auto hasSiNTransAngle = firstFace.attributes().hasSiNTransAngle();

    auto sinTransMagMulti = false;
    const auto& sinTransMag = firstFace.attributes().sinTransMag();
    auto hasSiNTransMag = firstFace.attributes().hasSiNTransMag();

    auto sinTranslucenceMulti = false;
    const auto& sinTranslucence = firstFace.attributes().sinTranslucence();
    auto hasSiNTranslucence = firstFace.attributes().hasSiNTranslucence();

    auto sinRestitutionMulti = false;
    const auto& sinRestitution = firstFace.attributes().sinRestitution();
    auto hasSiNRestitution = firstFace.attributes().hasSiNRestitution();

    auto sinFrictionMulti = false;
    const auto& sinFriction = firstFace.attributes().sinFriction();
    auto hasSiNFriction = firstFace.attributes().hasSiNFriction();

    auto sinAnimTimeMulti = false;
    const auto& sinAnimTime = firstFace.attributes().sinAnimTime();
    auto hasSiNAnimTime = firstFace.attributes().hasSiNAnimTime();

    auto sinDirectStyleValueMulti = false;
    const auto& sinDirectStyleValue = firstFace.attributes().sinDirectStyle();
    auto hasSiNDirectStyleValue = firstFace.attributes().hasSiNDirectStyle();

    auto sinDirectMulti = false;
    const auto& sinDirect = firstFace.attributes().sinDirect();
    auto hasSiNDirect = firstFace.attributes().hasSiNDirect();

    auto sinDirectAngleMulti = false;
    const auto& sinDirectAngle = firstFace.attributes().sinDirectAngle();
    auto hasSiNDirectAngle = firstFace.attributes().hasSiNDirectAngle();

    // SiN extended
    auto sinExtDirectScaleMulti = false;
    const auto& sinExtDirectScale = firstFace.attributes().sinExtDirectScale();
    auto hasSiNExtDirectScale = firstFace.attributes().hasSiNExtDirectScale();

    auto sinExtPatchScaleMulti = false;
    const auto& sinExtPatchScale = firstFace.attributes().sinExtPatchScale();
    auto hasSiNExtPatchScale = firstFace.attributes().hasSiNExtPatchScale();

    auto sinExtMinLightMulti = false;
    const auto& sinExtMinLight = firstFace.attributes().sinExtMinLight();
    auto hasSiNExtMinLight = firstFace.attributes().hasSiNExtMinLight();

    auto sinExtMaxLightMulti = false;
    const auto& sinExtMaxLight = firstFace.attributes().sinExtMaxLight();
    auto hasSiNExtMaxLight = firstFace.attributes().hasSiNExtMaxLight();

    auto sinExtLuxelScaleMulti = false;
    const auto& sinExtLuxelScale = firstFace.attributes().sinExtLuxelScale();
    auto hasSiNExtLuxelScale = firstFace.attributes().hasSiNExtLuxelScale();

    auto sinExtMottleMulti = false;
    const auto& sinExtMottle = firstFace.attributes().sinExtMottle();
    auto hasSiNExtMottle = firstFace.attributes().hasSiNExtMottle();

    auto setExtFlags = firstFace.attributes().extendedFlags().value_or(0);
    auto hasExtFlags = firstFace.attributes().extendedFlags().has_value();
    auto mixedExtFlags = 0;

    for (size_t i = 1; i < faceHandles.size(); i++)
    {
      const auto& face = faceHandles[i].face();
      materialMulti |= (materialName != face.attributes().materialName());
      xOffsetMulti |= (xOffset != face.attributes().xOffset());
      yOffsetMulti |= (yOffset != face.attributes().yOffset());
      rotationMulti |= (rotation != face.attributes().rotation());
      xScaleMulti |= (xScale != face.attributes().xScale());
      yScaleMulti |= (yScale != face.attributes().yScale());
      surfaceValueMulti |= (surfaceValue != face.resolvedSurfaceValue());
      colorValueMulti |= (colorValue != face.attributes().color());
      hasSurfaceValue |= face.attributes().surfaceValue().has_value();
      hasSurfaceFlags |= face.attributes().surfaceFlags().has_value();
      hasSurfaceContents |= face.attributes().surfaceContents().has_value();
      hasColorValue |= face.attributes().hasColor();

      combineFlags(
        sizeof(int) * 8, face.resolvedSurfaceFlags(), setSurfaceFlags, mixedSurfaceFlags);
      combineFlags(
        sizeof(int) * 8,
        face.resolvedSurfaceContents(),
        setSurfaceContents,
        mixedSurfaceContents);

      // SiN
      sinNonlitValueMulti |= (sinNonlitValue != face.attributes().sinNonlitValue());
      hasSiNNonlitValue |= face.attributes().hasSiNNonlitValue();
      sinTransAngleMulti |= (sinTransAngle != face.attributes().sinTransAngle());
      hasSiNTransAngle |= face.attributes().hasSiNTransAngle();
      sinTransMagMulti |= (sinTransMag != face.attributes().sinTransMag());
      hasSiNTransMag |= face.attributes().hasSiNTransMag();
      sinTranslucenceMulti |= (sinTranslucence != face.attributes().sinTranslucence());
      hasSiNTranslucence |= face.attributes().hasSiNTranslucence();
      sinRestitutionMulti |= (sinRestitution != face.attributes().sinRestitution());
      hasSiNRestitution |= face.attributes().hasSiNRestitution();
      sinFrictionMulti |= (sinFriction != face.attributes().sinFriction());
      hasSiNFriction |= face.attributes().hasSiNFriction();
      sinAnimTimeMulti |= (sinAnimTime != face.attributes().sinAnimTime());
      hasSiNAnimTime |= face.attributes().hasSiNAnimTime();
      sinDirectStyleValueMulti |=
        (sinDirectStyleValue != face.attributes().sinDirectStyle());
      hasSiNDirectStyleValue |= face.attributes().hasSiNDirectStyle();
      sinDirectMulti |= (sinDirect != face.attributes().sinDirect());
      hasSiNDirect |= face.attributes().hasSiNDirect();
      sinDirectAngleMulti |= (sinDirectAngle != face.attributes().sinDirectAngle());
      hasSiNDirectAngle |= face.attributes().hasSiNDirectAngle();

      // SiN extended
      hasExtFlags |= face.attributes().extendedFlags().has_value();

      sinExtDirectScaleMulti |=
        (sinExtDirectScale != face.attributes().sinExtDirectScale());
      hasSiNExtDirectScale |= face.attributes().hasSiNExtDirectScale();
      sinExtPatchScaleMulti |= (sinExtPatchScale != face.attributes().sinExtPatchScale());
      hasSiNExtPatchScale |= face.attributes().hasSiNExtPatchScale();
      sinExtMinLightMulti |= (sinExtMinLight != face.attributes().sinExtMinLight());
      hasSiNExtMinLight |= face.attributes().hasSiNExtMinLight();
      sinExtMaxLightMulti |= (sinExtMaxLight != face.attributes().sinExtMaxLight());
      hasSiNExtMaxLight |= face.attributes().hasSiNExtMaxLight();
      sinExtLuxelScaleMulti |= (sinExtLuxelScale != face.attributes().sinExtLuxelScale());
      hasSiNExtLuxelScale |= face.attributes().hasSiNExtLuxelScale();
      sinExtMottleMulti |= (sinExtMottle != face.attributes().sinExtMottle());
      hasSiNExtMottle |= face.attributes().hasSiNExtMottle();

      combineFlags(
        sizeof(int) * 8,
        face.attributes().extendedFlags().value_or(0),
        setExtFlags,
        mixedExtFlags);
    }

    m_xOffsetEditor->setEnabled(true);
    m_yOffsetEditor->setEnabled(true);
    m_rotationEditor->setEnabled(true);
    m_xScaleEditor->setEnabled(true);
    m_yScaleEditor->setEnabled(true);
    m_surfaceValueEditor->setEnabled(true);
    m_surfaceFlagsEditor->setEnabled(true);
    m_contentFlagsEditor->setEnabled(true);
    m_colorEditor->setEnabled(true);

    if (materialMulti)
    {
      m_materialName->setText("multi");
      m_materialName->setEnabled(false);
      m_textureSize->setText("multi");
      m_textureSize->setEnabled(false);
    }
    else
    {
      if (materialName == mdl::BrushFaceAttributes::NoMaterialName)
      {
        m_materialName->setText("none");
        m_materialName->setEnabled(false);
        m_textureSize->setText("");
        m_textureSize->setEnabled(false);
      }
      else
      {
        if (const auto* texture = getTexture(firstFace.material()))
        {
          m_materialName->setText(QString::fromStdString(materialName));
          m_textureSize->setText(
            QStringLiteral("%1 * %2").arg(texture->width()).arg(texture->height()));
          m_materialName->setEnabled(true);
          m_textureSize->setEnabled(true);
        }
        else
        {
          m_materialName->setText(QString::fromStdString(materialName) + " (not found)");
          m_materialName->setEnabled(false);
          m_textureSize->setEnabled(false);
        }
      }
    }
    setValueOrMulti(m_xOffsetEditor, xOffsetMulti, double(xOffset));
    setValueOrMulti(m_yOffsetEditor, yOffsetMulti, double(yOffset));
    setValueOrMulti(m_rotationEditor, rotationMulti, double(rotation));
    setValueOrMulti(m_xScaleEditor, xScaleMulti, double(xScale));
    setValueOrMulti(m_yScaleEditor, yScaleMulti, double(yScale));
    setValueOrMulti(m_surfaceValueEditor, surfaceValueMulti, double(surfaceValue));
    if (hasColorValue)
    {
      if (colorValueMulti)
      {
        m_colorEditor->setPlaceholderText("multi");
        m_colorEditor->setText("");
      }
      else
      {
        m_colorEditor->setPlaceholderText("");
        m_colorEditor->setText(QString::fromStdString(colorValue->to<RgbB>().toString()));
      }
    }
    else
    {
      m_colorEditor->setPlaceholderText("");
      m_colorEditor->setText("");
    }
    m_surfaceFlagsEditor->setFlagValue(setSurfaceFlags, mixedSurfaceFlags);
    m_contentFlagsEditor->setFlagValue(setSurfaceContents, mixedSurfaceContents);

    m_surfaceValueUnsetButton->setEnabled(hasSurfaceValue);
    m_surfaceFlagsUnsetButton->setEnabled(hasSurfaceFlags);
    m_contentFlagsUnsetButton->setEnabled(hasSurfaceContents);
    m_colorUnsetButton->setEnabled(hasColorValue);

    // SiN
    setValueOrMulti(
      m_surfaceSiNNonlitValueEditor,
      sinNonlitValueMulti,
      double(sinNonlitValue.value_or(mdl::BrushFaceAttributes::SiNDefaultNonLitValue)));
    m_surfaceSiNNonlitValueUnsetButton->setEnabled(hasSiNNonlitValue);
    setValueOrMulti(
      m_surfaceSiNTransAngleEditor,
      sinTransAngleMulti,
      double(sinTransAngle.value_or(0)));
    m_surfaceSiNTransAngleUnsetButton->setEnabled(hasSiNTransAngle);
    setValueOrMulti(
      m_surfaceSiNTransMagEditor, sinTransMagMulti, double(sinTransMag.value_or(0)));
    m_surfaceSiNTransMagUnsetButton->setEnabled(hasSiNTransMag);
    setValueOrMulti(
      m_surfaceSiNTranslucenceEditor,
      sinTranslucenceMulti,
      double(sinTranslucence.value_or(0)));
    m_surfaceSiNTranslucenceUnsetButton->setEnabled(hasSiNTranslucence);
    setValueOrMulti(
      m_surfaceSiNRestitutionEditor,
      sinRestitutionMulti,
      double(sinRestitution.value_or(0)));
    m_surfaceSiNRestitutionUnsetButton->setEnabled(hasSiNRestitution);
    setValueOrMulti(
      m_surfaceSiNFrictionEditor,
      sinFrictionMulti,
      double(sinFriction.value_or(mdl::BrushFaceAttributes::SiNDefaultFriction)));
    m_surfaceSiNFrictionUnsetButton->setEnabled(hasSiNFriction);
    setValueOrMulti(
      m_surfaceSiNAnimTimeEditor,
      sinAnimTimeMulti,
      double(sinAnimTime.value_or(mdl::BrushFaceAttributes::SiNDefaultAnimTime)));
    m_surfaceSiNAnimTimeUnsetButton->setEnabled(hasSiNAnimTime);
    if (hasSiNDirectStyleValue)
    {
      if (sinDirectStyleValueMulti)
      {
        m_surfaceSiNDirectStyleEditor->setPlaceholderText("multi");
        m_surfaceSiNDirectStyleEditor->setText("");
      }
      else
      {
        m_surfaceSiNDirectStyleEditor->setPlaceholderText("");
        m_surfaceSiNDirectStyleEditor->setText(
          QString::fromStdString(*sinDirectStyleValue));
      }
    }
    else
    {
      m_surfaceSiNDirectStyleEditor->setPlaceholderText("");
      m_surfaceSiNDirectStyleEditor->setText("");
    }
    m_surfaceSiNDirectStyleUnsetButton->setEnabled(hasSiNDirectStyleValue);
    setValueOrMulti(
      m_surfaceSiNDirectEditor, sinDirectMulti, double(sinDirect.value_or(0)));
    m_surfaceSiNDirectUnsetButton->setEnabled(hasSiNDirect);
    setValueOrMulti(
      m_surfaceSiNDirectAngleEditor,
      sinDirectAngleMulti,
      double(sinDirectAngle.value_or(0)));
    m_surfaceSiNDirectAngleUnsetButton->setEnabled(hasSiNDirectAngle);

    m_surfaceSiNNonlitValueEditor->setEnabled(true);
    m_surfaceSiNTransAngleEditor->setEnabled(true);
    m_surfaceSiNTransMagEditor->setEnabled(true);
    m_surfaceSiNTranslucenceEditor->setEnabled(true);
    m_surfaceSiNRestitutionEditor->setEnabled(true);
    m_surfaceSiNFrictionEditor->setEnabled(true);
    m_surfaceSiNAnimTimeEditor->setEnabled(true);
    m_surfaceSiNDirectStyleEditor->setEnabled(true);
    m_surfaceSiNDirectEditor->setEnabled(true);
    m_surfaceSiNDirectAngleEditor->setEnabled(true);

    // SiN Extended
    setValueOrMulti(
      m_surfaceSiNExtDirectScaleEditor,
      sinExtDirectScaleMulti,
      double(
        sinExtDirectScale.value_or(mdl::BrushFaceAttributes::SiNDefaultExtDirectScale)));
    m_surfaceSiNExtDirectScaleUnsetButton->setEnabled(hasSiNExtDirectScale);
    setValueOrMulti(
      m_surfaceSiNExtPatchScaleEditor,
      sinExtPatchScaleMulti,
      double(
        sinExtPatchScale.value_or(mdl::BrushFaceAttributes::SiNDefaultExtPatchScale)));
    m_surfaceSiNExtPatchScaleUnsetButton->setEnabled(hasSiNExtPatchScale);
    setValueOrMulti(
      m_surfaceSiNExtMinLightEditor,
      sinExtMinLightMulti,
      double(sinExtMinLight.value_or(0)));
    m_surfaceSiNExtMinLightUnsetButton->setEnabled(hasSiNExtMinLight);
    setValueOrMulti(
      m_surfaceSiNExtMaxLightEditor,
      sinExtMaxLightMulti,
      double(sinExtMaxLight.value_or(mdl::BrushFaceAttributes::SiNDefaultExtMaxLight)));
    m_surfaceSiNExtMaxLightUnsetButton->setEnabled(hasSiNExtMaxLight);
    setValueOrMulti(
      m_surfaceSiNExtLuxelScaleEditor,
      sinExtLuxelScaleMulti,
      double(
        sinExtLuxelScale.value_or(mdl::BrushFaceAttributes::SiNDefaultExtLuxelScale)));
    m_surfaceSiNExtLuxelScaleUnsetButton->setEnabled(hasSiNExtLuxelScale);
    setValueOrMulti(
      m_surfaceSiNExtMottleEditor,
      sinExtMottleMulti,
      double(sinExtMottle.value_or(mdl::BrushFaceAttributes::SiNDefaultExtMottle)));
    m_surfaceSiNExtMottleUnsetButton->setEnabled(hasSiNExtMottle);

    m_surfaceSiNExtDirectScaleEditor->setEnabled(true);
    m_surfaceSiNExtPatchScaleEditor->setEnabled(true);
    m_surfaceSiNExtMinLightEditor->setEnabled(true);
    m_surfaceSiNExtMaxLightEditor->setEnabled(true);
    m_surfaceSiNExtLuxelScaleEditor->setEnabled(true);
    m_surfaceSiNExtMottleEditor->setEnabled(true);

    m_sinExtFlagsEditor->setEnabled(true);
    m_sinExtFlagsEditor->setFlagValue(setExtFlags, mixedExtFlags);
  }
  else
  {
    disableAndSetPlaceholder(m_xOffsetEditor, "n/a");
    disableAndSetPlaceholder(m_yOffsetEditor, "n/a");
    disableAndSetPlaceholder(m_xScaleEditor, "n/a");
    disableAndSetPlaceholder(m_yScaleEditor, "n/a");
    disableAndSetPlaceholder(m_rotationEditor, "n/a");
    disableAndSetPlaceholder(m_surfaceValueEditor, "n/a");

    m_surfaceFlagsEditor->setEnabled(false);
    m_contentFlagsEditor->setEnabled(false);
    m_colorEditor->setText("");
    m_colorEditor->setPlaceholderText("n/a");
    m_colorEditor->setEnabled(false);

    m_surfaceValueUnsetButton->setEnabled(false);
    m_surfaceFlagsUnsetButton->setEnabled(false);
    m_contentFlagsUnsetButton->setEnabled(false);
    m_colorUnsetButton->setEnabled(false);

    // SiN
    disableAndSetPlaceholder(m_surfaceSiNNonlitValueEditor, "n/a");
    disableAndSetPlaceholder(m_surfaceSiNTransAngleEditor, "n/a");
    disableAndSetPlaceholder(m_surfaceSiNTransMagEditor, "n/a");
    disableAndSetPlaceholder(m_surfaceSiNTranslucenceEditor, "n/a");
    disableAndSetPlaceholder(m_surfaceSiNRestitutionEditor, "n/a");
    disableAndSetPlaceholder(m_surfaceSiNFrictionEditor, "n/a");
    disableAndSetPlaceholder(m_surfaceSiNAnimTimeEditor, "n/a");
    m_surfaceSiNDirectStyleEditor->setText("");
    m_surfaceSiNDirectStyleEditor->setPlaceholderText("n/a");
    m_surfaceSiNDirectStyleEditor->setEnabled(false);
    m_surfaceSiNDirectStyleUnsetButton->setEnabled(false);
    disableAndSetPlaceholder(m_surfaceSiNDirectEditor, "n/a");
    disableAndSetPlaceholder(m_surfaceSiNDirectAngleEditor, "n/a");

    // SiN extended
    disableAndSetPlaceholder(m_surfaceSiNExtDirectScaleEditor, "n/a");
    disableAndSetPlaceholder(m_surfaceSiNExtPatchScaleEditor, "n/a");
    disableAndSetPlaceholder(m_surfaceSiNExtMinLightEditor, "n/a");
    disableAndSetPlaceholder(m_surfaceSiNExtMaxLightEditor, "n/a");
    disableAndSetPlaceholder(m_surfaceSiNExtLuxelScaleEditor, "n/a");
    disableAndSetPlaceholder(m_surfaceSiNExtMottleEditor, "n/a");
    m_sinExtFlagsEditor->setEnabled(false);
    m_sinExtFlagsUnsetButton->setEnabled(false);
  }
}

void FaceAttribsEditor::updateControlsDelayed()
{
  m_updateControlsSignalDelayer->queueSignal();
}

bool FaceAttribsEditor::hasSurfaceFlags() const
{
  const auto& gameInfo = m_document.map().gameInfo();
  return !gameInfo.gameConfig.faceAttribsConfig.surfaceFlags.flags.empty();
}

bool FaceAttribsEditor::hasContentFlags() const
{
  const auto& gameInfo = m_document.map().gameInfo();
  return !gameInfo.gameConfig.faceAttribsConfig.contentFlags.flags.empty();
}

void FaceAttribsEditor::showSurfaceFlagsEditor()
{
  m_surfaceValueLabel->show();
  m_surfaceValueEditorLayout->show();
  m_surfaceFlagsLabel->show();
  m_surfaceFlagsEditorLayout->show();
}

void FaceAttribsEditor::showContentFlagsEditor()
{
  m_contentFlagsLabel->show();
  m_contentFlagsEditorLayout->show();
}

void FaceAttribsEditor::hideSurfaceFlagsEditor()
{
  m_surfaceValueLabel->hide();
  m_surfaceValueEditorLayout->hide();
  m_surfaceFlagsLabel->hide();
  m_surfaceFlagsEditorLayout->hide();
}

void FaceAttribsEditor::hideContentFlagsEditor()
{
  m_contentFlagsLabel->hide();
  m_contentFlagsEditorLayout->hide();
}

bool FaceAttribsEditor::hasColorAttribs() const
{
  return m_document.map().worldNode().mapFormat() == mdl::MapFormat::Daikatana
         || hasSiNAttributes();
}

void FaceAttribsEditor::showColorAttribEditor()
{
  m_colorLabel->show();
  m_colorEditorLayout->show();
}

void FaceAttribsEditor::hideColorAttribEditor()
{
  m_colorLabel->hide();
  m_colorEditorLayout->hide();
}

// SiN
bool FaceAttribsEditor::hasSiNAttributes() const
{
  return m_document.map().worldNode().mapFormat() == mdl::MapFormat::SiN
         || m_document.map().worldNode().mapFormat() == mdl::MapFormat::SiN_Valve;
}

void FaceAttribsEditor::showSiNAttribEditor()
{
  m_surfaceSiNNonlitValueLabel->show();
  m_surfaceSiNNonlitValueEditorLayout->show();
  m_surfaceSiNTransAngleLabel->show();
  m_surfaceSiNTransAngleEditorLayout->show();
  m_surfaceSiNTransMagLabel->show();
  m_surfaceSiNTransMagEditorLayout->show();
  m_surfaceSiNTranslucenceLabel->show();
  m_surfaceSiNTranslucenceEditorLayout->show();
  m_surfaceSiNRestitutionLabel->show();
  m_surfaceSiNRestitutionEditorLayout->show();
  m_surfaceSiNFrictionLabel->show();
  m_surfaceSiNFrictionEditorLayout->show();
  m_surfaceSiNAnimTimeLabel->show();
  m_surfaceSiNAnimTimeEditorLayout->show();
  m_surfaceSiNDirectStyleLabel->show();
  m_surfaceSiNDirectStyleEditorLayout->show();
  m_surfaceSiNDirectLabel->show();
  m_surfaceSiNDirectEditorLayout->show();
  m_surfaceSiNDirectAngleLabel->show();
  m_surfaceSiNDirectAngleEditorLayout->show();

  m_surfaceSiNExtDirectScaleLabel->show();
  m_surfaceSiNExtDirectScaleEditorLayout->show();
  m_surfaceSiNExtPatchScaleLabel->show();
  m_surfaceSiNExtPatchScaleEditorLayout->show();
  m_surfaceSiNExtMinLightLabel->show();
  m_surfaceSiNExtMinLightEditorLayout->show();
  m_surfaceSiNExtMaxLightLabel->show();
  m_surfaceSiNExtMaxLightEditorLayout->show();
  m_surfaceSiNExtLuxelScaleLabel->show();
  m_surfaceSiNExtLuxelScaleEditorLayout->show();
  m_surfaceSiNExtMottleLabel->show();
  m_surfaceSiNExtMottleEditorLayout->show();
  m_sinExtFlagsLabel->show();
  m_sinExtFlagsEditorLayout->show();
}

void FaceAttribsEditor::hideSiNAttribEditor()
{
  m_surfaceSiNNonlitValueLabel->hide();
  m_surfaceSiNNonlitValueEditorLayout->hide();
  m_surfaceSiNTransAngleLabel->hide();
  m_surfaceSiNTransAngleEditorLayout->hide();
  m_surfaceSiNTransMagLabel->hide();
  m_surfaceSiNTransMagEditorLayout->hide();
  m_surfaceSiNTranslucenceLabel->hide();
  m_surfaceSiNTranslucenceEditorLayout->hide();
  m_surfaceSiNRestitutionLabel->hide();
  m_surfaceSiNRestitutionEditorLayout->hide();
  m_surfaceSiNFrictionLabel->hide();
  m_surfaceSiNFrictionEditorLayout->hide();
  m_surfaceSiNAnimTimeLabel->hide();
  m_surfaceSiNAnimTimeEditorLayout->hide();
  m_surfaceSiNDirectStyleLabel->hide();
  m_surfaceSiNDirectStyleEditorLayout->hide();
  m_surfaceSiNDirectLabel->hide();
  m_surfaceSiNDirectEditorLayout->hide();
  m_surfaceSiNDirectAngleLabel->hide();
  m_surfaceSiNDirectAngleEditorLayout->hide();

  m_surfaceSiNExtDirectScaleLabel->hide();
  m_surfaceSiNExtDirectScaleEditorLayout->hide();
  m_surfaceSiNExtPatchScaleLabel->hide();
  m_surfaceSiNExtPatchScaleEditorLayout->hide();
  m_surfaceSiNExtMinLightLabel->hide();
  m_surfaceSiNExtMinLightEditorLayout->hide();
  m_surfaceSiNExtMaxLightLabel->hide();
  m_surfaceSiNExtMaxLightEditorLayout->hide();
  m_surfaceSiNExtLuxelScaleLabel->hide();
  m_surfaceSiNExtLuxelScaleEditorLayout->hide();
  m_surfaceSiNExtMottleLabel->hide();
  m_surfaceSiNExtMottleEditorLayout->hide();
  m_sinExtFlagsLabel->hide();
  m_sinExtFlagsEditorLayout->hide();
}

namespace
{
std::tuple<QList<int>, QStringList, QStringList> getFlags(
  const std::vector<mdl::FlagConfig>& flags)
{
  auto values = QList<int>{};
  auto names = QStringList{};
  auto descriptions = QStringList{};

  for (const auto& flag : flags)
  {
    values.push_back(flag.value);
    names.push_back(QString::fromStdString(flag.name));
    descriptions.push_back(QString::fromStdString(flag.description));
  }

  return {std::move(values), std::move(names), std::move(descriptions)};
}
} // namespace

std::tuple<QList<int>, QStringList, QStringList> FaceAttribsEditor::getSurfaceFlags()
  const
{
  const auto& gameInfo = m_document.map().gameInfo();
  const auto& surfaceFlags = gameInfo.gameConfig.faceAttribsConfig.surfaceFlags;
  return getFlags(surfaceFlags.flags);
}

std::tuple<QList<int>, QStringList, QStringList> FaceAttribsEditor::getContentFlags()
  const
{
  const auto& gameInfo = m_document.map().gameInfo();
  const auto& contentFlags = gameInfo.gameConfig.faceAttribsConfig.contentFlags;
  return getFlags(contentFlags.flags);
}

// SiN
std::tuple<QList<int>, QStringList, QStringList> FaceAttribsEditor::getExtendedFlags()
  const
{
  const auto& gameInfo = m_document.map().gameInfo();
  const auto& extendedFlags = gameInfo.gameConfig.faceAttribsConfig.extendedFlags;
  return getFlags(extendedFlags.flags);
}

} // namespace tb::ui
