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

#include "ui/DrawShapeTool.h"

#include "Logger.h"
#include "mdl/Brush.h" // IWYU pragma: keep
#include "mdl/BrushNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/Transaction.h"
#include "ui/DrawShapeToolExtension.h"
#include "ui/DrawShapeToolPage.h"
#include "ui/MapDocument.h"

#include "kd/ranges/to.h"
#include "kd/result.h"

#include <ranges>

namespace tb::ui
{

DrawShapeTool::DrawShapeTool(MapDocument& document)
  : CreateBrushesToolBase{true, document}
  , m_extensionManager{document}
{
  m_notifierConnection += m_extensionManager.applyParametersNotifier.connect([&]() {
    auto& map = m_document.map();
    if (const auto bounds = map.selectionBounds())
    {
      auto transaction = mdl::Transaction{map, "Apply shape parameters"};
      update(*bounds);
      mdl::removeSelectedNodes(map);
      createBrushes();
      transaction.commit();
    }
  });
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
    m_document.logger().error() << "Could not update brushes: " << e;
  });
}

bool DrawShapeTool::cancel()
{
  if (m_document.map().selection().hasAny())
  {
    // Let the map view deselect before we switch the shapes
    return false;
  }

  return m_extensionManager.setCurrentExtensionIndex(0);
}

QWidget* DrawShapeTool::doCreatePage(QWidget* parent)
{
  auto* page = new DrawShapeToolPage{m_extensionManager, parent};
  m_notifierConnection += page->applyParametersNotifier.connect([this]() {
    auto& map = m_document.map();
    if (const auto& selectionBounds = map.selectionBounds())
    {
      m_extensionManager.createBrushes(*selectionBounds)
        | kdl::transform([](auto brushes) {
            return brushes | std::views::transform([](auto brush) {
                     return std::make_unique<mdl::BrushNode>(std::move(brush));
                   })
                   | kdl::ranges::to<std::vector>();
          })
        | kdl::transform([&](auto brushNodes) {
            auto transaction = mdl::Transaction{map, "Update Brushes"};

            removeSelectedNodes(map);
            const auto addedNodes = addNodes(
              map,
              {
                {parentForNodes(map), brushNodes | std::views::transform([](auto& node) {
                                        return static_cast<mdl::Node*>(node.release());
                                      }) | kdl::ranges::to<std::vector>()},
              });
            selectNodes(map, addedNodes);

            transaction.commit();
          })
        | kdl::transform_error([&](auto e) {
            m_document.logger().error() << "Could not update brushes: " << e;
          });
    }
  });

  return page;
}

} // namespace tb::ui
