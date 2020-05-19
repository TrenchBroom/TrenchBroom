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

#include "InvalidTextureScaleIssueGenerator.h"

#include "Model/BrushNode.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceAttributes.h"
#include "Model/BrushFaceHandle.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/MapFacade.h"
#include "Model/PushSelection.h"

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class InvalidTextureScaleIssueGenerator::InvalidTextureScaleIssue : public BrushFaceIssue {
        public:
            friend class InvalidTextureScaleIssueQuickFix;
        public:
            static const IssueType Type;
        public:
            explicit InvalidTextureScaleIssue(BrushNode* node, BrushFace* face) :
            BrushFaceIssue(node, face) {}

            IssueType doGetType() const override {
                return Type;
            }

            std::string doGetDescription() const override {
                return "Face has invalid texture scale.";
            }
        };

        const IssueType InvalidTextureScaleIssueGenerator::InvalidTextureScaleIssue::Type = Issue::freeType();

        class InvalidTextureScaleIssueGenerator::InvalidTextureScaleIssueQuickFix : public IssueQuickFix {
        public:
            InvalidTextureScaleIssueQuickFix() :
            IssueQuickFix(InvalidTextureScaleIssue::Type, "Reset texture scale") {}
        private:
            void doApply(MapFacade* facade, const IssueList& issues) const override {
                const PushSelection push(facade);

                std::vector<BrushFaceHandle> faceHandles;
                for (const auto* issue : issues) {
                    if (issue->type() == InvalidTextureScaleIssue::Type) {
                        BrushNode* node = static_cast<BrushNode*>(issue->node());
                        BrushFace* face = static_cast<const InvalidTextureScaleIssue*>(issue)->face();
                        faceHandles.push_back(BrushFaceHandle(node, face));
                    }
                }

                ChangeBrushFaceAttributesRequest request;
                request.setScale(vm::vec2f::one());

                facade->deselectAll();
                facade->select(faceHandles);
                facade->setFaceAttributes(request);
            }
        };

        InvalidTextureScaleIssueGenerator::InvalidTextureScaleIssueGenerator() :
        IssueGenerator(InvalidTextureScaleIssue::Type, "Invalid texture scale") {
            addQuickFix(new InvalidTextureScaleIssueQuickFix());
        }

        void InvalidTextureScaleIssueGenerator::doGenerate(BrushNode* brushNode, IssueList& issues) const {
            const Brush& brush = brushNode->brush();
            for (BrushFace* face : brush.faces()) {
                if (!face->attribs().valid()) {
                    issues.push_back(new InvalidTextureScaleIssue(brushNode, face));
                }
            }
        }
    }
}
