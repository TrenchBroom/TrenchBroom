/*
 Copyright (C) 2023 Kristian Duske

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

#include "DrawShapeToolExtension.h"

#include <QHBoxLayout>
#include <QPushButton>

#include "Ensure.h"
#include "mdl/Map.h"
#include "ui/DrawShapeToolExtensions.h"
#include "ui/ViewConstants.h"

#include "kdl/ranges/to.h"
#include "kdl/vector_utils.h"

#include <ranges>

namespace tb::ui
{

DrawShapeToolExtensionPage::DrawShapeToolExtensionPage(QWidget* parent)
  : QWidget{parent}
{
  auto* layout = new QHBoxLayout{};
  layout->setContentsMargins(QMargins{});
  layout->setSpacing(LayoutConstants::MediumHMargin);
  layout->addStretch(1);
  setLayout(layout);
}

void DrawShapeToolExtensionPage::addWidget(QWidget* widget)
{
  auto* boxLayout = qobject_cast<QHBoxLayout*>(layout());
  boxLayout->insertWidget(boxLayout->count() - 1, widget, 0, Qt::AlignVCenter);
}

void DrawShapeToolExtensionPage::addApplyButton(mdl::Map& map)
{
  auto* applyButton = new QPushButton{tr("Apply")};
  applyButton->setEnabled(false);
  connect(applyButton, &QPushButton::clicked, this, [&]() { applyParametersNotifier(); });

  addWidget(applyButton);

  m_notifierConnection +=
    map.selectionDidChangeNotifier.connect([&map, applyButton](const auto&) {
      applyButton->setEnabled(map.selection().hasNodes());
    });
}

vm::axis::type ShapeParameters::axis() const
{
  return m_axis;
}

void ShapeParameters::setAxis(const vm::axis::type axis)
{
  if (axis != m_axis)
  {
    m_axis = axis;
    parametersDidChangeNotifier();
  }
}

const mdl::CircleShape& ShapeParameters::circleShape() const
{
  return m_circleShape;
}

void ShapeParameters::setCircleShape(mdl::CircleShape circleShape)
{
  if (circleShape != m_circleShape)
  {
    m_circleShape = std::move(circleShape);
    parametersDidChangeNotifier();
  }
}

bool ShapeParameters::hollow() const
{
  return m_hollow;
}

void ShapeParameters::setHollow(const bool hollow)
{
  if (hollow != m_hollow)
  {
    m_hollow = hollow;
    parametersDidChangeNotifier();
  }
}

double ShapeParameters::thickness() const
{
  return m_thickness;
}

void ShapeParameters::setThickness(const double thickness)
{
  if (thickness != m_thickness)
  {
    m_thickness = thickness;
    parametersDidChangeNotifier();
  }
}

size_t ShapeParameters::numRings() const
{
  return m_numRings;
}

void ShapeParameters::setNumRings(const size_t numRings)
{
  if (numRings != m_numRings)
  {
    m_numRings = numRings;
    parametersDidChangeNotifier();
  }
}

size_t ShapeParameters::accuracy() const
{
  return m_accuracy;
}

void ShapeParameters::setAccuracy(const size_t accuracy)
{
  if (accuracy != m_accuracy)
  {
    m_accuracy = accuracy;
    parametersDidChangeNotifier();
  }
}

DrawShapeToolExtension::DrawShapeToolExtension(mdl::Map& map)
  : m_map{map}
{
}

DrawShapeToolExtension::~DrawShapeToolExtension() = default;

DrawShapeToolExtensionManager::DrawShapeToolExtensionManager(mdl::Map& map)
  : m_extensions{createDrawShapeToolExtensions(map)}
{
  ensure(!m_extensions.empty(), "extensions must not be empty");
}

const std::vector<DrawShapeToolExtension*> DrawShapeToolExtensionManager::extensions()
  const
{
  return m_extensions
         | std::views::transform([](const auto& extension) { return extension.get(); })
         | kdl::ranges::to<std::vector>();
}

const DrawShapeToolExtension& DrawShapeToolExtensionManager::currentExtension() const
{
  return *m_extensions[m_currentExtensionIndex];
}

bool DrawShapeToolExtensionManager::setCurrentExtensionIndex(size_t currentExtensionIndex)
{
  if (currentExtensionIndex != m_currentExtensionIndex)
  {
    m_currentExtensionIndex = currentExtensionIndex;
    currentExtensionDidChangeNotifier(m_currentExtensionIndex);
    return true;
  }

  return false;
}

std::vector<DrawShapeToolExtensionPage*> DrawShapeToolExtensionManager::createToolPages(
  QWidget* parent)
{
  return m_extensions | std::views::transform([&](const auto& extension) {
           return extension->createToolPage(m_parameters, parent);
         })
         | kdl::ranges::to<std::vector>();
}

Result<std::vector<mdl::Brush>> DrawShapeToolExtensionManager::createBrushes(
  const vm::bbox3d& bounds) const
{
  return currentExtension().createBrushes(bounds, m_parameters);
}

} // namespace tb::ui
