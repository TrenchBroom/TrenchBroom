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

#include "mdl/Brush.h" // IWYU pragma: keep
#include "mdl/BrushNode.h"
#include "ui/DrawShapeToolExtension.h"
#include "ui/DrawShapeToolExtensions.h"
#include "ui/DrawShapeToolPage.h"
#include "ui/MapDocument.h"
#include "ui/Transaction.h"

#include "kdl/memory_utils.h"
#include "kdl/range_to_vector.h"
#include "kdl/result.h"

#include <ranges>

namespace tb::ui
{

DrawShapeTool::DrawShapeTool(std::weak_ptr<MapDocument> document)
  : CreateBrushesToolBase{true, document}
  , m_extensionManager{createDrawShapeToolExtensions(document)}
{
}

void DrawShapeTool::update(const vm::bbox3d& bounds)
{
  m_extensionManager.currentExtension().createBrushes(bounds)
    | kdl::transform([&](auto brushes) {
        updateBrushes(
          brushes | std::views::transform([](auto brush) {
            return std::make_unique<mdl::BrushNode>(std::move(brush));
          })
          | kdl::to_vector);
      })
    | kdl::transform_error([&](auto e) {
        clearBrushes();
        auto document = kdl::mem_lock(m_document);
        document->error() << "Could not update brushes: " << e;
      });
}

bool DrawShapeTool::cancel()
{
  auto document = kdl::mem_lock(m_document);
  if (document->hasSelection())
  {
    // Let the map view deselect before we switch the shapes
    return false;
  }

  return m_extensionManager.setCurrentExtensionIndex(0);
}

QWidget* DrawShapeTool::doCreatePage(QWidget* parent)
{
  auto* page = new DrawShapeToolPage{m_document, m_extensionManager, parent};
  m_notifierConnection += page->settingsDidChangeNotifier.connect([&]() {
    auto document = kdl::mem_lock(m_document);
    if (document->hasSelectedNodes())
    {
      m_extensionManager.currentExtension().createBrushes(document->selectionBounds())
        | kdl::transform([](auto brushes) {
            return brushes | std::views::transform([](auto brush) {
                     return std::make_unique<mdl::BrushNode>(std::move(brush));
                   })
                   | kdl::to_vector;
          })
        | kdl::transform([&](auto brushNodes) {
            auto transaction = Transaction{document, "Update Brushes"};

            document->deleteObjects();
            const auto addedNodes = document->addNodes({
              {document->parentForNodes(),
               brushNodes | std::views::transform([](auto& node) {
                 return static_cast<mdl::Node*>(node.release());
               }) | kdl::to_vector},
            });
            document->selectNodes(addedNodes);

            transaction.commit();
          })
        | kdl::transform_error(
          [&](auto e) { document->error() << "Could not update brushes: " << e; });
    }
  });

  return page;
}

} // namespace tb::ui
