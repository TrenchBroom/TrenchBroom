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

#include "ui/UvEditor.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QToolButton>
#include <QtGlobal>

#include "mdl/GameConfig.h"
#include "mdl/GameInfo.h" // IWYU pragma: keep
#include "mdl/Map.h"
#include "mdl/Map_Brushes.h"
#include "mdl/UpdateBrushFaceAttributes.h"
#include "ui/BitmapButton.h"
#include "ui/MapDocument.h"
#include "ui/QStyleUtils.h"
#include "ui/UvView.h"
#include "ui/ViewConstants.h"

namespace tb::ui
{

UvEditor::UvEditor(AppController& appController, MapDocument& document, QWidget* parent)
  : QWidget{parent}
  , m_document{document}
{
  createGui(appController);
  connectObservers();
}

bool UvEditor::cancelMouseDrag()
{
  return m_uvView->cancelDrag();
}

void UvEditor::updateButtons()
{
  const bool enabled = !m_document.map().selection().allBrushFaces().empty();

  m_resetUvButton->setEnabled(enabled);
  m_resetUvToWorldButton->setEnabled(enabled);
  m_flipUAxisButton->setEnabled(enabled);
  m_flipVAxisButton->setEnabled(enabled);
  m_rotateUvCCWButton->setEnabled(enabled);
  m_rotateUvCWButton->setEnabled(enabled);
}

void UvEditor::createGui(AppController& appController)
{
  m_uvView = new UvView{appController, m_document};

  m_resetUvButton = createBitmapButton("ResetUV.svg", tr("Reset UV alignment"), this);
  m_resetUvToWorldButton = createBitmapButton(
    "ResetUVToWorld.svg", tr("Reset UV alignment to world aligned"), this);
  m_flipUAxisButton = createBitmapButton("FlipUAxis.svg", tr("Flip U axis"), this);
  m_flipVAxisButton = createBitmapButton("FlipVAxis.svg", tr("Flip V axis"), this);
  m_rotateUvCCWButton =
    createBitmapButton("RotateUVCCW.svg", tr("Rotate UV 90° counter-clockwise"), this);
  m_rotateUvCWButton =
    createBitmapButton("RotateUVCW.svg", tr("Rotate UV 90° clockwise"), this);

  connect(m_resetUvButton, &QAbstractButton::clicked, this, &UvEditor::resetUvClicked);
  connect(
    m_resetUvToWorldButton,
    &QAbstractButton::clicked,
    this,
    &UvEditor::resetUvToWorldClicked);
  connect(m_flipUAxisButton, &QAbstractButton::clicked, this, &UvEditor::flipUvHClicked);
  connect(m_flipVAxisButton, &QAbstractButton::clicked, this, &UvEditor::flipUvVClicked);
  connect(
    m_rotateUvCCWButton, &QAbstractButton::clicked, this, &UvEditor::rotateUvCCWClicked);
  connect(
    m_rotateUvCWButton, &QAbstractButton::clicked, this, &UvEditor::rotateUvCWClicked);

  auto* gridLabel = new QLabel{"Grid "};
  setEmphasizedStyle(gridLabel);
  m_xSubDivisionEditor = new QSpinBox{};
  m_xSubDivisionEditor->setRange(1, 16);
  m_xSubDivisionEditor->setValue(1);

  m_ySubDivisionEditor = new QSpinBox{};
  m_ySubDivisionEditor->setRange(1, 16);
  m_ySubDivisionEditor->setValue(1);

  connect(
    m_xSubDivisionEditor,
    QOverload<int>::of(&QSpinBox::valueChanged),
    this,
    &UvEditor::subDivisionChanged);
  connect(
    m_ySubDivisionEditor,
    QOverload<int>::of(&QSpinBox::valueChanged),
    this,
    &UvEditor::subDivisionChanged);

  auto* bottomLayout = new QHBoxLayout{};
  bottomLayout->setContentsMargins(
    LayoutConstants::NarrowHMargin, 0, LayoutConstants::NarrowHMargin, 0);
  bottomLayout->setSpacing(LayoutConstants::NarrowHMargin);
  bottomLayout->addWidget(m_resetUvButton);
  bottomLayout->addWidget(m_resetUvToWorldButton);
  bottomLayout->addWidget(m_flipUAxisButton);
  bottomLayout->addWidget(m_flipVAxisButton);
  bottomLayout->addWidget(m_rotateUvCCWButton);
  bottomLayout->addWidget(m_rotateUvCWButton);
  bottomLayout->addStretch();
  bottomLayout->addWidget(gridLabel);
  bottomLayout->addWidget(new QLabel{"X:"});
  bottomLayout->addWidget(m_xSubDivisionEditor);
  bottomLayout->addSpacing(
    LayoutConstants::MediumHMargin - LayoutConstants::NarrowHMargin);
  bottomLayout->addWidget(new QLabel{"Y:"});
  bottomLayout->addWidget(m_ySubDivisionEditor);

  auto* outerLayout = new QVBoxLayout{};
  outerLayout->setContentsMargins(0, 0, 0, 0);
  outerLayout->setSpacing(LayoutConstants::NarrowVMargin);
  outerLayout->addWidget(m_uvView, 1);
  outerLayout->addLayout(bottomLayout);
  setLayout(outerLayout);

  updateButtons();
}

void UvEditor::connectObservers()
{
  m_notifierConnection +=
    m_document.documentWasLoadedNotifier.connect([&]() { updateButtons(); });
  m_notifierConnection +=
    m_document.documentDidChangeNotifier.connect([&]() { updateButtons(); });
  m_notifierConnection +=
    m_document.selectionDidChangeNotifier.connect([&](const auto&) { updateButtons(); });
}

void UvEditor::resetUvClicked()
{
  auto& map = m_document.map();
  setBrushFaceAttributes(
    map, mdl::resetAll(map.gameInfo().gameConfig.faceAttribsConfig.defaultUvAttributes));
}

void UvEditor::resetUvToWorldClicked()
{
  auto& map = m_document.map();
  setBrushFaceAttributes(
    map,
    mdl::resetAllToParaxial(
      map.gameInfo().gameConfig.faceAttribsConfig.defaultUvAttributes));
}

void UvEditor::flipUvHClicked()
{
  setBrushFaceAttributes(m_document.map(), {.xScale = mdl::MultiplyValue{-1.0f}});
}

void UvEditor::flipUvVClicked()
{
  setBrushFaceAttributes(m_document.map(), {.yScale = mdl::MultiplyValue{-1.0f}});
}

void UvEditor::rotateUvCCWClicked()
{
  setBrushFaceAttributes(m_document.map(), {.rotation = mdl::AddValue{90.0f}});
}

void UvEditor::rotateUvCWClicked()
{
  setBrushFaceAttributes(m_document.map(), {.rotation = mdl::AddValue{-90.0f}});
}

void UvEditor::subDivisionChanged()
{
  const auto x = m_xSubDivisionEditor->value();
  const auto y = m_ySubDivisionEditor->value();
  m_uvView->setSubDivisions(vm::vec2i(x, y));
}

} // namespace tb::ui
