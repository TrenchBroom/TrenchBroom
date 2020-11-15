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

#include "MissingDefinitionIssueGenerator.h"

#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/MapFacade.h"

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class MissingDefinitionIssueGenerator::MissingDefinitionIssue : public Issue {
        public:
            static const IssueType Type;
        public:
            explicit MissingDefinitionIssue(AttributableNode* node) :
            Issue(node) {}
        private:
            IssueType doGetType() const override {
                return Type;
            }

            std::string doGetDescription() const override {
                const AttributableNode* attributableNode = static_cast<AttributableNode*>(node());
                return attributableNode->name() + " not found in entity definitions";
            }
        };

        const IssueType MissingDefinitionIssueGenerator::MissingDefinitionIssue::Type = Issue::freeType();

        class MissingDefinitionIssueGenerator::MissingDefinitionIssueQuickFix : public IssueQuickFix {
        public:
            MissingDefinitionIssueQuickFix() :
            IssueQuickFix(MissingDefinitionIssue::Type, "Delete entities") {}
        private:
            void doApply(MapFacade* facade, const IssueList& /* issues */) const override {
                facade->deleteObjects();
            }
        };

        MissingDefinitionIssueGenerator::MissingDefinitionIssueGenerator() :
        IssueGenerator(MissingDefinitionIssue::Type, "Missing entity definition") {
            addQuickFix(new MissingDefinitionIssueQuickFix());
        }

        void MissingDefinitionIssueGenerator::doGenerate(AttributableNode* node, IssueList& issues) const {
            if (node->entity().definition() == nullptr)
                issues.push_back(new MissingDefinitionIssue(node));
        }
    }
}
