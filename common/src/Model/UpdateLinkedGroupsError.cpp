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

#include "UpdateLinkedGroupsError.h"

#include <ostream>

namespace TrenchBroom
{
namespace Model
{
std::ostream& operator<<(std::ostream& str, const UpdateLinkedGroupsError& e)
{
  switch (e)
  {
  case UpdateLinkedGroupsError::TransformIsNotInvertible:
    str << "Group transformation is not invertible";
    break;
  case UpdateLinkedGroupsError::TransformFailed:
    str << "Failed to transform a linked node";
    break;
  case UpdateLinkedGroupsError::UpdateExceedsWorldBounds:
    str << "Updating a linked node would exceed world bounds";
    break;
  case UpdateLinkedGroupsError::UpdateIsInconsistent:
    str << "Cannot update multiple members of the same link set";
    break;
  }
  return str;
}
} // namespace Model
} // namespace TrenchBroom
