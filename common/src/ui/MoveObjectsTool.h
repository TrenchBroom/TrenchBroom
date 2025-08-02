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

#include "vm/vec.h"

namespace tb::mdl
{
class Grid;
class Map;
} // namespace tb::mdl

namespace tb::ui
{
class InputState;

class MoveObjectsTool : public Tool
{
public:
  enum class MoveResult
  {
    Continue,
    Deny,
    Cancel,
  };

private:
  mdl::Map& m_map;
  bool m_duplicateObjects = false;

public:
  explicit MoveObjectsTool(mdl::Map& map);

public:
  const mdl::Grid& grid() const;

  bool startMove(const InputState& inputState);
  MoveResult move(const InputState& inputState, const vm::vec3d& delta);
  void endMove(const InputState& inputState);
  void cancelMove();

private:
  bool duplicateObjects(const InputState& inputState) const;
};

} // namespace tb::ui
