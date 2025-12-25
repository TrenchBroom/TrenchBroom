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

#include "mdl/NonIntegerVerticesValidator.h"

#include "mdl/Brush.h"
#include "mdl/BrushNode.h"
#include "mdl/Issue.h"
#include "mdl/IssueQuickFix.h"
#include "mdl/Map.h"
#include "mdl/Map_Geometry.h"

#include <algorithm>
#include <string>

namespace tb::mdl
{
namespace
{

const auto Type = freeIssueType();

IssueQuickFix makeSnapVerticesQuickFix()
{
  return {"Snap Vertices", [](auto& map, const auto&) { snapVertices(map, 1); }};
}

} // namespace

NonIntegerVerticesValidator::NonIntegerVerticesValidator()
  : Validator{Type, "Non-integer vertices"}
{
  addQuickFix(makeSnapVerticesQuickFix());
}

void NonIntegerVerticesValidator::doValidate(
  BrushNode& brushNode, std::vector<std::unique_ptr<Issue>>& issues) const
{
  const auto& vertices = brushNode.brush().vertices();
  if (!std::ranges::all_of(
        vertices, [](const auto* vertex) { return vm::is_integral(vertex->position()); }))
  {
    issues.push_back(
      std::make_unique<Issue>(Type, brushNode, "Brush has non-integer vertices"));
  }
}

} // namespace tb::mdl
