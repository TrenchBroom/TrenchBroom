/*
 Copyright (C) 2021 Amara M. Kilic
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

#include "IO/ExportOptions.h"

#include "Macros.h"

#include "kdl/overload.h"
#include "kdl/reflection_impl.h"

#include <ostream>

namespace TrenchBroom
{
namespace IO
{

kdl_reflect_impl(MapExportOptions);

std::ostream& operator<<(std::ostream& lhs, const ObjMtlPathMode rhs)
{
  switch (rhs)
  {
  case ObjMtlPathMode::RelativeToGamePath:
    lhs << "RelativeToGamePath";
    break;
  case ObjMtlPathMode::RelativeToExportPath:
    lhs << "RelativeToExportPath";
    break;
    switchDefault();
  }
  return lhs;
}

kdl_reflect_impl(ObjExportOptions);

std::ostream& operator<<(std::ostream& lhs, const ExportOptions& rhs)
{
  std::visit([&](const auto& o) { lhs << o; }, rhs);
  return lhs;
}
} // namespace IO
} // namespace TrenchBroom
