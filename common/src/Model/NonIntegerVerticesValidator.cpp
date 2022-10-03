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

#include "NonIntegerVerticesValidator.h"

#include "Model/BrushGeometry.h"
#include "Model/BrushNode.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/MapFacade.h"
#include "Polyhedron.h"

#include <string>

namespace TrenchBroom {
namespace Model {
namespace {
static const auto Type = freeIssueType();

class NonIntegerVerticesIssueQuickFix : public IssueQuickFix {
public:
  NonIntegerVerticesIssueQuickFix()
    : IssueQuickFix{Type, "Convert vertices to integer"} {}

private:
  void doApply(MapFacade* facade, const std::vector<const Issue*>& /* issues */) const override {
    facade->snapVertices(1);
  }
};
} // namespace

NonIntegerVerticesValidator::NonIntegerVerticesValidator()
  : Validator(Type, "Non-integer vertices") {
  addQuickFix(std::make_unique<NonIntegerVerticesIssueQuickFix>());
}

void NonIntegerVerticesValidator::doValidate(
  BrushNode& brushNode, std::vector<std::unique_ptr<Issue>>& issues) const {
  const auto& vertices = brushNode.brush().vertices();
  if (!std::all_of(vertices.begin(), vertices.end(), [](const auto* vertex) {
        return vm::is_integral(vertex->position());
      })) {
    issues.push_back(std::make_unique<Issue>(Type, brushNode, "Brush has non-integer vertices"));
  }
}
} // namespace Model
} // namespace TrenchBroom
