/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "SetLockStateCommand.h"
#include "Macros.h"
#include "Model/BrushNode.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/LockState.h"
#include "Model/Node.h"
#include "Model/WorldNode.h"
#include "View/MapDocumentCommandFacade.h"

#include <kdl/overload.h>

#include <string>

namespace TrenchBroom {
namespace View {
const Command::CommandType SetLockStateCommand::Type = Command::freeType();

std::unique_ptr<SetLockStateCommand> SetLockStateCommand::lock(
  const std::vector<Model::Node*>& nodes) {
  return std::make_unique<SetLockStateCommand>(nodes, Model::LockState::Locked);
}

std::unique_ptr<SetLockStateCommand> SetLockStateCommand::unlock(
  const std::vector<Model::Node*>& nodes) {
  return std::make_unique<SetLockStateCommand>(nodes, Model::LockState::Unlocked);
}

std::unique_ptr<SetLockStateCommand> SetLockStateCommand::reset(
  const std::vector<Model::Node*>& nodes) {
  return std::make_unique<SetLockStateCommand>(nodes, Model::LockState::Inherited);
}

static bool shouldUpdateModificationCount(const std::vector<Model::Node*>& nodes) {
  for (const auto* node : nodes) {
    const auto modifiesLayer = node->accept(kdl::overload(
      [](const Model::WorldNode*) {
        return false;
      },
      [](const Model::LayerNode*) {
        return true;
      },
      [](const Model::GroupNode*) {
        return false;
      },
      [](const Model::EntityNode*) {
        return false;
      },
      [](const Model::BrushNode*) {
        return false;
      },
      [](const Model::PatchNode*) {
        return false;
      }));
    if (modifiesLayer) {
      return true;
    }
  }
  return false;
}

SetLockStateCommand::SetLockStateCommand(
  const std::vector<Model::Node*>& nodes, const Model::LockState lockState)
  : UndoableCommand(Type, makeName(lockState), shouldUpdateModificationCount(nodes))
  , m_nodes(nodes)
  , m_lockState(lockState) {}

std::string SetLockStateCommand::makeName(const Model::LockState state) {
  switch (state) {
    case Model::LockState::Inherited:
      return "Reset Locking";
    case Model::LockState::Locked:
      return "Lock Objects";
    case Model::LockState::Unlocked:
      return "Unlock Objects";
      switchDefault();
  }
}

std::unique_ptr<CommandResult> SetLockStateCommand::doPerformDo(
  MapDocumentCommandFacade* document) {
  m_oldLockState = document->setLockState(m_nodes, m_lockState);
  return std::make_unique<CommandResult>(true);
}

std::unique_ptr<CommandResult> SetLockStateCommand::doPerformUndo(
  MapDocumentCommandFacade* document) {
  document->restoreLockState(m_oldLockState);
  return std::make_unique<CommandResult>(true);
}

bool SetLockStateCommand::doCollateWith(UndoableCommand&) {
  return false;
}
} // namespace View
} // namespace TrenchBroom
