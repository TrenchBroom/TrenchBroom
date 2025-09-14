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

#include "FaceAttribsEditor.h"

#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QVBoxLayout>
#include <QtGlobal>

#include "Color.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/Game.h"
#include "mdl/GameConfig.h"
#include "mdl/Grid.h"
#include "mdl/Map.h"
#include "mdl/MapFormat.h"
#include "mdl/Map_Brushes.h"
#include "mdl/Material.h"
#include "mdl/Texture.h"
#include "mdl/UpdateBrushFaceAttributes.h"
#include "mdl/WorldNode.h"
#include "ui/BorderLine.h"
#include "ui/FlagsPopupEditor.h"
#include "ui/MapDocument.h"
#include "ui/QtUtils.h"
#include "ui/SignalDelayer.h"
#include "ui/SpinControl.h"
#include "ui/UVEditor.h"
#include "ui/ViewConstants.h"
#include "ui/ViewUtils.h"

#include "kdl/string_format.h"
#include "kdl/string_utils.h"

#include "vm/vec_io.h" // IWYU pragma: keep

#include <string>

namespace tb::ui
{

FaceAttribsEditor::FaceAttribsEditor(
  MapDocument& document, GLContextManager& contextManager, QWidget* parent)
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
    if (const auto color = Color::parse(str))
    {
      if (!setBrushFaceAttributes(map, {.color = *color}))
      {
        updateControls();
      }
    }
  }
  else
  {
    if (!setBrushFaceAttributes(map, {.color = Color{}}))
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

  if (!setBrushFaceAttributes(map, {.color = std::nullopt}))
  {
    updateControls();
  }
}

