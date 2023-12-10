/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "DrawBrushTool.h"

#include "Error.h" // IWYU pragma: keep
#include "FloatType.h"
#include "Model/Brush.h" // IWYU pragma: keep
#include "Model/BrushNode.h"
#include "View/DrawBrushToolExtension.h"
#include "View/DrawBrushToolExtensions.h"
#include "View/DrawBrushToolPage.h"
#include "View/MapDocument.h"

#include <kdl/memory_utils.h>
#include <kdl/result.h>

namespace TrenchBroom::View
{

namespace
{
auto createExtensions()
{
  auto result = std::vector<std::unique_ptr<DrawBrushToolExtension>>{};
  result.push_back(std::make_unique<DrawBrushToolCuboidExtension>());
  return result;
}
} // namespace

DrawBrushTool::DrawBrushTool(std::weak_ptr<MapDocument> document)
  : CreateBrushToolBase{true, std::move(document)}
  , m_extensionManager{createExtensions()}
{
}

void DrawBrushTool::update(const vm::bbox3& bounds, const vm::axis::type axis)
{
  auto document = kdl::mem_lock(m_document);
  m_extensionManager.currentExtension()
    .createBrush(bounds, axis, *document)
    .transform(
      [&](auto b) { updateBrush(std::make_unique<Model::BrushNode>(std::move(b))); })
    .transform_error([&](auto e) {
      updateBrush(nullptr);
      document->error() << "Could not update brush: " << e;
    });
}

bool DrawBrushTool::cancel()
{
  auto document = kdl::mem_lock(m_document);
  if (document->hasSelection())
  {
    // Let the map view deselect before we switch the shapes
    return false;
  }

  return m_extensionManager.setCurrentExtensionIndex(0);
}

QWidget* DrawBrushTool::doCreatePage(QWidget* parent)
{
  return new DrawBrushToolPage{m_document, m_extensionManager, parent};
}

} // namespace TrenchBroom::View
