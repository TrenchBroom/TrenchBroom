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

#include "ui/Tool.h"

#include "vm/bbox.h"
#include "vm/ray.h"

#include <string>

namespace tb
{
namespace mdl
{
class EntityNode;
class Map;
class PickResult;
} // namespace mdl

namespace ui
{

class CreateEntityTool : public Tool
{
private:
  mdl::Map& m_map;
  mdl::EntityNode* m_entity = nullptr;
  vm::bbox3d m_referenceBounds;

public:
  explicit CreateEntityTool(mdl::Map& map);

  bool createEntity(const std::string& classname);
  void removeEntity();
  void commitEntity();

  void updateEntityPosition2D(const vm::ray3d& pickRay);
  void updateEntityPosition3D(
    const vm::ray3d& pickRay, const mdl::PickResult& pickResult);
};

} // namespace ui
} // namespace tb
