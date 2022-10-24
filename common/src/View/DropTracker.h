/*
 Copyright (C) 2021 Kristian Duske

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

namespace TrenchBroom
{
namespace View
{
class InputState;

class DropTracker
{
public:
  virtual ~DropTracker();

  virtual bool move(const InputState& inputState) = 0;
  virtual bool drop(const InputState& inputState) = 0;
  virtual void leave(const InputState& inputState) = 0;
};
} // namespace View
} // namespace TrenchBroom
