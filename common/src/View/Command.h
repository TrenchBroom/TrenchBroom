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

#include <memory>
#include <string>

namespace TrenchBroom
{
namespace View
{
class MapDocumentCommandFacade;

class CommandResult
{
private:
  bool m_success;

public:
  explicit CommandResult(bool success);
  virtual ~CommandResult();

  bool success() const;
};

class Command
{
public:
  enum class CommandState
  {
    Default,
    Doing,
    Done,
    Undoing
  };

protected:
  CommandState m_state;
  std::string m_name;

public:
  Command(std::string name);
  virtual ~Command();

public:
  CommandState state() const;
  const std::string& name() const;

  virtual std::unique_ptr<CommandResult> performDo(MapDocumentCommandFacade* document);

private:
  virtual std::unique_ptr<CommandResult> doPerformDo(
    MapDocumentCommandFacade* document) = 0;

  deleteCopyAndMove(Command);
};
} // namespace View
} // namespace TrenchBroom
