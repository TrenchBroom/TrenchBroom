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

#include "AssembleBrushTool.h"

#include "Error.h" // IWYU pragma: keep
#include "mdl/BrushBuilder.h"
#include "mdl/BrushNode.h"
#include "mdl/Game.h"
#include "mdl/Map.h"
#include "mdl/Polyhedron.h"
#include "mdl/WorldNode.h"
#include "ui/MapDocument.h"

#include "kdl/memory_utils.h"
#include "kdl/result.h"
#include "kdl/vector_utils.h"

namespace tb::ui
{

AssembleBrushTool::AssembleBrushTool(std::weak_ptr<MapDocument> document)
  : CreateBrushesToolBase{false, std::move(document)}
  , m_polyhedron{std::make_unique<mdl::Polyhedron3>()}
{
}

const mdl::Polyhedron3& AssembleBrushTool::polyhedron() const
{
  return *m_polyhedron;
}

void AssembleBrushTool::update(const mdl::Polyhedron3& polyhedron)
{
  *m_polyhedron = polyhedron;
  if (m_polyhedron->closed())
  {
    auto& map = kdl::mem_lock(m_document)->map();
    const auto game = map.game();
    const auto builder = mdl::BrushBuilder{
      map.world()->mapFormat(),
      map.worldBounds(),
      game->config().faceAttribsConfig.defaults};

    builder.createBrush(*m_polyhedron, map.currentMaterialName())
      | kdl::transform([&](auto b) {
          updateBrushes(kdl::vec_from(std::make_unique<mdl::BrushNode>(std::move(b))));
        })
      | kdl::transform_error([&](auto e) {
          clearBrushes();
          map.logger().error() << "Could not update brush: " << e.msg;
        });
  }
  else
  {
    clearBrushes();
  }
}

bool AssembleBrushTool::doActivate()
{
  update(mdl::Polyhedron3{});
  return true;
}

bool AssembleBrushTool::doDeactivate()
{
  update(mdl::Polyhedron3{});
  return true;
}

void AssembleBrushTool::doBrushesWereCreated()
{
  update(mdl::Polyhedron3{});
}

} // namespace tb::ui
