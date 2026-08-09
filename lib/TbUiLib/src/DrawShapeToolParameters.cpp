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

#include "ui/DrawShapeToolParameters.h"

namespace tb::ui
{

vm::axis::type DrawShapeToolParameters::axis() const
{
  return m_axis;
}

void DrawShapeToolParameters::setAxis(const vm::axis::type axis)
{
  if (axis != m_axis)
  {
    m_axis = axis;
    parametersDidChangeNotifier();
  }
}

const mdl::CircleShape& DrawShapeToolParameters::circleShape() const
{
  return m_circleShape;
}

void DrawShapeToolParameters::setCircleShape(mdl::CircleShape circleShape)
{
  if (circleShape != m_circleShape)
  {
    m_circleShape = std::move(circleShape);
    parametersDidChangeNotifier();
  }
}

bool DrawShapeToolParameters::hollow() const
{
  return m_hollow;
}

void DrawShapeToolParameters::setHollow(const bool hollow)
{
  if (hollow != m_hollow)
  {
    m_hollow = hollow;
    parametersDidChangeNotifier();
  }
}

double DrawShapeToolParameters::thickness() const
{
  return m_thickness;
}

void DrawShapeToolParameters::setThickness(const double thickness)
{
  if (thickness != m_thickness)
  {
    m_thickness = thickness;
    parametersDidChangeNotifier();
  }
}

size_t DrawShapeToolParameters::numRings() const
{
  return m_numRings;
}

void DrawShapeToolParameters::setNumRings(const size_t numRings)
{
  if (numRings != m_numRings)
  {
    m_numRings = numRings;
    parametersDidChangeNotifier();
  }
}

size_t DrawShapeToolParameters::accuracy() const
{
  return m_accuracy;
}

void DrawShapeToolParameters::setAccuracy(const size_t accuracy)
{
  if (accuracy != m_accuracy)
  {
    m_accuracy = accuracy;
    parametersDidChangeNotifier();
  }
}

double DrawShapeToolParameters::stepHeight() const
{
  return m_stepHeight;
}

void DrawShapeToolParameters::setStepHeight(const double stepHeight)
{
  if (stepHeight != m_stepHeight)
  {
    m_stepHeight = stepHeight;
    parametersDidChangeNotifier();
  }
}

DrawShapeToolParameters::StairDirection DrawShapeToolParameters::stairDirection() const
{
  return m_stairDirection;
}

void DrawShapeToolParameters::setStairDirection(const StairDirection stairDirection)
{
  if (stairDirection != m_stairDirection)
  {
    m_stairDirection = stairDirection;
    parametersDidChangeNotifier();
  }
}

} // namespace tb::ui
