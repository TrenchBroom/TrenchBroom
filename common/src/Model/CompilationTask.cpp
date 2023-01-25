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

#include "CompilationTask.h"

#include <kdl/reflection_impl.h>

namespace TrenchBroom
{
namespace Model
{

kdl_reflect_impl(CompilationExportMap);

kdl_reflect_impl(CompilationCopyFiles);

kdl_reflect_impl(CompilationDeleteFiles);

kdl_reflect_impl(CompilationRunTool);

std::ostream& operator<<(std::ostream& lhs, const CompilationTask& rhs)
{
  std::visit([&](const auto& x) { lhs << x; }, rhs);
  return lhs;
}

} // namespace Model
} // namespace TrenchBroom
