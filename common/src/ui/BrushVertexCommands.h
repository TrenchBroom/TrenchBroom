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
#include "ui/SwapNodeContentsCommand.h"

#include "vm/polygon.h"
#include "vm/segment.h"
#include "vm/vec.h"

#include <memory>
#include <vector>

namespace tb::ui
{
class MapDocument;
class VertexHandleManagerBase;
template <typename H>
class VertexHandleManagerBaseT;

class BrushVertexCommandBase : public SwapNodeContentsCommand
{
protected:
  BrushVertexCommandBase(
    std::string name, std::vector<std::pair<mdl::Node*, mdl::NodeContents>> nodes);

private:
  std::unique_ptr<CommandResult> doPerformDo(MapDocument& document) override;
  virtual std::unique_ptr<CommandResult> createCommandResult(
    std::unique_ptr<CommandResult> swapResult);

public:
  void removeHandles(VertexHandleManagerBase& manager);
  void addHandles(VertexHandleManagerBase& manager);

public:
  virtual void selectNewHandlePositions(
    VertexHandleManagerBaseT<vm::vec3d>& manager) const;
  virtual void selectOldHandlePositions(
    VertexHandleManagerBaseT<vm::vec3d>& manager) const;
  virtual void selectNewHandlePositions(
    VertexHandleManagerBaseT<vm::segment3d>& manager) const;
  virtual void selectOldHandlePositions(
    VertexHandleManagerBaseT<vm::segment3d>& manager) const;
  virtual void selectNewHandlePositions(
    VertexHandleManagerBaseT<vm::polygon3d>& manager) const;
  virtual void selectOldHandlePositions(
    VertexHandleManagerBaseT<vm::polygon3d>& manager) const;

  deleteCopyAndMove(BrushVertexCommandBase);
};

class BrushVertexCommandResult : public CommandResult
{
private:
  bool m_hasRemainingVertices;

public:
  BrushVertexCommandResult(bool success, bool hasRemainingVertices);

  bool hasRemainingVertices() const;
};

class BrushVertexCommand : public BrushVertexCommandBase
{
private:
  std::vector<vm::vec3d> m_oldVertexPositions;
  std::vector<vm::vec3d> m_newVertexPositions;

public:
  BrushVertexCommand(
    std::string name,
    std::vector<std::pair<mdl::Node*, mdl::NodeContents>> nodes,
    std::vector<vm::vec3d> oldVertexPositions,
    std::vector<vm::vec3d> newVertexPositions);

private:
  std::unique_ptr<CommandResult> createCommandResult(
    std::unique_ptr<CommandResult> swapResult) override;

  bool doCollateWith(UndoableCommand& command) override;

  void selectNewHandlePositions(
    VertexHandleManagerBaseT<vm::vec3d>& manager) const override;
  void selectOldHandlePositions(
    VertexHandleManagerBaseT<vm::vec3d>& manager) const override;

  deleteCopyAndMove(BrushVertexCommand);
};

class BrushEdgeCommand : public BrushVertexCommandBase
{
private:
  std::vector<vm::segment3d> m_oldEdgePositions;
  std::vector<vm::segment3d> m_newEdgePositions;

public:
  BrushEdgeCommand(
    std::string name,
    std::vector<std::pair<mdl::Node*, mdl::NodeContents>> nodes,
    std::vector<vm::segment3d> oldEdgePositions,
    std::vector<vm::segment3d> newEdgePositions);

private:
  bool doCollateWith(UndoableCommand& command) override;

  void selectNewHandlePositions(
    VertexHandleManagerBaseT<vm::segment3d>& manager) const override;
  void selectOldHandlePositions(
    VertexHandleManagerBaseT<vm::segment3d>& manager) const override;

  deleteCopyAndMove(BrushEdgeCommand);
};

class BrushFaceCommand : public BrushVertexCommandBase
{
private:
  std::vector<vm::polygon3d> m_oldFacePositions;
  std::vector<vm::polygon3d> m_newFacePositions;

public:
  BrushFaceCommand(
    std::string name,
    std::vector<std::pair<mdl::Node*, mdl::NodeContents>> nodes,
    std::vector<vm::polygon3d> oldFacePositions,
    std::vector<vm::polygon3d> newFacePositions);

private:
  bool doCollateWith(UndoableCommand& command) override;

  void selectNewHandlePositions(
    VertexHandleManagerBaseT<vm::polygon3d>& manager) const override;
  void selectOldHandlePositions(
    VertexHandleManagerBaseT<vm::polygon3d>& manager) const override;

  deleteCopyAndMove(BrushFaceCommand);
};

} // namespace tb::ui
