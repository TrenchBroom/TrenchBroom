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

#include "ui/UVEditor.h"

#include <QGridLayout>
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
#include "ui/Drawer.h"
#include "ui/MapDocument.h"
#include "ui/QStyleUtils.h"
#include "ui/UVView.h"
#include "ui/ViewConstants.h"

namespace tb::ui
{

UVEditor::UVEditor(
  MapDocument& document, gl::ContextManager& contextManager, QWidget* parent)
  : QWidget{parent}
  , m_document{document}
{
  createGui(contextManager);
  connectObservers();
}

bool UVEditor::cancelMouseDrag()
{
  return m_uvView->cancelDrag();
}

void UVEditor::updateButtons()
{
  const bool enabled = !m_document.map().selection().allBrushFaces().empty();

  m_resetUVButton->setEnabled(enabled);
  m_resetUVToWorldButton->setEnabled(enabled);
  m_flipUAxisButton->setEnabled(enabled);
  m_flipVAxisButton->setEnabled(enabled);
  m_rotateUVCCWButton->setEnabled(enabled);
  m_rotateUVCWButton->setEnabled(enabled);
  m_alignButton->setEnabled(enabled);
  m_justifyUpButton->setEnabled(enabled);
  m_justifyDownButton->setEnabled(enabled);
  m_justifyLeftButton->setEnabled(enabled);
  m_justifyRightButton->setEnabled(enabled);
  m_fitHButton->setEnabled(enabled);
  m_fitVButton->setEnabled(enabled);
  m_autoFitButton->setEnabled(enabled);
}

void UVEditor::createGui(gl::ContextManager& contextManager)
{
  m_uvView = new UVView{m_document, contextManager};
  m_drawer = new Drawer{createFitter(), m_uvView};

  connect(m_resetUVButton, &QAbstractButton::clicked, this, &UVEditor::resetUVClicked);
  connect(
    m_resetUVToWorldButton,
    &QAbstractButton::clicked,
    this,
    &UVEditor::resetUVToWorldClicked);
  connect(m_flipUAxisButton, &QAbstractButton::clicked, this, &UVEditor::flipUVHClicked);
  connect(m_flipVAxisButton, &QAbstractButton::clicked, this, &UVEditor::flipUVVClicked);
  connect(
    m_rotateUVCCWButton, &QAbstractButton::clicked, this, &UVEditor::rotateUVCCWClicked);
  connect(
    m_rotateUVCWButton, &QAbstractButton::clicked, this, &UVEditor::rotateUVCWClicked);
  connect(m_alignButton, &QAbstractButton::clicked, this, &UVEditor::alignClicked);
  connect(m_justifyUpButton, &QAbstractButton::clicked, [&]() {
    justifyClicked(mdl::UvAxis::v, mdl::UvDirection::forward);
  });
  connect(m_justifyDownButton, &QAbstractButton::clicked, [&]() {
    justifyClicked(mdl::UvAxis::v, mdl::UvDirection::backward);
  });
  connect(m_justifyLeftButton, &QAbstractButton::clicked, [&]() {
    justifyClicked(mdl::UvAxis::u, mdl::UvDirection::backward);
  });
  connect(m_justifyRightButton, &QAbstractButton::clicked, [&]() {
    justifyClicked(mdl::UvAxis::u, mdl::UvDirection::forward);
  });
  connect(m_fitHButton, &QAbstractButton::clicked, [&]() { fitClicked(mdl::UvAxis::u); });
  connect(m_fitVButton, &QAbstractButton::clicked, [&]() { fitClicked(mdl::UvAxis::v); });

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
    &UVEditor::subDivisionChanged);
  connect(
    m_ySubDivisionEditor,
    QOverload<int>::of(&QSpinBox::valueChanged),
    this,
    &UVEditor::subDivisionChanged);

