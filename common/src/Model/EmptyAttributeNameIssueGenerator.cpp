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

#include "EmptyAttributeNameIssueGenerator.h"

#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/MapFacade.h"
#include "Model/PushSelection.h"

#include <string>

namespace TrenchBroom {
    namespace Model {
        class EmptyAttributeNameIssueGenerator::EmptyAttributeNameIssue : public Issue {
        public:
            static const IssueType Type;
        public:
            explicit EmptyAttributeNameIssue(AttributableNode* node) :
            Issue(node) {}

            IssueType doGetType() const override {
                return Type;
            }

            std::string doGetDescription() const override {
                const AttributableNode* attributableNode = static_cast<AttributableNode*>(node());
                return attributableNode->name() + " has a property with an empty name.";
            }
        };

        const IssueType EmptyAttributeNameIssueGenerator::EmptyAttributeNameIssue::Type = Issue::freeType();

        class EmptyAttributeNameIssueGenerator::EmptyAttributeNameIssueQuickFix : public IssueQuickFix {
        public:
            EmptyAttributeNameIssueQuickFix() :
            IssueQuickFix(EmptyAttributeNameIssue::Type, "Delete property") {}
        private:
            void doApply(MapFacade* facade, const Issue* issue) const override {
                const PushSelection push(facade);

                // If world node is affected, the selection will fail, but if nothing is selected,
                // the removeAttribute call will correctly affect worldspawn either way.

                facade->deselectAll();
                facade->select(issue->node());
                facade->removeAttribute("");
            }
        };

        EmptyAttributeNameIssueGenerator::EmptyAttributeNameIssueGenerator() :
        IssueGenerator(EmptyAttributeNameIssue::Type, "Empty property name") {
            addQuickFix(new EmptyAttributeNameIssueQuickFix());
        }

        void EmptyAttributeNameIssueGenerator::doGenerate(AttributableNode* node, IssueList& issues) const {
            if (node->entity().hasAttribute(""))
                issues.push_back(new EmptyAttributeNameIssue(node));
        }
    }
}
