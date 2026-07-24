/*
 Copyright (C) 2026 Kristian Duske

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

#include "base/Macros.h"
#include "mdl/NodeContents.h"
#include "mdl/NodeHandleManager.h"
#include "mdl/NodeHandles.h"
#include "mdl/SwapNodeContentsCommand.h"

#include "kd/ranges/to.h"

#include <ranges>
#include <vector>

namespace tb::mdl
{
class BrushNode;

namespace detail
{

template <typename HandleType>
std::vector<HandleType> toHandles(
  const std::vector<typename HandleType::Position>& positions)
{
  return positions | std::views::transform([](const auto& position) {
           return HandleType{position};
         })
         | kdl::ranges::to<std::vector>();
}

} // namespace detail

class NodeHandleManager;

template <typename HandleType>
class NodeHandleCommand : public SwapNodeContentsCommand
{
private:
  std::vector<HandleType> m_oldHandles;
  std::vector<HandleType> m_newHandles;

public:
  NodeHandleCommand(
    std::string name,
    std::vector<std::pair<Node*, NodeContents>> nodes,
    const std::vector<typename HandleType::Position>& oldHandlePositions,
    const std::vector<typename HandleType::Position>& newHandlePositions)
    : SwapNodeContentsCommand{std::move(name), std::move(nodes)}
    , m_oldHandles{detail::toHandles<HandleType>(oldHandlePositions)}
    , m_newHandles{detail::toHandles<HandleType>(newHandlePositions)}
  {
  }

public:
  bool hasRemainingHandles() const { return !m_newHandles.empty(); }

  template <typename... AdditionalHandleTypes>
  void removeHandles(NodeHandleManager& manager)
  {
    const auto nodes = m_nodes | std::views::keys | kdl::ranges::to<std::vector>();
    manager.removeHandles<HandleType>(nodes);
    (manager.removeHandles<AdditionalHandleTypes>(nodes), ...);
  }

  template <typename... AdditionalHandleTypes>
  void addHandles(NodeHandleManager& manager)
  {
    const auto nodes = m_nodes | std::views::keys | kdl::ranges::to<std::vector>();
    manager.addHandles<HandleType>(nodes);
    (manager.addHandles<AdditionalHandleTypes>(nodes), ...);
  }

  void selectNewHandlePositions(NodeHandleManager& manager)
  {
    manager.selectHandles<HandleType>(m_newHandles);
  }

  void selectOldHandlePositions(NodeHandleManager& manager)
  {
    manager.selectHandles<HandleType>(m_oldHandles);
  }

  deleteCopyAndMove(NodeHandleCommand);
};

} // namespace tb::mdl
