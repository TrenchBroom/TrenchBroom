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

#include "CurrentGroupCommand.h"

#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
namespace View {
std::unique_ptr<CurrentGroupCommand> CurrentGroupCommand::push(Model::GroupNode* group) {
  return std::make_unique<CurrentGroupCommand>(group);
}

std::unique_ptr<CurrentGroupCommand> CurrentGroupCommand::pop() {
  return std::make_unique<CurrentGroupCommand>(nullptr);
}

CurrentGroupCommand::CurrentGroupCommand(Model::GroupNode* group)
  : UndoableCommand(group != nullptr ? "Push Group" : "Pop Group", false)
  , m_group(group) {}

std::unique_ptr<CommandResult> CurrentGroupCommand::doPerformDo(
  MapDocumentCommandFacade* document) {
  if (m_group != nullptr) {
    document->performPushGroup(m_group);
    m_group = nullptr;
  } else {
    m_group = document->currentGroup();
    document->performPopGroup();
  }
  return std::make_unique<CommandResult>(true);
}

std::unique_ptr<CommandResult> CurrentGroupCommand::doPerformUndo(
  MapDocumentCommandFacade* document) {
  if (m_group == nullptr) {
    m_group = document->currentGroup();
    document->performPopGroup();
  } else {
    document->performPushGroup(m_group);
    m_group = nullptr;
  }
  return std::make_unique<CommandResult>(true);
}
} // namespace View
} // namespace TrenchBroom
