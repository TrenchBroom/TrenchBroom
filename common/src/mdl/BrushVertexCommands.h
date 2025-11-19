/*
 Copyright (C) 2020 Kristian Duske

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

#include "Macros.h"
#include "mdl/NodeContents.h"
#include "mdl/SwapNodeContentsCommand.h"

#include "vm/polygon.h"
#include "vm/segment.h"
#include "vm/vec.h"

#include <memory>
#include <vector>

namespace tb::mdl
{
class BrushNode;

namespace detail
{

std::vector<BrushNode*> collectBrushNodes(
  const std::vector<std::pair<Node*, NodeContents>>& nodes);

} // namespace detail

template <typename H>
class VertexHandleManagerBaseT;

class BrushVertexCommandResult : public CommandResult
{
private:
  bool m_hasRemainingVertices;

public:
  BrushVertexCommandResult(bool success, bool hasRemainingVertices);

  bool hasRemainingVertices() const;
};

template <typename H>
class BrushVertexCommandT : public SwapNodeContentsCommand
{
private:
  std::vector<H> m_oldPositions;
  std::vector<H> m_newPositions;

public:
  BrushVertexCommandT(
    std::string name,
    std::vector<std::pair<Node*, NodeContents>> nodes,
    std::vector<H> oldPositions,
    std::vector<H> newPositions)
    : SwapNodeContentsCommand{std::move(name), std::move(nodes)}
    , m_oldPositions{std::move(oldPositions)}
    , m_newPositions{std::move(newPositions)}
  {
  }

private:
  std::unique_ptr<CommandResult> doPerformDo(Map& document) override
  {
    const auto swapResult = SwapNodeContentsCommand::doPerformDo(document);
    return std::make_unique<BrushVertexCommandResult>(
      swapResult->success(), !m_newPositions.empty());
  }

public:
  bool hasRemainingHandles() const { return !m_newPositions.empty(); }

  template <typename HT>
  void removeHandles(VertexHandleManagerBaseT<HT>& manager)
  {
    const auto nodes = detail::collectBrushNodes(m_nodes);
    manager.removeHandles(nodes);
  }

  template <typename HT>
  void addHandles(VertexHandleManagerBaseT<HT>& manager)
  {
    const auto nodes = detail::collectBrushNodes(m_nodes);
    manager.addHandles(nodes);
  }

  template <typename HT>
  void selectNewHandlePositions(VertexHandleManagerBaseT<HT>& manager)
  {
    manager.select(m_newPositions);
  }

  template <typename HT>
  void selectOldHandlePositions(VertexHandleManagerBaseT<HT>& manager)
  {
    manager.select(m_oldPositions);
  }

  deleteCopyAndMove(BrushVertexCommandT);
};

using BrushVertexCommand = BrushVertexCommandT<vm::vec3d>;
using BrushEdgeCommand = BrushVertexCommandT<vm::segment3d>;
using BrushFaceCommand = BrushVertexCommandT<vm::polygon3d>;

} // namespace tb::mdl