void FaceAttribsEditor::updateIncrements()
{
  const auto& map = m_document.map();
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

void FaceAttribsEditor::createGui(GLContextManager& contextManager)
{
  m_uvEditor = new UVEditor{m_document, contextManager};

  auto* materialNameLabel = new QLabel{"Material"};
  makeEmphasized(materialNameLabel);
  m_materialName = new QLabel{"none"};
  m_materialName->setTextInteractionFlags(Qt::TextSelectableByMouse);

  auto* textureSizeLabel = new QLabel{"Size"};
  makeEmphasized(textureSizeLabel);
  m_textureSize = new QLabel{""};

  const auto max = std::numeric_limits<double>::max();
  const auto min = -max;

  auto* xOffsetLabel = new QLabel{"X Offset"};
  makeEmphasized(xOffsetLabel);
  m_xOffsetEditor = new SpinControl{};
  m_xOffsetEditor->setRange(min, max);
  m_xOffsetEditor->setDigits(0, 6);

  auto* yOffsetLabel = new QLabel{"Y Offset"};
  makeEmphasized(yOffsetLabel);
  m_yOffsetEditor = new SpinControl{};
  m_yOffsetEditor->setRange(min, max);
  m_yOffsetEditor->setDigits(0, 6);

  auto* xScaleLabel = new QLabel{"X Scale"};
  makeEmphasized(xScaleLabel);
  m_xScaleEditor = new SpinControl{};
  m_xScaleEditor->setRange(min, max);
  m_xScaleEditor->setIncrements(0.1, 0.25, 0.01);
  m_xScaleEditor->setDigits(0, 6);

  auto* yScaleLabel = new QLabel{"Y Scale"};
  makeEmphasized(yScaleLabel);
  m_yScaleEditor = new SpinControl{};
  m_yScaleEditor->setRange(min, max);
  m_yScaleEditor->setIncrements(0.1, 0.25, 0.01);
  m_yScaleEditor->setDigits(0, 6);

  auto* rotationLabel = new QLabel{"Angle"};
  makeEmphasized(rotationLabel);
  m_rotationEditor = new SpinControl{};
  m_rotationEditor->setRange(min, max);
  m_rotationEditor->setDigits(0, 6);

  m_surfaceValueLabel = new QLabel{"Value"};
  makeEmphasized(m_surfaceValueLabel);
  m_surfaceValueEditor = new SpinControl{};
  m_surfaceValueEditor->setRange(min, max);
  m_surfaceValueEditor->setIncrements(1.0, 10.0, 100.0);
  m_surfaceValueEditor->setDigits(0, 6);
  m_surfaceValueUnsetButton =
    createBitmapButton("ResetUV.svg", tr("Unset surface value"));
  m_surfaceValueEditorLayout =
    createUnsetButtonLayout(m_surfaceValueEditor, m_surfaceValueUnsetButton);

  m_surfaceFlagsLabel = new QLabel{"Surface"};
  makeEmphasized(m_surfaceFlagsLabel);
  m_surfaceFlagsEditor = new FlagsPopupEditor{2};
  m_surfaceFlagsUnsetButton =
    createBitmapButton("ResetUV.svg", tr("Unset surface flags"));
  m_surfaceFlagsEditorLayout =
    createUnsetButtonLayout(m_surfaceFlagsEditor, m_surfaceFlagsUnsetButton);

  m_contentFlagsLabel = new QLabel{"Content"};
  makeEmphasized(m_contentFlagsLabel);
  m_contentFlagsEditor = new FlagsPopupEditor{2};
  m_contentFlagsUnsetButton =
    createBitmapButton("ResetUV.svg", tr("Unset content flags"));
  m_contentFlagsEditorLayout =
    createUnsetButtonLayout(m_contentFlagsEditor, m_contentFlagsUnsetButton);

  m_colorLabel = new QLabel{"Color"};
  makeEmphasized(m_colorLabel);
  m_colorEditor = new QLineEdit{};
  m_colorUnsetButton = createBitmapButton("ResetUV.svg", tr("Unset color"));
  m_colorEditorLayout = createUnsetButtonLayout(m_colorEditor, m_colorUnsetButton);

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
}

void FaceAttribsEditor::connectObservers()
{
  auto& map = m_document.map();
  m_notifierConnection +=
    map.mapWasCreatedNotifier.connect(this, &FaceAttribsEditor::mapWasCreated);
  m_notifierConnection +=
    map.mapWasLoadedNotifier.connect(this, &FaceAttribsEditor::mapWasLoaded);
  m_notifierConnection +=
    map.nodesDidChangeNotifier.connect(this, &FaceAttribsEditor::nodesDidChange);
  m_notifierConnection += map.brushFacesDidChangeNotifier.connect(
    this, &FaceAttribsEditor::brushFacesDidChange);
  m_notifierConnection +=
    map.selectionDidChangeNotifier.connect(this, &FaceAttribsEditor::selectionDidChange);
  m_notifierConnection += map.materialCollectionsDidChangeNotifier.connect(
    this, &FaceAttribsEditor::materialCollectionsDidChange);
  m_notifierConnection +=
    map.grid().gridDidChangeNotifier.connect(this, &FaceAttribsEditor::updateIncrements);
}

void FaceAttribsEditor::mapWasCreated(mdl::Map&)
{
  updateControls();
}

void FaceAttribsEditor::mapWasLoaded(mdl::Map&)
{
  updateControls();
}

void FaceAttribsEditor::nodesDidChange(const std::vector<mdl::Node*>&)
{
  updateControlsDelayed();
}

void FaceAttribsEditor::brushFacesDidChange(const std::vector<mdl::BrushFaceHandle>&)
{
  updateControlsDelayed();
}

void FaceAttribsEditor::selectionDidChange(const mdl::SelectionChange&)
{
  updateControlsDelayed();
}

void FaceAttribsEditor::materialCollectionsDidChange()
{
  updateControls();
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

  const auto& map = m_document.map();
  const auto faceHandles = map.selection().allBrushFaces();
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
    const auto colorValue = firstFace.attributes().color();
    auto hasSurfaceValue = firstFace.attributes().surfaceValue().has_value();
    auto hasSurfaceFlags = firstFace.attributes().surfaceFlags().has_value();
    auto hasSurfaceContents = firstFace.attributes().surfaceContents().has_value();
    auto hasColorValue = firstFace.attributes().hasColor();

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
        m_colorEditor->setText(QString::fromStdString(kdl::str_to_string(*colorValue)));
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
  }
}

void FaceAttribsEditor::updateControlsDelayed()
{
  m_updateControlsSignalDelayer->queueSignal();
}

bool FaceAttribsEditor::hasSurfaceFlags() const
{
  const auto& map = m_document.map();
  const auto game = map.game();
  return !game->config().faceAttribsConfig.surfaceFlags.flags.empty();
}

bool FaceAttribsEditor::hasContentFlags() const
{
  const auto& map = m_document.map();
  const auto game = map.game();
  return !game->config().faceAttribsConfig.contentFlags.flags.empty();
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
  const auto& map = m_document.map();
  return map.world()->mapFormat() == mdl::MapFormat::Daikatana;
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
  const auto& map = m_document.map();
  const auto game = map.game();
  const auto& surfaceFlags = game->config().faceAttribsConfig.surfaceFlags;
  return getFlags(surfaceFlags.flags);
}

std::tuple<QList<int>, QStringList, QStringList> FaceAttribsEditor::getContentFlags()
  const
{
  const auto& map = m_document.map();
  const auto game = map.game();
  const auto& contentFlags = game->config().faceAttribsConfig.contentFlags;
  return getFlags(contentFlags.flags);
}

} // namespace tb::ui
