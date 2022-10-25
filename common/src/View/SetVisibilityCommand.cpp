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

#include "SetVisibilityCommand.h"
#include "Macros.h"
#include "Model/VisibilityState.h"
#include "View/MapDocumentCommandFacade.h"

#include <string>

namespace TrenchBroom
{
namespace View
{
std::unique_ptr<SetVisibilityCommand> SetVisibilityCommand::show(
  std::vector<Model::Node*> nodes)
{
  return std::make_unique<SetVisibilityCommand>(std::move(nodes), Action::Show);
}

std::unique_ptr<SetVisibilityCommand> SetVisibilityCommand::hide(
  std::vector<Model::Node*> nodes)
{
  return std::make_unique<SetVisibilityCommand>(std::move(nodes), Action::Hide);
}

std::unique_ptr<SetVisibilityCommand> SetVisibilityCommand::ensureVisible(
  std::vector<Model::Node*> nodes)
{
  return std::make_unique<SetVisibilityCommand>(std::move(nodes), Action::Ensure);
}

std::unique_ptr<SetVisibilityCommand> SetVisibilityCommand::reset(
  std::vector<Model::Node*> nodes)
{
  return std::make_unique<SetVisibilityCommand>(std::move(nodes), Action::Reset);
}

SetVisibilityCommand::SetVisibilityCommand(
  std::vector<Model::Node*> nodes, const Action action)
  : UndoableCommand(makeName(action), false)
  , m_nodes{std::move(nodes)}
  , m_action{action}
{
}

std::string SetVisibilityCommand::makeName(const Action action)
{
  switch (action)
  {
  case Action::Reset:
    return "Reset Visibility";
  case Action::Hide:
    return "Hide Objects";
  case Action::Show:
    return "Show Objects";
  case Action::Ensure:
    return "Ensure Objects Visible";
    switchDefault();
  }
}

std::unique_ptr<CommandResult> SetVisibilityCommand::doPerformDo(
  MapDocumentCommandFacade* document)
{
  switch (m_action)
  {
  case Action::Reset:
    m_oldState = document->setVisibilityState(m_nodes, Model::VisibilityState::Inherited);
    break;
  case Action::Hide:
    m_oldState = document->setVisibilityState(m_nodes, Model::VisibilityState::Hidden);
    break;
  case Action::Show:
    m_oldState = document->setVisibilityState(m_nodes, Model::VisibilityState::Shown);
    break;
  case Action::Ensure:
    m_oldState = document->setVisibilityEnsured(m_nodes);
    break;
    switchDefault();
  }
  return std::make_unique<CommandResult>(true);
}

std::unique_ptr<CommandResult> SetVisibilityCommand::doPerformUndo(
  MapDocumentCommandFacade* document)
{
  document->restoreVisibilityState(m_oldState);
  return std::make_unique<CommandResult>(true);
}
} // namespace View
} // namespace TrenchBroom
