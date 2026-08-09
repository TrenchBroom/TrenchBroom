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

#include "base/Result.h"
#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"

#include <filesystem>
#include <string>
#include <vector>

namespace tb::ui
{
class DrawShapeToolParameters;
class MapDocument;

class DrawShapeToolExtension
{
protected:
  MapDocument& m_document;

  explicit DrawShapeToolExtension(MapDocument& document);

public:
  virtual ~DrawShapeToolExtension();
  virtual const std::string& name() const = 0;
  virtual const std::filesystem::path& iconPath() const = 0;
  virtual Result<std::vector<mdl::Brush>> createBrushes(
    const vm::bbox3d& bounds, const DrawShapeToolParameters& parameters) const = 0;
};

} // namespace tb::ui
