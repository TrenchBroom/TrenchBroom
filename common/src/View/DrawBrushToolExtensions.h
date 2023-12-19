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

#pragma once

#include "View/DrawBrushToolExtension.h"

namespace TrenchBroom::View
{
class MapDocument;

class DrawBrushToolCuboidExtension : public DrawBrushToolExtension
{
public:
  const std::string& name() const override;
  QWidget* createToolPage(QWidget* parent) override;
  Result<Model::Brush> createBrush(
    const vm::bbox3& bounds,
    vm::axis::type axis,
    const MapDocument& document) const override;
};

} // namespace TrenchBroom::View
