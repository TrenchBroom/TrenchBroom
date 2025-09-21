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
#include "mdl/Command.h"

#include <memory>
#include <string>

namespace tb::mdl
{

class UndoableCommand : public Command
{
private:
  size_t m_modificationCount;

protected:
  UndoableCommand(std::string name, bool updateModificationCount);

public:
  ~UndoableCommand() override;

  std::unique_ptr<CommandResult> performDo(Map& map) override;
  virtual std::unique_ptr<CommandResult> performUndo(Map& map);

  virtual bool collateWith(UndoableCommand& command);

protected:
  virtual std::unique_ptr<CommandResult> doPerformUndo(Map& map) = 0;

  virtual bool doCollateWith(UndoableCommand& command);

  void setModificationCount(Map& map) const;
  void resetModificationCount(Map& map) const;

  deleteCopyAndMove(UndoableCommand);
};

} // namespace tb::mdl
