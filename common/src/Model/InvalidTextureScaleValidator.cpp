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

#include "InvalidTextureScaleValidator.h"

#include "Model/BrushFace.h"
#include "Model/BrushFaceAttributes.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushNode.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/MapFacade.h"
#include "Model/PushSelection.h"

#include <string>
#include <vector>

namespace TrenchBroom {
namespace Model {
class InvalidTextureScaleValidator::InvalidTextureScaleIssue : public BrushFaceIssue {
public:
  friend class InvalidTextureScaleIssueQuickFix;

public:
  static const IssueType Type;

public:
  explicit InvalidTextureScaleIssue(BrushNode& node, const size_t faceIndex)
    : BrushFaceIssue(node, faceIndex) {}

  IssueType doGetType() const override { return Type; }

  std::string doGetDescription() const override { return "Face has invalid texture scale."; }
};

const IssueType InvalidTextureScaleValidator::InvalidTextureScaleIssue::Type = Issue::freeType();

class InvalidTextureScaleValidator::InvalidTextureScaleIssueQuickFix : public IssueQuickFix {
public:
  InvalidTextureScaleIssueQuickFix()
    : IssueQuickFix(InvalidTextureScaleIssue::Type, "Reset texture scale") {}

private:
  void doApply(MapFacade* facade, const std::vector<const Issue*>& issues) const override {
    const PushSelection push(facade);

    std::vector<BrushFaceHandle> faceHandles;
    for (const auto* issue : issues) {
      if (issue->type() == InvalidTextureScaleIssue::Type) {
        auto& brushNode = static_cast<BrushNode&>(issue->node());
        const auto faceIndex = static_cast<const InvalidTextureScaleIssue*>(issue)->faceIndex();
        faceHandles.push_back(BrushFaceHandle(&brushNode, faceIndex));
      }
    }

    ChangeBrushFaceAttributesRequest request;
    request.setScale(vm::vec2f::one());

    facade->deselectAll();
    facade->selectBrushFaces(faceHandles);
    facade->setFaceAttributes(request);
  }
};

InvalidTextureScaleValidator::InvalidTextureScaleValidator()
  : Validator(InvalidTextureScaleIssue::Type, "Invalid texture scale") {
  addQuickFix(std::make_unique<InvalidTextureScaleIssueQuickFix>());
}

void InvalidTextureScaleValidator::doValidate(
  BrushNode& brushNode, std::vector<Issue*>& issues) const {
  const Brush& brush = brushNode.brush();
  for (size_t i = 0u; i < brush.faceCount(); ++i) {
    const BrushFace& face = brush.face(i);
    if (!face.attributes().valid()) {
      issues.push_back(new InvalidTextureScaleIssue(brushNode, i));
    }
  }
}
} // namespace Model
} // namespace TrenchBroom
