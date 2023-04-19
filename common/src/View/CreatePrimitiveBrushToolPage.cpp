/*
 Copyright (C) 2010-2023 Kristian Duske, Nathan "jitspoe" Wulf

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

#include "CreatePrimitiveBrushToolPage.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QToolButton>

#include "CreatePrimitiveBrushTool.h"
#include "View/MapDocument.h"
#include "View/QtUtils.h"
#include "View/ViewConstants.h"

#include <kdl/memory_utils.h>

#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

namespace TrenchBroom
{
namespace View
{
CreatePrimitiveBrushToolPage::CreatePrimitiveBrushToolPage(
  std::weak_ptr<MapDocument> document, CreatePrimitiveBrushTool& tool, QWidget* parent)
  : QWidget(parent)
  , m_document(document)
  , m_tool(tool)
{
  createGui();
  connectObservers();
  updateGui();
}

void CreatePrimitiveBrushToolPage::connectObservers()
{
  auto document = kdl::mem_lock(m_document);
  m_notifierConnection += document->selectionDidChangeNotifier.connect(
    this, &CreatePrimitiveBrushToolPage::selectionDidChange);
}

void CreatePrimitiveBrushToolPage::createGui()
{
  QLabel* typeLabel = new QLabel(tr("Type: "));
  QComboBox* typeComboBox = new QComboBox();
  // NOTE: Make sure these 1:1 match TrenchBroom::View::PrimitiveBrushData::ShapeType
  typeComboBox->addItem(tr("Cylinder"));
  typeComboBox->addItem(tr("Cone"));
  QLabel* numSidesLabel = new QLabel(tr("Number of Sides: "));
  QSpinBox* numSidesBox = new QSpinBox();
  numSidesBox->setRange(
    3, 256); // set before connecting callbacks so values won't get overwritten
  numSidesBox->setValue(m_tool.m_primitiveBrushData.numSides);
  QLabel* snapLabel = new QLabel(tr("Snap: "));
  QComboBox* snapComboBox = new QComboBox();
  // NOTE: Make sure these 1:1 match TrenchBroom::View::PrimitiveBrushData::SnapType
  snapComboBox->addItem(tr("Disabled"));
  snapComboBox->addItem(tr("Integer"));
  snapComboBox->addItem(tr("Grid"));
  snapComboBox->setCurrentIndex(1);
  QCheckBox* uniformAspectCheckbox = new QCheckBox(tr("Uniform Aspect"));
  uniformAspectCheckbox->setChecked(m_tool.m_primitiveBrushData.uniformAspect);
  QButtonGroup* radiusModeButtonGroup = new QButtonGroup();
  QToolButton* radiusModeEdgeButton =
    createBitmapToggleButton("RadiusModeEdge.svg", tr("Radius is to edge"));
  radiusModeEdgeButton->setIconSize(QSize(24, 24));
  QToolButton* radiusModeVertexButton =
    createBitmapToggleButton("RadiusModeVertex.svg", tr("Radius is to vertex"));
  radiusModeVertexButton->setIconSize(QSize(24, 24));
  radiusModeEdgeButton->setObjectName(
    "toolButton_borderOnCheck"); // Style sheet makes this have a border when checked,
                                 // otherwise nothing.
  radiusModeEdgeButton->setChecked(true);
  radiusModeVertexButton->setObjectName("toolButton_borderOnCheck");
  radiusModeButtonGroup->addButton(radiusModeEdgeButton);
  radiusModeButtonGroup->addButton(radiusModeVertexButton);

  connect(
    numSidesBox,
    QOverload<int>::of(&QSpinBox::valueChanged),
    this,
    [=](int numSidesValue) {
      this->m_tool.m_primitiveBrushData.numSides = numSidesValue;
      this->m_tool.update();
    });
  connect(
    snapComboBox,
    QOverload<int>::of(&QComboBox::currentIndexChanged),
    this,
    [=](int index) {
      this->m_tool.m_primitiveBrushData.snapType = PrimitiveBrushData::SnapType(index);
      this->m_tool.update();
    });
  connect(
    typeComboBox,
    QOverload<int>::of(&QComboBox::currentIndexChanged),
    this,
    [=](int index) {
      this->m_tool.m_primitiveBrushData.shapeType = PrimitiveBrushData::ShapeType(index);
      this->m_tool.update();
    });
  connect(uniformAspectCheckbox, &QCheckBox::toggled, this, [=](bool checked) {
    this->m_tool.m_primitiveBrushData.uniformAspect = checked;
    this->m_tool.update();
  });
  connect(radiusModeEdgeButton, &QToolButton::clicked, this, [=]() {
    this->m_tool.m_primitiveBrushData.radiusMode = PrimitiveBrushData::RADIUS_MODE_EDGE;
    this->m_tool.update();
  });
  connect(radiusModeVertexButton, &QToolButton::clicked, this, [=]() {
    this->m_tool.m_primitiveBrushData.radiusMode = PrimitiveBrushData::RADIUS_MODE_VERTEX;
    this->m_tool.update();
  });

  auto* layout = new QHBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(LayoutConstants::MediumHMargin);

  layout->addWidget(typeLabel, 0, Qt::AlignVCenter);
  layout->addWidget(typeComboBox, 0, Qt::AlignVCenter);
  layout->addWidget(numSidesLabel, 0, Qt::AlignVCenter);
  layout->addWidget(numSidesBox, 0, Qt::AlignVCenter);
  layout->addWidget(uniformAspectCheckbox, 0, Qt::AlignVCenter);
  layout->addWidget(snapLabel, 0, Qt::AlignVCenter);
  layout->addWidget(snapComboBox, 0, Qt::AlignVCenter);
  layout->addWidget(radiusModeEdgeButton, 0, Qt::AlignVCenter);
  layout->addWidget(radiusModeVertexButton, 0, Qt::AlignVCenter);
  layout->addStretch(1);

  setLayout(layout);
}

void CreatePrimitiveBrushToolPage::updateGui()
{
  // NOTE: This gets called after creating a brush, so we can't consider this to only be
  // called when selecting brushes manually.
  auto document = kdl::mem_lock(m_document);
}

void CreatePrimitiveBrushToolPage::selectionDidChange(const Selection&)
{
  updateGui();
}

} // namespace View
} // namespace TrenchBroom