  auto* bottomLayout = new QHBoxLayout{};
  bottomLayout->setContentsMargins(
    LayoutConstants::NarrowHMargin, 0, LayoutConstants::NarrowHMargin, 0);
  bottomLayout->setSpacing(LayoutConstants::NarrowHMargin);
  bottomLayout->addWidget(m_resetUVButton);
  bottomLayout->addWidget(m_resetUVToWorldButton);
  bottomLayout->addWidget(m_flipUAxisButton);
  bottomLayout->addWidget(m_flipVAxisButton);
  bottomLayout->addWidget(m_rotateUVCCWButton);
  bottomLayout->addWidget(m_rotateUVCWButton);
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

QWidget* UVEditor::createFitter()
{

  m_resetUVButton = createBitmapButton("ResetUV.svg", tr("Reset UV alignment"), this);
  m_resetUVToWorldButton = createBitmapButton(
    "ResetUVToWorld.svg", tr("Reset UV alignment to world aligned"), this);
  m_flipUAxisButton = createBitmapButton("FlipUAxis.svg", tr("Flip U axis"), this);
  m_flipVAxisButton = createBitmapButton("FlipVAxis.svg", tr("Flip V axis"), this);
  m_rotateUVCCWButton =
    createBitmapButton("RotateUVCCW.svg", tr("Rotate UV 90° counter-clockwise"), this);
  m_rotateUVCWButton =
    createBitmapButton("RotateUVCW.svg", tr("Rotate UV 90° clockwise"), this);
  m_alignButton = createBitmapButton(
    "AlignTexture.svg",
    tr("Align texture to face edges. Click again to cycle through edges. Hold shift to "
       "cycle backwards."),
    this);
  m_justifyUpButton = createBitmapButton(
    "JustifyTextureUp.svg",
    tr("Justify texture upwards. Click again to cycle through options. Hold shift to "
       "cycle backwards."),
    this);
  m_justifyDownButton = createBitmapButton(
    "JustifyTextureDown.svg",
    tr("Justify texture downwards. Click again to cycle through options. Hold shift to "
       "cycle backwards."),
    this);
  m_justifyLeftButton = createBitmapButton(
    "JustifyTextureLeft.svg",
    tr("Justify texture leftwards. Click again to cycle through options. Hold shift to "
       "cycle backwards."),
    this);
  m_justifyRightButton = createBitmapButton(
    "JustifyTextureRight.svg",
    tr("Justify texture rightwards. Click again to cycle through options. Hold shift to "
       "cycle backwards."),
    this);
  m_fitHButton = createBitmapButton(
    "FitTextureHorizontally.svg",
    tr("Fit texture horizontally. Click again to cycle through options. Hold shift to "
       "cycle backwards."),
    this);
  m_fitVButton = createBitmapButton(
    "FitTextureVertically.svg",
    tr("Fit texture vertically. Click again to cycle through options. Hold shift to "
       "cycle backwards."),
    this);
  m_autoFitButton = createBitmapButton(
    "AutoFitTexture.svg",
    tr("Auto fit texture. Click again to cycle through options. Hold shift to cycle "
       "backwards."),
    this);


  auto* innerLayout = new QGridLayout{};
  innerLayout->addWidget(m_alignButton, 0, 0);
  innerLayout->addWidget(m_justifyUpButton, 0, 1);
  innerLayout->addWidget(m_justifyLeftButton, 1, 0);
  innerLayout->addWidget(m_autoFitButton, 1, 1);
  innerLayout->addWidget(m_justifyRightButton, 1, 2);
  innerLayout->addWidget(m_fitHButton, 2, 0);
  innerLayout->addWidget(m_justifyDownButton, 2, 1);
  innerLayout->addWidget(m_fitVButton, 2, 2);

  innerLayout->setContentsMargins(QMargins{
    LayoutConstants::WideHMargin,
    LayoutConstants::WideVMargin,
    LayoutConstants::WideHMargin,
    LayoutConstants::WideVMargin});
  innerLayout->setSpacing(LayoutConstants::MediumHMargin);

  auto* outerLayout = new QVBoxLayout{};
  outerLayout->addStretch(0);
  outerLayout->addLayout(innerLayout);
  outerLayout->addStretch(0);

  auto* container = new QWidget{};

  auto col = QColor{Qt::black};
  col.setAlpha(128);

  auto pal = QPalette{};
  pal.setColor(QPalette::Window, col);

  container->setAutoFillBackground(true);
  container->setPalette(pal);
  container->setLayout(outerLayout);
  container->adjustSize();

  return container;
}

void UVEditor::connectObservers()
{
  m_notifierConnection +=
    m_document.documentWasLoadedNotifier.connect([&]() { updateButtons(); });
  m_notifierConnection +=
    m_document.documentDidChangeNotifier.connect([&]() { updateButtons(); });
  m_notifierConnection +=
    m_document.selectionDidChangeNotifier.connect([&](const auto&) { updateButtons(); });
}

void UVEditor::resetUVClicked()
{
  auto& map = m_document.map();
  setBrushFaceAttributes(
    map, mdl::resetAll(map.gameInfo().gameConfig.faceAttribsConfig.defaults));
}

void UVEditor::resetUVToWorldClicked()
{
  auto& map = m_document.map();
  setBrushFaceAttributes(
    map, mdl::resetAllToParaxial(map.gameInfo().gameConfig.faceAttribsConfig.defaults));
}

void UVEditor::flipUVHClicked()
{
  setBrushFaceAttributes(m_document.map(), {.xScale = mdl::MultiplyValue{-1.0f}});
}

void UVEditor::flipUVVClicked()
{
  setBrushFaceAttributes(m_document.map(), {.yScale = mdl::MultiplyValue{-1.0f}});
}

void UVEditor::rotateUVCCWClicked()
{
  setBrushFaceAttributes(m_document.map(), {.rotation = mdl::AddValue{90.0f}});
}

void UVEditor::rotateUVCWClicked()
{
  setBrushFaceAttributes(m_document.map(), {.rotation = mdl::AddValue{-90.0f}});
}

void UVEditor::alignClicked()
{
  auto& map = m_document.map();
  auto& selection = map.selection();
  contract_assert(selection.brushFaces.size() == 1);

  const auto& brushFace = selection.brushFaces.front().face();
  const auto policy = qApp->keyboardModifiers().testFlag(Qt::ShiftModifier)
                        ? mdl::UvPolicy::prev
                        : mdl::UvPolicy::next;

  setBrushFaceAttributes(m_document.map(), mdl::align(brushFace, policy));
}

void UVEditor::justifyClicked(const mdl::UvAxis axis, const mdl::UvDirection direction)
{
  auto& map = m_document.map();
  auto& selection = map.selection();
  contract_assert(selection.brushFaces.size() == 1);

  const auto& brushFace = selection.brushFaces.front().face();
  const auto policy = qApp->keyboardModifiers().testFlag(Qt::ShiftModifier)
                        ? mdl::UvPolicy::prev
                        : mdl::UvPolicy::next;

  setBrushFaceAttributes(
    m_document.map(), mdl::justify(brushFace, axis, direction, policy));
}

void UVEditor::fitClicked(const mdl::UvAxis axis)
{
  auto& map = m_document.map();
  auto& selection = map.selection();
  contract_assert(selection.brushFaces.size() == 1);

  const auto& brushFace = selection.brushFaces.front().face();
  const auto policy = qApp->keyboardModifiers().testFlag(Qt::ShiftModifier)
                        ? mdl::UvPolicy::prev
                        : mdl::UvPolicy::next;

  setBrushFaceAttributes(m_document.map(), mdl::fit(brushFace, axis, policy));
}

void UVEditor::subDivisionChanged()
{
  const auto x = m_xSubDivisionEditor->value();
  const auto y = m_ySubDivisionEditor->value();
  m_uvView->setSubDivisions(vm::vec2i(x, y));
}

} // namespace tb::ui
