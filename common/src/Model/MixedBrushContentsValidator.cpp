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

#include "MixedBrushContentsValidator.h"

#include "Model/BrushFace.h"
#include "Model/BrushNode.h"
#include "Model/Issue.h"

#include <cassert>
#include <string>
#include <vector>

namespace TrenchBroom {
namespace Model {
class MixedBrushContentsValidator::MixedBrushContentsIssue : public Issue {
public:
  friend class MixedBrushContentsIssueQuickFix;

public:
  static const IssueType Type;

public:
  explicit MixedBrushContentsIssue(BrushNode& brush)
    : Issue(brush) {}

  IssueType doGetType() const override { return Type; }

  std::string doGetDescription() const override { return "Brush has mixed content flags"; }
};

const IssueType MixedBrushContentsValidator::MixedBrushContentsIssue::Type = Issue::freeType();

MixedBrushContentsValidator::MixedBrushContentsValidator()
  : Validator(MixedBrushContentsIssue::Type, "Mixed brush content flags") {}

void MixedBrushContentsValidator::doValidate(
  BrushNode& brushNode, std::vector<std::unique_ptr<Issue>>& issues) const {
  const Brush& brush = brushNode.brush();
  const auto& faces = brush.faces();
  auto it = std::begin(faces);
  auto end = std::end(faces);
  assert(it != end);

  const int contentFlags = it->resolvedSurfaceContents();
  ++it;
  while (it != end) {
    if (it->resolvedSurfaceContents() != contentFlags) {
      issues.push_back(std::make_unique<MixedBrushContentsIssue>(brushNode));
    }
    ++it;
  }
}
} // namespace Model
} // namespace TrenchBroom
