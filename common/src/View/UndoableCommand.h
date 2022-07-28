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

#pragma once

#include "Macros.h"
#include "View/Command.h"

#include <memory>
#include <string>

namespace TrenchBroom {
namespace View {
class MapDocumentCommandFacade;

class UndoableCommand : public Command {
private:
  size_t m_modificationCount;

protected:
  UndoableCommand(std::string name, bool updateModificationCount);

public:
  virtual ~UndoableCommand();

  std::unique_ptr<CommandResult> performDo(MapDocumentCommandFacade* document) override;
  virtual std::unique_ptr<CommandResult> performUndo(MapDocumentCommandFacade* document);

  virtual bool collateWith(UndoableCommand& command);

private:
  virtual std::unique_ptr<CommandResult> doPerformUndo(MapDocumentCommandFacade* document) = 0;

  virtual bool doCollateWith(UndoableCommand& command);

  deleteCopyAndMove(UndoableCommand);
};
} // namespace View
} // namespace TrenchBroom
