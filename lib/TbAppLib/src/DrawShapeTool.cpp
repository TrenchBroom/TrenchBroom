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

#include "base/Logger.h"
#include "base/PreferenceManager.h"
#include "mdl/Brush.h" // IWYU pragma: keep
#include "mdl/BrushNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Transaction.h"
#include "prefs/Preferences.h"
#include "ui/DrawShapeToolExtensionManager.h"
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
      update(*bounds);
      if (hasBrushes())
      {
        auto transaction = mdl::Transaction{map, "Apply shape parameters"};
        mdl::removeSelectedNodes(map);
        createBrushes(groupNameForCreatedBrushes());
        transaction.commit();
      }
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
    m_document.logger().error() << "Could not update brushes: " << e.msg;
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

DrawShapeToolExtensionManager& DrawShapeTool::extensionManager()
{
  return m_extensionManager;
}

std::optional<std::string> DrawShapeTool::groupNameForCreatedBrushes() const
{
  return pref(Preferences::GroupBrushesCreatedByShapeTool)
           ? std::optional{m_extensionManager.currentExtension().name()}
           : std::nullopt;
}

void DrawShapeTool::doBrushesWereCreated()
{
  m_extensionManager.brushesWereCreated();
}

} // namespace tb::ui
