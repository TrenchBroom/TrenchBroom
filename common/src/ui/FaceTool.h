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

#include "vm/polygon.h"

#include <string>
#include <vector>

namespace tb::ui
{
class FaceHandleManager;

class FaceTool : public VertexToolBase<vm::polygon3d>
{
public:
  explicit FaceTool(std::weak_ptr<MapDocument> document);

public:
  std::vector<mdl::BrushNode*> findIncidentBrushes(const vm::polygon3d& handle) const;

private:
  using VertexToolBase::findIncidentBrushes;

public:
  void pick(
    const vm::ray3d& pickRay,
    const render::Camera& camera,
    mdl::PickResult& pickResult) const override;

public:
  FaceHandleManager& handleManager() override;
  const FaceHandleManager& handleManager() const override;

public:
  std::tuple<vm::vec3d, vm::vec3d> handlePositionAndHitPoint(
    const std::vector<mdl::Hit>& hits) const override;

  MoveResult move(const vm::vec3d& delta) override;

  std::string actionName() const override;

  void removeSelection();
};

} // namespace tb::ui
