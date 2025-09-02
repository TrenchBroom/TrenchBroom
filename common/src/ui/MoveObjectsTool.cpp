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
#include "mdl/TransactionScope.h"
#include "ui/InputState.h"
#include "ui/MapDocument.h"

#include "kdl/memory_utils.h"

#include "vm/bbox.h"

#include <cassert>
#include <utility>

namespace tb::ui
{

MoveObjectsTool::MoveObjectsTool(std::weak_ptr<MapDocument> document)
  : Tool{true}
  , m_document{std::move(document)}
{
}

const mdl::Grid& MoveObjectsTool::grid() const
{
  return kdl::mem_lock(m_document)->grid();
}

bool MoveObjectsTool::startMove(const InputState& inputState)
{
  auto document = kdl::mem_lock(m_document);

  if (!document->selection().brushFaces.empty())
  {
    return false;
  }

  document->startTransaction(
    duplicateObjects(inputState) ? "Duplicate Objects" : "Move Objects",
    mdl::TransactionScope::LongRunning);
  m_duplicateObjects = duplicateObjects(inputState);
  return true;
}

MoveObjectsTool::MoveResult MoveObjectsTool::move(
  const InputState&, const vm::vec3d& delta)
{
  auto document = kdl::mem_lock(m_document);
  const auto& worldBounds = document->worldBounds();
  const auto bounds = document->selectionBounds();
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
    document->duplicate();
  }

  return document->translate(delta) ? MoveResult::Continue : MoveResult::Deny;
}

void MoveObjectsTool::endMove(const InputState&)
{
  auto document = kdl::mem_lock(m_document);
  document->commitTransaction();
}

void MoveObjectsTool::cancelMove()
{
  auto document = kdl::mem_lock(m_document);
  document->cancelTransaction();
}

bool MoveObjectsTool::duplicateObjects(const InputState& inputState) const
{
  return inputState.modifierKeysDown(ModifierKeys::CtrlCmd);
}

} // namespace tb::ui
