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

#include "DrawShapeToolExtensions.h"

#include <QWidget>

#include "Error.h" // IWYU pragma: keep
#include "Model/BrushBuilder.h"
#include "Model/WorldNode.h"
#include "View/MapDocument.h"

#include "kdl/result.h"

namespace TrenchBroom::View
{

const std::string& DrawShapeToolCuboidExtension::name() const
{
  static const auto name = std::string{"Cuboid"};
  return name;
}

QWidget* DrawShapeToolCuboidExtension::createToolPage(QWidget* parent)
{
  return new QWidget{parent};
}

Result<std::vector<Model::Brush>> DrawShapeToolCuboidExtension::createBrushes(
  const vm::bbox3& bounds, const vm::axis::type, const MapDocument& document) const
{
  const auto game = document.game();
  const auto builder = Model::BrushBuilder{
    document.world()->mapFormat(), document.worldBounds(), game->defaultFaceAttribs()};

  return builder.createCuboid(bounds, document.currentTextureName())
    .transform([](auto brush) { return std::vector{std::move(brush)}; });
}

} // namespace TrenchBroom::View
