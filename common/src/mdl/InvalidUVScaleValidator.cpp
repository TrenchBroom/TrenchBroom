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

#include "InvalidUVScaleValidator.h"

#include "mdl/BrushFace.h"
#include "mdl/BrushFaceAttributes.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushNode.h"
#include "mdl/Issue.h"
#include "mdl/IssueQuickFix.h"
#include "mdl/Map.h"
#include "mdl/Map_Brushes.h"
#include "mdl/Map_Selection.h"
#include "mdl/PushSelection.h"
#include "mdl/UpdateBrushFaceAttributes.h"

#include <string>
#include <vector>

namespace tb::mdl
{
namespace
{

const auto Type = freeIssueType();

IssueQuickFix makeResetUVScaleQuickFix()
{
  return {"Reset UV Scale", [](Map& map, const std::vector<const Issue*>& issues) {
            const auto pushSelection = PushSelection{map};

            auto faceHandles = std::vector<BrushFaceHandle>{};
            for (const auto* issue : issues)
            {
              if (issue->type() == Type)
              {
                auto& brushNode = static_cast<BrushNode&>(issue->node());
                const auto faceIndex =
                  static_cast<const BrushFaceIssue*>(issue)->faceIndex();
                faceHandles.emplace_back(&brushNode, faceIndex);
              }
            }

            deselectAll(map);
            selectBrushFaces(map, faceHandles);
            setBrushFaceAttributes(
              map,
              {
                .xScale = SetValue{1.0f},
                .yScale = SetValue{1.0f},
              });
          }};
}
} // namespace

InvalidUVScaleValidator::InvalidUVScaleValidator()
  : Validator{Type, "Invalid UV scale"}
{
  addQuickFix(makeResetUVScaleQuickFix());
}

void InvalidUVScaleValidator::doValidate(
  BrushNode& brushNode, std::vector<std::unique_ptr<Issue>>& issues) const
{
  const auto& brush = brushNode.brush();
  for (size_t i = 0u; i < brush.faceCount(); ++i)
  {
    const auto& face = brush.face(i);
    if (!face.attributes().valid())
    {
      issues.push_back(std::make_unique<BrushFaceIssue>(
        Type, brushNode, i, "Face has invalid UV scale."));
    }
  }
}

} // namespace tb::mdl
