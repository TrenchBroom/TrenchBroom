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

#include "SetVisibilityCommand.h"

#include "Macros.h"
#include "mdl/VisibilityState.h"
#include "ui/MapDocument.h"

#include <string>

namespace tb::mdl
{
namespace
{

auto setVisibilityState(
  const std::vector<mdl::Node*>& nodes,
  const mdl::VisibilityState visibilityState,
  ui::MapDocument& document)
{
  auto result = std::vector<std::tuple<mdl::Node*, mdl::VisibilityState>>{};
  result.reserve(nodes.size());

  auto changedNodes = std::vector<mdl::Node*>{};
  changedNodes.reserve(nodes.size());

  for (auto* node : nodes)
  {
    const auto oldState = node->visibilityState();
    if (node->setVisibilityState(visibilityState))
    {
      changedNodes.push_back(node);
      result.emplace_back(node, oldState);
    }
  }

  document.nodeVisibilityDidChangeNotifier(changedNodes);

  return result;
}

auto setVisibilityEnsured(const std::vector<mdl::Node*>& nodes, ui::MapDocument& document)
{
  auto result = std::vector<std::tuple<mdl::Node*, mdl::VisibilityState>>{};
  result.reserve(nodes.size());

  auto changedNodes = std::vector<mdl::Node*>{};
  changedNodes.reserve(nodes.size());

  for (auto* node : nodes)
  {
    const auto oldState = node->visibilityState();
    if (node->ensureVisible())
    {
      changedNodes.push_back(node);
      result.emplace_back(node, oldState);
    }
  }

  document.nodeVisibilityDidChangeNotifier(changedNodes);

  return result;
}

void restoreVisibilityState(
  const std::vector<std::tuple<mdl::Node*, mdl::VisibilityState>>& nodes,
  ui::MapDocument& document)
{
  auto changedNodes = std::vector<mdl::Node*>{};
  changedNodes.reserve(nodes.size());

  for (const auto& [node, state] : nodes)
  {
    if (node->setVisibilityState(state))
    {
      changedNodes.push_back(node);
    }
  }

  document.nodeVisibilityDidChangeNotifier(changedNodes);
}

} // namespace

enum class SetVisibilityCommand::Action
{
  Reset,
  Hide,
  Show,
  Ensure,
};

std::unique_ptr<SetVisibilityCommand> SetVisibilityCommand::show(
  std::vector<mdl::Node*> nodes)
{
  return std::make_unique<SetVisibilityCommand>(std::move(nodes), Action::Show);
}

std::unique_ptr<SetVisibilityCommand> SetVisibilityCommand::hide(
  std::vector<mdl::Node*> nodes)
{
  return std::make_unique<SetVisibilityCommand>(std::move(nodes), Action::Hide);
}

std::unique_ptr<SetVisibilityCommand> SetVisibilityCommand::ensureVisible(
  std::vector<mdl::Node*> nodes)
{
  return std::make_unique<SetVisibilityCommand>(std::move(nodes), Action::Ensure);
}

std::unique_ptr<SetVisibilityCommand> SetVisibilityCommand::reset(
  std::vector<mdl::Node*> nodes)
{
  return std::make_unique<SetVisibilityCommand>(std::move(nodes), Action::Reset);
}

SetVisibilityCommand::SetVisibilityCommand(
  std::vector<mdl::Node*> nodes, const Action action)
  : UndoableCommand{makeName(action), false}
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
  ui::MapDocument& document)
{
  switch (m_action)
  {
  case Action::Reset:
    m_oldState = setVisibilityState(m_nodes, mdl::VisibilityState::Inherited, document);
    break;
  case Action::Hide:
    m_oldState = setVisibilityState(m_nodes, mdl::VisibilityState::Hidden, document);
    break;
  case Action::Show:
    m_oldState = setVisibilityState(m_nodes, mdl::VisibilityState::Shown, document);
    break;
  case Action::Ensure:
    m_oldState = setVisibilityEnsured(m_nodes, document);
    break;
    switchDefault();
  }
  return std::make_unique<CommandResult>(true);
}

std::unique_ptr<CommandResult> SetVisibilityCommand::doPerformUndo(
  ui::MapDocument& document)
{
  restoreVisibilityState(m_oldState, document);
  return std::make_unique<CommandResult>(true);
}

} // namespace tb::mdl
