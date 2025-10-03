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

#include "DrawShapeTool.h"

#include "Logger.h"
#include "mdl/Brush.h" // IWYU pragma: keep
#include "mdl/BrushNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/Transaction.h"
#include "ui/DrawShapeToolExtension.h"
#include "ui/DrawShapeToolPage.h"

#include "kdl/ranges/to.h"
#include "kdl/result.h"

#include <ranges>

namespace tb::ui
{

DrawShapeTool::DrawShapeTool(mdl::Map& map)
  : CreateBrushesToolBase{true, map}
  , m_extensionManager{map}
{
}

void DrawShapeTool::update(const vm::bbox3d& bounds)
{
  m_extensionManager.createBrushes(bounds) | kdl::transform([&](auto brushes) {
    updateBrushes(
      brushes | std::views::transform([](auto brush) {
        return std::make_unique<mdl::BrushNode>(std::move(brush));
      })
      | kdl::ranges::to<std::vector>());
  }) | kdl::transform_error([&](auto e) {
    clearBrushes();
    m_map.logger().error() << "Could not update brushes: " << e;
  });
}

bool DrawShapeTool::cancel()
{
  if (m_map.selection().hasAny())
  {
    // Let the map view deselect before we switch the shapes
    return false;
  }

  return m_extensionManager.setCurrentExtensionIndex(0);
}

QWidget* DrawShapeTool::doCreatePage(QWidget* parent)
{
  auto* page = new DrawShapeToolPage{m_map, m_extensionManager, parent};
  m_notifierConnection += page->applyParametersNotifier.connect([&]() {
    if (const auto& selectionBounds = m_map.selectionBounds())
    {
      m_extensionManager.createBrushes(*selectionBounds)
        | kdl::transform([](auto brushes) {
            return brushes | std::views::transform([](auto brush) {
                     return std::make_unique<mdl::BrushNode>(std::move(brush));
                   })
                   | kdl::ranges::to<std::vector>();
          })
        | kdl::transform([&](auto brushNodes) {
            auto transaction = mdl::Transaction{m_map, "Update Brushes"};

            removeSelectedNodes(m_map);
            const auto addedNodes = addNodes(
              m_map,
              {
                {parentForNodes(m_map),
                 brushNodes | std::views::transform([](auto& node) {
                   return static_cast<mdl::Node*>(node.release());
                 }) | kdl::ranges::to<std::vector>()},
              });
            selectNodes(m_map, addedNodes);

            transaction.commit();
          })
        | kdl::transform_error(
          [&](auto e) { m_map.logger().error() << "Could not update brushes: " << e; });
    }
  });

  return page;
}

} // namespace tb::ui
