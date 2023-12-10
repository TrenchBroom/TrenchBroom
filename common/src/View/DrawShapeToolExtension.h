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

#include "Model/Brush.h"
#include "Notifier.h"
#include "Result.h"

#include "vm/util.h"

#include <memory>
#include <string>
#include <vector>

class QWidget;

namespace TrenchBroom::View
{
class MapDocument;

class DrawShapeToolExtension
{
public:
  virtual ~DrawShapeToolExtension();
  virtual const std::string& name() const = 0;
  virtual QWidget* createToolPage(QWidget* parent = nullptr) = 0;
  virtual Result<std::vector<Model::Brush>> createBrushes(
    const vm::bbox3& bounds, vm::axis::type axis, const MapDocument& document) const = 0;
};

class DrawShapeToolExtensionManager
{
public:
  Notifier<size_t> currentExtensionDidChangeNotifier;

  explicit DrawShapeToolExtensionManager(
    std::vector<std::unique_ptr<DrawShapeToolExtension>> extensions);

  const std::vector<DrawShapeToolExtension*> extensions() const;

  DrawShapeToolExtension& currentExtension();
  bool setCurrentExtensionIndex(size_t currentExtensionIndex);

private:
  std::vector<std::unique_ptr<DrawShapeToolExtension>> m_extensions;
  size_t m_currentExtensionIndex = 0;
};

} // namespace TrenchBroom::View
