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

#include <string>

namespace tb::mdl
{
class Map;

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
  explicit Command(std::string name);
  virtual ~Command();

public:
  CommandState state() const;
  const std::string& name() const;

  virtual bool performDo(Map& map);

private:
  virtual bool doPerformDo(Map& map) = 0;

  deleteCopyAndMove(Command);
};

} // namespace tb::mdl
