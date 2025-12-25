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

#pragma once

#include "ui/VertexToolBase.h"

#include <string>
#include <vector>

namespace tb
{
namespace gl
{
class Camera;
}

namespace mdl
{
class BrushNode;
class PickResult;
} // namespace mdl

namespace ui
{
class MapDocument;

class EdgeTool : public VertexToolBase<vm::segment3d>
{
public:
  explicit EdgeTool(MapDocument& document);

public:
  std::vector<mdl::BrushNode*> findIncidentBrushes(const vm::segment3d& handle) const;

private:
  using VertexToolBase::findIncidentBrushes;

public:
  void pick(
    const vm::ray3d& pickRay,
    const gl::Camera& camera,
    double handleRadius,
    mdl::PickResult& pickResult) const override;

public:
  mdl::EdgeHandleManager& handleManager() override;
  const mdl::EdgeHandleManager& handleManager() const override;

public:
  std::tuple<vm::vec3d, vm::vec3d> handlePositionAndHitPoint(
    const std::vector<mdl::Hit>& hits) const override;

  MoveResult move(const vm::vec3d& delta) override;

  std::string actionName() const override;

  void removeSelection();
};

} // namespace ui
} // namespace tb
