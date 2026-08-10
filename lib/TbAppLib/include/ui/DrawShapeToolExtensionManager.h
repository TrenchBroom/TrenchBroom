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

#include "base/Notifier.h"
#include "ui/DrawShapeToolExtension.h"
#include "ui/DrawShapeToolParameters.h"

#include <memory>
#include <vector>

namespace tb::ui
{
class MapDocument;

class DrawShapeToolExtensionManager
{
private:
  MapDocument& m_document;
  DrawShapeToolParameters m_parameters;
  std::vector<std::unique_ptr<DrawShapeToolExtension>> m_extensions;
  size_t m_currentExtensionIndex = 0;

public:
  Notifier<size_t> currentExtensionDidChangeNotifier;
  Notifier<> applyParametersNotifier;

  explicit DrawShapeToolExtensionManager(MapDocument& document);

  MapDocument& document() const;
  DrawShapeToolParameters& parameters();

  const std::vector<DrawShapeToolExtension*> extensions() const;

  const DrawShapeToolExtension& currentExtension() const;
  bool setCurrentExtensionIndex(size_t currentExtensionIndex);

  Result<std::vector<mdl::Brush>> createBrushes(const vm::bbox3d& bounds) const;
};

} // namespace tb::ui
