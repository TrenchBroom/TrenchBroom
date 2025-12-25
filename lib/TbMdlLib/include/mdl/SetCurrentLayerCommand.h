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

#include "Macros.h"
#include "mdl/UndoableCommand.h"

#include <memory>

namespace tb::mdl
{
class LayerNode;

class SetCurrentLayerCommand : public UndoableCommand
{
private:
  LayerNode* m_currentLayer = nullptr;
  LayerNode* m_oldCurrentLayer = nullptr;

public:
  static std::unique_ptr<SetCurrentLayerCommand> set(LayerNode* layer);

  explicit SetCurrentLayerCommand(LayerNode* layer);

private:
  bool doPerformDo(Map& map) override;
  bool doPerformUndo(Map& map) override;

  bool doCollateWith(UndoableCommand& command) override;

  deleteCopyAndMove(SetCurrentLayerCommand);
};

} // namespace tb::mdl
