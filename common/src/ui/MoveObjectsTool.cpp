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

#include "MoveObjectsTool.h"

#include "mdl/Grid.h"
#include "mdl/Map.h"
#include "mdl/TransactionScope.h"
#include "ui/InputState.h"

#include "vm/bbox.h"

#include <cassert>

namespace tb::ui
{

MoveObjectsTool::MoveObjectsTool(mdl::Map& map)
  : Tool{true}
  , m_map{map}
{
}

const mdl::Grid& MoveObjectsTool::grid() const
{
  return m_map.grid();
}

bool MoveObjectsTool::startMove(const InputState& inputState)
{
  if (!m_map.selection().brushFaces.empty())
  {
    return false;
  }

  m_map.startTransaction(
    duplicateObjects(inputState) ? "Duplicate Objects" : "Move Objects",
    mdl::TransactionScope::LongRunning);
  m_duplicateObjects = duplicateObjects(inputState);
  return true;
}

MoveObjectsTool::MoveResult MoveObjectsTool::move(
  const InputState&, const vm::vec3d& delta)
{
  const auto& worldBounds = m_map.worldBounds();
  const auto bounds = m_map.selectionBounds();
  if (!bounds)
  {
    return MoveResult::Cancel;
  }

  if (!worldBounds.contains(bounds->translate(delta)))
  {
    return MoveResult::Deny;
  }

  if (m_duplicateObjects)
  {
    m_duplicateObjects = false;
    m_map.duplicateSelectedNodes();
  }

  return m_map.translateSelection(delta) ? MoveResult::Continue : MoveResult::Deny;
}

void MoveObjectsTool::endMove(const InputState&)
{
  m_map.commitTransaction();
}

void MoveObjectsTool::cancelMove()
{
  m_map.cancelTransaction();
}

bool MoveObjectsTool::duplicateObjects(const InputState& inputState) const
{
  return inputState.modifierKeysDown(ModifierKeys::CtrlCmd);
}

} // namespace tb::ui
